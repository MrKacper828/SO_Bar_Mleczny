//pracownik.cpp - proces i logika pracownika

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <atomic>
#include <algorithm>
#include <sys/msg.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

//odbieranie sygnałów kierownika
volatile sig_atomic_t flaga_podwojenie = 0;
volatile sig_atomic_t flaga_rezerwacja = 0;
volatile sig_atomic_t flaga_pozar = 0;

void handler_sygnalow(int signum) {
    if (signum == SIGUSR1) {
        flaga_podwojenie = 1;
    }
    else if (signum == SIGUSR2) {
        flaga_rezerwacja = 1;
    }
    else if (signum == SIGTERM) {
        flaga_pozar = 1;
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handler_sygnalow;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    semaforOpusc(sem_id, SEM_MAIN);
    pam->pracownik_pid = getpid();
    semaforPodnies(sem_id, SEM_MAIN);

    logger("Pracownik: Zaczynam pracę");

    Komunikat zamowienie;
    while (true) {
        if (flaga_pozar) {
            logger("Pracownik: Otrzymałem od Kierownika sygnał Pożar. Ewakuacja");

            while(true) {
                semaforOpusc(sem_id, SEM_MAIN);
                int pozostalo = pam->liczba_klientow;
                semaforPodnies(sem_id, SEM_MAIN);

                if (pozostalo <= 0) {
                    break;
                }
                usleep(50000);
            }
            usleep(200000);
            logger("Pracownik: Pusto. Uciekam po Kasjerze");
            break;
        }

        //podwojenie liczby stolików od kierownika
        if (flaga_podwojenie) {
            flaga_podwojenie = 0;
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

        if (flaga_rezerwacja) {
            flaga_rezerwacja = 0;
            //stała rezerwacja 2 stolików 4 osobowych na sygnał 2 od kierownika z zabezpieczeniem jak już braknie stolików
            int chciana_ilosc = 2;
            int typ_rez = 4;

            semaforOpusc(sem_id, SEM_STOLIKI);
            int dostepne_do_rezerwacji = 0;
            for (int i=0; i < STOLIKI_X4; i++) {
                if (!pam->stoliki_x4[i].zarezerwowany) {
                    dostepne_do_rezerwacji++;
                }
            }
            semaforPodnies(sem_id, SEM_STOLIKI);

            int ilosc_do_wykonania = std::min(chciana_ilosc, dostepne_do_rezerwacji);

            if (ilosc_do_wykonania == 0) {
                logger("Pracownik (SIGUSR2): Brak wolnych stolików do rezerwacji");
            }
            else {
                logger("Pracownik (SIGUSR2): Rezerwuję " + std::to_string(ilosc_do_wykonania) + " stolików typu " + std::to_string(typ_rez));

                semaforOpusc(sem_id, SEM_MAIN);
                pam->blokada_rezerwacyjna = true;
                semaforPodnies(sem_id, SEM_MAIN);

                int zarezerwowano = 0;
                while (zarezerwowano < ilosc_do_wykonania) {
                    if (flaga_pozar) {
                        break;
                    }

                    semaforOpusc(sem_id, SEM_STOLIKI);
                    for (int i = 0; i < STOLIKI_X4; i++) {
                        if (!pam->stoliki_x4[i].zarezerwowany && pam->stoliki_x4[i].ile_zajetych_miejsc == 0) {
                            pam->stoliki_x4[i].zarezerwowany = true;
                            zarezerwowano++;
                            if (zarezerwowano == ilosc_do_wykonania) {
                                break;
                            }
                        }
                    }
                    semaforPodnies(sem_id, SEM_STOLIKI);

                    //obsługa klientów chcących dostać jedzenie i tych oddających talerze w trakcie rezerwacji
                    if (zarezerwowano < ilosc_do_wykonania) {
                        while (msgrcv(kol_id, &zamowienie, ROZMIAR_KOM, TYP_PRACOWNIK, IPC_NOWAIT) != -1) {
                            if (zamowienie.id_stolika == 200) {
                                wyslijKomunikat(kol_id, zamowienie.nadawca, getpid(), 0, 0, 0, 0);
                            }
                            else if (zamowienie.id_stolika < 100) {
                                wyslijKomunikat(kol_id, zamowienie.nadawca, getpid(), 1, zamowienie.typ_stolika, zamowienie.id_stolika, 0);
                            }
                        }
                        usleep(50000);
                    }
                }

                semaforOpusc(sem_id, SEM_MAIN);
                pam->blokada_rezerwacyjna = false;
                semaforPodnies(sem_id, SEM_MAIN);
                if (!flaga_pozar) {
                    logger("Pracownik: Zakończono rezerwację (SIGUSR2)");
                }
            }
        }

        //normalna obsługa klientów (odbiór naczyń i wydawanie posiłków)
        ssize_t status = msgrcv(kol_id, &zamowienie, ROZMIAR_KOM, TYP_PRACOWNIK, 0);
        if (status == -1) {
            if (errno == EINTR) {
                continue;
            }
            else {
                perror("Błąd msgrcv pracownik");
                break;
            }
        }

        int kod_polecenia = zamowienie.id_stolika;

        if (kod_polecenia == 200) {
            logger("Pracownik: Odbieram naczynia od " + std::to_string(zamowienie.nadawca));
            wyslijKomunikat(kol_id, zamowienie.nadawca, getpid(), 0, 0, 0, 0);
        }
        else if (kod_polecenia < 100) {
            logger("Pracownik: Wydaję posiłek dla " + std::to_string(zamowienie.nadawca) + " (stolik " + std::to_string(kod_polecenia) + ")");
            wyslijKomunikat(kol_id, zamowienie.nadawca, getpid(), 1, zamowienie.typ_stolika, kod_polecenia, 0);
        }
    }
    odlaczPamiec(pam);
    return 0;
}