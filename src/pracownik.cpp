//pracownik.cpp - proces i logika pracownika

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    signal(SIGINT, SIG_IGN);
    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Pracownik: Zaczynam pracę");

    Komunikat zamowienie;
    while (true) {
        if (odbierzKomunikat(kol_id, TYP_PRACOWNIK, &zamowienie, false)) {

            semaforOpusc(sem_id, SEM_MAIN);
            if (pam->pozar) {
                semaforPodnies(sem_id, SEM_MAIN);
            }
            else {
                semaforPodnies(sem_id, SEM_MAIN);
                int kod_polecenia = zamowienie.id_stolika;

                //podwojenie liczby stolików od kierownika
                if (kod_polecenia == 101) {
                    semaforOpusc(sem_id, SEM_MAIN);
                    if (!pam->podwojenie_X3) {
                        pam->podwojenie_X3 = true;
                        semaforPodnies(sem_id, SEM_MAIN);
                        semaforOpusc(sem_id, SEM_STOLIKI);
                        pam->aktualna_liczba_X3 = STOLIKI_X3;
                        semaforPodnies(sem_id, SEM_STOLIKI);
                        logger("Pracownik: Wykonałem polecenie podwojenia liczby stolików X3");
                    }
                    else {
                        semaforPodnies(sem_id, SEM_MAIN);
                        logger("Pracownik: Nie mogę podwoić liczby stolików X3, bo już to zrobiłem wcześniej");
                    }
                }

                //rezerwacja od kierownika
                else if (kod_polecenia == 102) {
                    int ilosc_rezerwacji = zamowienie.dane;
                    int typ_rezerwacji = zamowienie.typ_stolika;
                    int zarezerwowano = 0;

                    semaforOpusc(sem_id, SEM_STOLIKI);
                    Stolik *tablica_wolne = nullptr;
                    int ilosc_stolikow = 0;
                    if (typ_rezerwacji == 1) {
                        tablica_wolne = pam->stoliki_x1;
                        ilosc_stolikow = STOLIKI_X1;
                    }
                    else if (typ_rezerwacji == 2) {
                        tablica_wolne = pam->stoliki_x2;
                        ilosc_stolikow = STOLIKI_X2;
                    }
                    else if (typ_rezerwacji == 3) {
                        tablica_wolne = pam->stoliki_x3;
                        ilosc_stolikow = pam->aktualna_liczba_X3;
                    }
                    else if (typ_rezerwacji == 4) {
                        tablica_wolne = pam->stoliki_x4;
                        ilosc_stolikow = STOLIKI_X4;
                    }
                    semaforPodnies(sem_id, SEM_STOLIKI);

                    int rezerwacja_wolne = 0;
                    if (tablica_wolne != nullptr) {
                        for (int i = 0; i < ilosc_stolikow; i++) {
                            if (!tablica_wolne[i].zarezerwowany) {
                                rezerwacja_wolne++;
                            }
                        }
                    }

                    if (rezerwacja_wolne == 0) {
                        logger("Pracownik: Wszystkie stoliki typu " + std::to_string(typ_rezerwacji) + " są już zarezerwowane, nie wykonuję zadanej rezerwacji");
                    }
                    else {
                        if (ilosc_rezerwacji > rezerwacja_wolne) {
                            std::string rezerwacja = "Pracownik: Żądana rezerwacja przekracza liczbę dostępnych/niezarezerwowanych danych stolików, rezerwuję wszystkie dostępne danego typu";
                            logger(rezerwacja);
                            ilosc_rezerwacji = rezerwacja_wolne;
                        }
                        logger("Pracownik: Zaczynam rezerwację " + std::to_string(ilosc_rezerwacji) + " stolików typu " + std::to_string(typ_rezerwacji));
                        
                        semaforOpusc(sem_id, SEM_MAIN);
                        pam->blokada_rezerwacyjna = true;
                        semaforPodnies(sem_id, SEM_MAIN);

                        while(zarezerwowano < ilosc_rezerwacji) {
                            semaforOpusc(sem_id, SEM_MAIN);
                            if (pam->pozar) {
                                pam->blokada_rezerwacyjna = false;
                                semaforPodnies(sem_id, SEM_MAIN);
                                logger("Pracownik: Pożar w trakcie rezerwacji, przerywam");
                                break;
                            }
                            else {
                                semaforPodnies(sem_id, SEM_MAIN);
                            }
                        
                            semaforOpusc(sem_id, SEM_STOLIKI);
                            Stolik *tablica = nullptr;

                            if (typ_rezerwacji == 1) {
                                tablica = pam->stoliki_x1;
                            }
                            else if (typ_rezerwacji == 2) {
                                tablica = pam->stoliki_x2;
                            }
                            else if (typ_rezerwacji == 3) {
                                tablica = pam->stoliki_x3;
                            }
                            else if (typ_rezerwacji == 4) {
                                tablica = pam->stoliki_x4;
                            }

                            if (tablica != nullptr) {
                                int petla = 0;
                                if (typ_rezerwacji == 3) {
                                    petla = pam->aktualna_liczba_X3;
                                }
                                else {
                                    petla = ilosc_stolikow;
                                }
                                for (int i = 0; i < petla; i++) {
                                    if (!tablica[i].zarezerwowany && tablica[i].ile_zajetych_miejsc == 0) {
                                        tablica[i].zarezerwowany = true;
                                        zarezerwowano++;
                                        logger("Pracownik: Zarezerwowałem stolik " + std::to_string(i) + " typu " + std::to_string(typ_rezerwacji));
                                        if (zarezerwowano == ilosc_rezerwacji) {
                                            break;
                                        }
                                    }
                                }
                                semaforPodnies(sem_id, SEM_STOLIKI);
                            }
                            else {
                                semaforPodnies(sem_id, SEM_STOLIKI);
                            }
                            if (zarezerwowano == ilosc_rezerwacji) {
                                break;
                            }

                            //odbiór naczyń i wydanie zaległych posiłków żeby się zwolniły miejsca
                            Komunikat zwalnianie;
                            if (odbierzKomunikat(kol_id, TYP_PRACOWNIK, &zwalnianie, false)) {
                                int kod = zwalnianie.id_stolika;
                                if (kod == 200) {
                                    logger("Pracownik: W trakcie rezerwacji odbieram naczynia od " + std::to_string(zwalnianie.nadawca));
                                    wyslijKomunikat(kol_id, zwalnianie.nadawca, getpid(), 0, 0, 0, 0);
                                }
                                else if (kod < 100) {
                                    logger("Pracownik: W trakcie rezerwacji wydaję posiłek dla " + std::to_string(zwalnianie.nadawca));
                                    wyslijKomunikat(kol_id, zwalnianie.nadawca, getpid(), 1, zwalnianie.typ_stolika, kod, 0);
                                }
                            }
                            usleep(50000);
                        }
                        semaforOpusc(sem_id, SEM_MAIN);
                        pam->blokada_rezerwacyjna = false;
                        semaforPodnies(sem_id, SEM_MAIN);

                        semaforOpusc(sem_id, SEM_MAIN);
                        if (!pam->pozar) {
                            semaforPodnies(sem_id, SEM_MAIN);
                            std::string sukces = "Pracownik: Rezerwacja zakończna " + std::to_string(zarezerwowano) + " stolików typu " + std::to_string(typ_rezerwacji) + " można wznowić przyjmowanie klientów"; 
                            logger(sukces);
                        }
                        else {
                            semaforPodnies(sem_id, SEM_MAIN);
                        }
                    }
                }

                //ovieranie naczyń od klientów
                else if (kod_polecenia == 200) {
                    pid_t klient_pid = zamowienie.nadawca;
                    std::string naczynia = "Pracownik: Odebrałem naczynia od klienta " + std::to_string(klient_pid);
                    logger(naczynia);

                    wyslijKomunikat(kol_id, klient_pid, getpid(), 0, 0, 0, 0);
                }

                //wydawanie posiłków dla klientów
                else {
                    pid_t klient_pid = zamowienie.nadawca;
                    int typ_stolika = zamowienie.typ_stolika;

                    std::string log = "Pracownik: Posiłek gotowy dla " + std::to_string(klient_pid) + " można kierować się do stolika";
                    logger(log);

                    wyslijKomunikat(kol_id, klient_pid, getpid(), 1, typ_stolika, kod_polecenia, 0);
                }
            }
        }
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            semaforPodnies(sem_id, SEM_MAIN);
            logger("Pracownik: Pożar! Ewakuacja klientów!");
            while (true) {
                semaforOpusc(sem_id, SEM_MAIN);
                int pozostalo = pam->liczba_klientow;
                semaforPodnies(sem_id, SEM_MAIN);
                if (pozostalo <= 0) {
                    break;
                }
                usleep(100000);
            }
            logger("Pracownik: Wszyscy klienci ewakuowani, uciekam");
            break;
        }
        else {
            semaforPodnies(sem_id, SEM_MAIN);
        }
        usleep(10000);
    }

    odlaczPamiec(pam);
    return 0;
}