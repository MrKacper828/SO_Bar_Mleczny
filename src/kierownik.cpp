//kierownik.cpp - proces i logika kierownika (odpalany osobno)

#include <iostream>
#include <unistd.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    int sem_id = polaczSemafor();
    int kol_id = polaczKolejke();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    std::cout << "Opcje Kierownika" << std::endl;
    std::cout << "1 - podwojenie liczby stolików X3" << std::endl;
    std::cout << "2 - rezerwacja stolików" << std::endl;
    std::cout << "3 - pożar i ewakuacja" << std::endl;

    int wybor;
    while (true) {
        std::cout << "Podaj numer sygnału: ";
        std::cin >> wybor;

        if (wybor == 1) {
            logger("Kierownik: Wysyłam do pracownika rozkaz podwojenia liczby stolików X3");
            wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), 0, 0, 101);
        }

        else if (wybor == 2) {
            int typ_stolika;
            int ilosc;

            std::cout << "Jaki typ stolika zarezerować (1, 2, 3, 4): ";
            std::cin >> typ_stolika;

            if (typ_stolika < 1 || typ_stolika > 4) {
                std::cout << "Niepoprawny typ stolika\n";
                continue;
            }

            std::cout << "Ile stolików typu " + typ_stolika << " zarezerować: ";
            std::cin >> ilosc;

            std::string kom = "Kierownik: wsyłam polecenie rezerwacji " + std::to_string(ilosc) + " stolików typu " + std::to_string(typ_stolika);
            logger(kom);
            wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), ilosc, typ_stolika, 102);
        }

        else if (wybor == 3) {
            semaforOpusc(sem_id);
            pam->pozar = true;
            logger("Kierownik: Ogłaszam pożar i ewakuację!");
            semaforPodnies(sem_id);

            while (true) {
                semaforOpusc(sem_id);
                int pozostalo = pam->liczba_klientow;
                semaforPodnies(sem_id);

                if (pozostalo <= 0) {
                    break;
                }
                usleep(500000);
            }
            logger("Kierownik: Lokal pusty, uciekam i zamykam bar");
            break;
        }

        else {
            std::cout << "nie ma takiej opcji";
            continue;
        }
    }

    odlaczPamiec(pam);
    return 0;
}