//kasjer.cpp - proces i logika kasjera

#include <iostream>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <atomic>
#include <sys/msg.h>
#include <cerrno>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

volatile sig_atomic_t ewakuacja = 0;

void handler_kasjer(int signum) {
    if (signum == SIGTERM) {
        ewakuacja = 1;
    }
}

//customowe wysyłanie komunikatów dla kasjera żeby nie zaciął się w razie pożaru przy specjalnie przepełnionej kolejce komunikatów
static bool wyslij_komunikat_przerywalnie(int kol_id, long mtyp, pid_t nadawca, int dane, int typ_stolika, int id_stolika, int id_dania,
                                         PamiecDzielona* pam) {
    Komunikat kom;
    kom.mtype = mtyp;
    kom.nadawca = nadawca;
    kom.dane = dane;
    kom.typ_stolika = typ_stolika;
    kom.id_stolika = id_stolika;
    kom.id_dania = id_dania;

    while (true) {
        if (ewakuacja || (pam != nullptr && pam->pozar)) {
            return false;
        }

        if (msgsnd(kol_id, &kom, ROZMIAR_KOM, IPC_NOWAIT) == 0) {
            return true;
        }

        if (errno == EINTR) {
            continue;
        }

        if (errno == EAGAIN) {
            usleep(2000);
            continue;
        }

        perror("Nieudana próba wysłania wiadomości (kasjer msgsnd)");
        return false;
    }
}

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
    struct sigaction sa;
    sa.sa_handler = handler_kasjer;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Kasjer: Zaczynam pracę");

    std::vector<Klient> kolejka;
    Komunikat kom;

    while (true) {
        //ewakuacja priorytetowa
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            ewakuacja = 1;
        }
        semaforPodnies(sem_id, SEM_MAIN);

        if (ewakuacja) {
            semaforCzekajNaZero(sem_id, SEM_W_BARZE);
            logger("Kasjer: Wszyscy klienci ewakuowani. Zamykam kasę i uciekam.");
            semaforPodniesBezUndo(sem_id, SEM_EWAK_KASJER_DONE);
            break;
        }


        //przerwa w przyjmowaniu klientów na czas rezerwacji przez pracownika
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->blokada_rezerwacyjna) {
            semaforPodnies(sem_id, SEM_MAIN);
            usleep(50000);
            continue;
        }
        else {
            semaforPodnies(sem_id, SEM_MAIN);
        }

        //odbieranie klientów w kolejce do kasy
        while (odbierzKomunikat(kol_id, TYP_KLIENT_KOLEJKA, &kom, false)) {
            if (ewakuacja) break;

            Klient nowy;
            nowy.pid = kom.nadawca;
            nowy.wielkosc_grupy = kom.dane;
            kolejka.push_back(nowy);

            int wartosc_zamowienia = kom.id_dania;
            
            semaforOpusc(sem_id, SEM_MAIN);
            pam->utarg += wartosc_zamowienia;
            if (pam->pozar) {
                ewakuacja = 1;
            }
            semaforPodnies(sem_id, SEM_MAIN);

            if (!ewakuacja) {
                logger("Kasjer: mam klienta: " + std::to_string(nowy.pid) +
                                    " grupa " + std::to_string(nowy.wielkosc_grupy) + " osobowa, zapłacił " +
                                    std::to_string(wartosc_zamowienia));
            }
        }

        if(ewakuacja) continue;
    
        //szukanie odpowiedniego stolika dla klienta i powiadomienie o tym pracownika jeżeli się znalazło
        if (!kolejka.empty()) {
            for (size_t i = 0; i < kolejka.size(); i++) {
                if (ewakuacja) break;

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

                semaforOpusc(sem_id, SEM_MAIN);
                if (pam->pozar) {
                    ewakuacja = 1;
                }
                semaforPodnies(sem_id, SEM_MAIN);

                if (przypisane_id != -1) {
                    if (!ewakuacja) {
                        logger("Kasjer: Przypisano stolik " + std::to_string(przypisane_id) + " typu: " + std::to_string(przypisany_typ) + " dla: " + std::to_string(k.pid));
                        (void)wyslij_komunikat_przerywalnie(kol_id, TYP_PRACOWNIK, k.pid, k.wielkosc_grupy, przypisany_typ, przypisane_id, 0, pam);
                    }
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