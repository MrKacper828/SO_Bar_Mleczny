//kierownik.cpp - proces i logika kierownika (odpalany osobno)

#include <iostream>
#include <unistd.h>
#include <sys/sem.h>
#include <cerrno>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

//funkcja do wyświetlania ile stolików na ile dostępnych jest zarezerwowanych
void stanStolikow(PamiecDzielona *pam) {
    int wolne_X1 = 0;
    int wolne_X2 = 0;
    int wolne_X3 = 0;
    int wolne_X4 = 0;

    for (int i = 0; i < STOLIKI_X1; i++) {
        if (!pam->stoliki_x1[i].zarezerwowany) {
            wolne_X1++;
        }
    }
    for (int i = 0; i < STOLIKI_X2; i++) {
        if (!pam->stoliki_x2[i].zarezerwowany) {
            wolne_X2++;
        }
    }
    for (int i = 0; i < pam->aktualna_liczba_X3; i++) {
        if (!pam->stoliki_x3[i].zarezerwowany) {
            wolne_X3++;
        }
    }
    for (int i = 0; i < STOLIKI_X4; i++) {
        if (!pam->stoliki_x4[i].zarezerwowany) {
            wolne_X4++;
        }
    }

    std::cout << "Aktualnie dostępne stoliki (niezarezerwowane):" << std::endl;
    std::cout << "1 osobowe: " << wolne_X1 << "/" << STOLIKI_X1 << std::endl;
    std::cout << "2 osobowe: " << wolne_X2 << "/" << STOLIKI_X2 << std::endl;
    std::cout << "3 osobowe: " << wolne_X3 << "/" << pam->aktualna_liczba_X3 << std::endl;
    std::cout << "4 osobowe: " << wolne_X4 << "/" << STOLIKI_X4 << std::endl;
}

void bladWejscia(const char* komunikat) {
    std::cout << "\033[31m[błąd danych]: " << komunikat << "\033[0m" << std::endl;
    std::cin.clear();
    std::cin.ignore(1000, '\n');
}

int main() {
    int sem_id = polaczSemafor();
    int kol_id = polaczKolejke();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    semaforOpusc(sem_id, SEM_STOLIKI);
    stanStolikow(pam);
    semaforPodnies(sem_id, SEM_STOLIKI);
    
    int wybor;
    while (true) {
        std::cout << "Opcje Kierownika" << std::endl;
        std::cout << "1 - podwojenie liczby stolików X3" << std::endl;
        std::cout << "2 - rezerwacja stolików" << std::endl;
        std::cout << "3 - pożar i ewakuacja" << std::endl;
        std::cout << "4 - aktualny stan stolików" << std::endl;

        std::cout << "Podaj numer sygnału: ";

        while (!(std::cin >> wybor)) {
            std::cout << "niepoprawne dane wejściowe: ";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
        }
        
        if (wybor == 1) {
            logger("Kierownik: Wysyłam do pracownika rozkaz podwojenia liczby stolików X3");
            wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), 0, 0, 101, 0);
        }

        else if (wybor == 2) {
            int typ_stolika;
            int ilosc;

            while (1) {
                std::cout << "Jaki typ stolika zarezerować (1, 2, 3, 4): ";
                if (!(std::cin >> typ_stolika)) {
                    bladWejscia("to nie jest liczba");
                    continue;
                }
                if (typ_stolika < 1 || typ_stolika > 4) {
                    bladWejscia("nie ma takiego typu stolika");
                    continue;
                }

                std::cout << "Ile stolików typu " + std::to_string(typ_stolika) << " zarezerować: ";
                if (!(std::cin >> ilosc)) {
                    bladWejscia("to nie jest liczba");
                    continue;
                }
                if (ilosc < 0) {
                    bladWejscia("ilość musi być >= 0");
                    continue;
                }
                break;
            }

            std::string kom = "Kierownik: wsyłam polecenie rezerwacji " + std::to_string(ilosc) + " stolików typu " + std::to_string(typ_stolika);
            logger(kom);
            wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), ilosc, typ_stolika, 102, 0);
            usleep(500000);
        }

        else if (wybor == 3) {
            semaforOpusc(sem_id, SEM_MAIN);
            pam->pozar = true;
            semaforPodnies(sem_id, SEM_MAIN);
            logger("Kierownik: Ogłaszam pożar i ewakuację!");

            while (true) {
                struct sembuf operacje;
                operacje.sem_num = SEM_MAIN;
                operacje.sem_op = -1;
                operacje.sem_flg = 0;

                if (semop(sem_id, &operacje, 1) == -1) {
                    if (errno == EIDRM || errno == EINVAL) {
                        break;
                    }
                }

                int pozostalo = pam->liczba_klientow;

                operacje.sem_op = 1;
                semop(sem_id, &operacje, 1);

                if (pozostalo <= 0) {
                    break;
                }
                usleep(500000);
            }
            logger("Kierownik: Lokal pusty, uciekam i zamykam bar");
            break;
        }

        else if (wybor == 4) {
            semaforOpusc(sem_id, SEM_STOLIKI);
            stanStolikow(pam);
            semaforPodnies(sem_id, SEM_STOLIKI);
        }

        else {
            std::cout << "nie ma takiej opcji";
            continue;
        }
    }

    odlaczPamiec(pam);
    return 0;
}