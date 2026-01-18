//kasjer.cpp - proces i logika kasjera

#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

struct Klient {
    pid_t pid;
    int wielkosc_grupy;
};

int sprobujZnalezcMiejsce(Stolik* tablica, int liczba_stolikow, int wielkosc_grupy) {
    for (int i = 0; i < liczba_stolikow; i++) {
        if (tablica[i].zarezerwowany) {
            continue;
        }

        bool stolik_pusty = (tablica[i].ile_zajetych_miejsc == 0);
        bool ta_sama_wielkosc = (tablica[i].wielkosc_grupy_siedzacej == wielkosc_grupy);

        if (!stolik_pusty && !ta_sama_wielkosc) {
            continue;
        }

        if (tablica[i].ile_zajetych_miejsc + wielkosc_grupy <= tablica[i].pojemnosc_max) {
            tablica[i].ile_zajetych_miejsc += wielkosc_grupy;
            if (stolik_pusty) {
                tablica[i].wielkosc_grupy_siedzacej = wielkosc_grupy;
            }
            return i;
        }
    }
    return -1;
}

int main() {
    signal(SIGINT, SIG_IGN);
    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Kasjer: Zaczynam pracę");

    std::vector<Klient> kolejka;
    Komunikat kom;

    while (true) {
        //przerwa w przyjmowaniu klientów na czas rezerwacji przez pracownika
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->blokada_rezerwacyjna) {
            if (!pam->pozar) {
                semaforPodnies(sem_id, SEM_MAIN);
                usleep(100000);
                continue;
            }
            else {
                semaforPodnies(sem_id, SEM_MAIN);
            }
        }
        else {
            semaforPodnies(sem_id, SEM_MAIN);
        }

        //odbieranie klientów w kolejce do kasy
        while (odbierzKomunikat(kol_id, TYP_KLIENT_KOLEJKA, &kom, false)) {
            Klient nowy;
            nowy.pid = kom.nadawca;
            nowy.wielkosc_grupy = kom.dane;
            kolejka.push_back(nowy);

            int id_dania = kom.id_dania;
            int ilosc = kom.dane;
            int cena_sztuki = MENU[id_dania];
            int wartosc_zamowienia = cena_sztuki * ilosc;
            semaforOpusc(sem_id, SEM_MAIN);
            pam->utarg += wartosc_zamowienia;
            semaforPodnies(sem_id, SEM_MAIN);
            std::string log = "Kasjer: mam klienta: " + std::to_string(nowy.pid) +
                                " grupa " + std::to_string(nowy.wielkosc_grupy) + " osobowa, danie nr: " +
                                std::to_string(id_dania) + " (Utarg + " + std::to_string(wartosc_zamowienia) + ")";
            logger(log);
        }

        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            semaforPodnies(sem_id, SEM_MAIN);
            logger("Kasjer: Pożar! Ewakuacja klientów!");
            while (true) {
                semaforOpusc(sem_id, SEM_MAIN);
                int pozostalo = pam->liczba_klientow;
                semaforPodnies(sem_id, SEM_MAIN);
                if (pozostalo <= 0) {
                    break;
                }
                usleep(100000);
            }
            logger("Kasjer: Wszyscy klienci ewakuowani, zamykam kasę i uciekam");
            break;
        }
        else {
            semaforPodnies(sem_id, SEM_MAIN);
        }
    
        //szukanie odpowiedniego stolika dla klienta i powiadomienie o tym pracownika jeżeli się znalazło
        if (!kolejka.empty()) {
            for (size_t i = 0; i < kolejka.size(); i++) {
                Klient k = kolejka[i];
                int przypisane_id = -1;
                int przypisany_typ = 0;

                semaforOpusc(sem_id, SEM_STOLIKI);
                //najbardziej optymalne zarobkowo przydzielanie miejsca
                if (k.wielkosc_grupy == 1) {
                    if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x1, STOLIKI_X1, 1)) != -1) {
                        przypisany_typ = 1;
                    }
                    else if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x2, STOLIKI_X2, 1)) != -1) {
                        przypisany_typ = 2;
                    }
                    else if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x3, pam->aktualna_liczba_X3, 1)) != -1) {
                        przypisany_typ = 3;
                    }
                    else if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x4, STOLIKI_X4, 1)) != -1) {
                        przypisany_typ = 4;
                    }
                }
                else if (k.wielkosc_grupy == 2) {
                    if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x2, STOLIKI_X2, 2)) != -1) {
                        przypisany_typ = 2;
                    }
                    else if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x4, STOLIKI_X4, 2)) != -1) {
                        przypisany_typ = 4;
                    }
                }
                else if (k.wielkosc_grupy == 3) {
                    if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x3, pam->aktualna_liczba_X3, 3)) != -1) {
                        przypisany_typ = 3;
                    }
                }
                else if (k.wielkosc_grupy == 4) {
                    if ((przypisane_id = sprobujZnalezcMiejsce(pam->stoliki_x4, STOLIKI_X4, 4)) != -1) {
                        przypisany_typ = 4;
                    }
                }

                semaforPodnies(sem_id, SEM_STOLIKI);

                if (przypisane_id != -1) {
                    std::string log = "Kasjer: Przypisano stolik " + std::to_string(przypisane_id) + " typu: " + std::to_string(przypisany_typ) + " dla: " + std::to_string(k.pid);
                    logger(log);
                    wyslijKomunikat(kol_id, TYP_PRACOWNIK, k.pid, k.wielkosc_grupy, przypisany_typ, przypisane_id, 0);
                    kolejka.erase(kolejka.begin() + i);
                    break;
                }
            }
       }
       usleep(100000);
    }
    odlaczPamiec(pam);
    return 0;
}