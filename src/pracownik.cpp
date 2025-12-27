//pracownik.cpp - proces i logika pracownika

#include <iostream>
#include <unistd.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Pracownik: Zaczynam pracę");

    Komunikat zamowienie;
    while (true) {
        if (odbierzKomunikat(kol_id, TYP_PRACOWNIK, &zamowienie, false)) {

            if (pam->pozar) {
                
            }
            else {
                int kod_polecenia = zamowienie.id_stolika;

                if (kod_polecenia == 101) {
                    semaforOpusc(sem_id);
                    if (!pam->podwojenie_X3) {
                        pam->podwojenie_X3 = true;
                        pam->aktualna_liczba_X3 = STOLIKI_X3;
                        logger("Pracownik: Wykonałem polecenie podwojenia liczby stolików X3");
                    }
                    else {
                        logger("Pracownik: Nie mogę podwoić liczby stolików X3, bo już to zrobiłem wcześniej");
                    }
                    semaforPodnies(sem_id);
                }

                else if (kod_polecenia == 102) {
                    int ilosc_rezerwacji = zamowienie.dane;
                    int typ_rezerwacji = zamowienie.typ_stolika;
                    int zarezerwowano = 0;

                    logger("Pracownik: Zaczynam rezerwację " + std::to_string(ilosc_rezerwacji) + " stolików typu " + std::to_string(typ_rezerwacji));
                    semaforOpusc(sem_id);

                    Stolik *tablica = nullptr;
                    int aktualna_liczba;

                    if (typ_rezerwacji == 1) {
                        tablica = pam->stoliki_x1;
                        aktualna_liczba = STOLIKI_X1;
                    }
                    else if (typ_rezerwacji == 2) {
                        tablica = pam->stoliki_x2;
                        aktualna_liczba = STOLIKI_X2;
                    }
                    else if (typ_rezerwacji == 3) {
                        tablica = pam->stoliki_x3;
                        aktualna_liczba = pam->aktualna_liczba_X3;
                    }
                    else if (typ_rezerwacji == 4) {
                        tablica = pam->stoliki_x4;
                        aktualna_liczba = STOLIKI_X4;
                    }

                    if (tablica != nullptr) {
                        for (int i = 0; i < aktualna_liczba; i++) {
                            if (!tablica[i].zarezerwowany && tablica[i].ile_zajetych_miejsc == 0) {
                                tablica[i].zarezerwowany = true;
                                zarezerwowano++;
                                logger("Pracownik: Zarezerwowałem stolik " + std::to_string(i) + " typu " + std::to_string(typ_rezerwacji));

                                if (zarezerwowano == ilosc_rezerwacji) {
                                    break;
                                }
                            }
                        }
                    }
                    semaforPodnies(sem_id);
                    if (zarezerwowano < ilosc_rezerwacji) {
                        logger("Pracownik: Niewykonalne wymagania, zarezerwowano: " + std::to_string(zarezerwowano));
                    }
                    else {
                        logger("Pracownik: Rezerwacja zakończona");
                    }
                }

                else if (kod_polecenia == 200) {
                    pid_t klient_pid = zamowienie.nadawca;
                    std::string naczynia = "Pracownik: Odebrałem naczynia od klienta " + std::to_string(klient_pid);
                    logger(naczynia);

                    wyslijKomunikat(kol_id, klient_pid, getpid(), 0, 0, 0);
                }

                else {
                    pid_t klient_pid = zamowienie.nadawca;
                    int typ_stolika = zamowienie.typ_stolika;

                    std::string log = "Pracownik: Posiłek gotowy dla " + std::to_string(klient_pid) + " można kierować się do stolika";
                    logger(log);

                    wyslijKomunikat(kol_id, klient_pid, getpid(), 1, typ_stolika, kod_polecenia);
                }
            }
        }
        if (pam->pozar) {
            logger("Pracownik: Pożar! Ewakuacja klientów!");
            while (true) {
                semaforOpusc(sem_id);
                int pozostalo = pam->liczba_klientow;
                semaforPodnies(sem_id);
                if (pozostalo <= 0) {
                    break;
                }
                usleep(100000);
            }
            logger("Pracownik: Wszyscy klienci ewakuowani, uciekam");
            break;
        }
        usleep(10000);
    }

    odlaczPamiec(pam);
    return 0;
}