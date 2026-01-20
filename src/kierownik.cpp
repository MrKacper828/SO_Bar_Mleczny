//kierownik.cpp - proces i logika kierownika (odpalany osobno)

#include <iostream>
#include <unistd.h>
#include <sys/sem.h>
#include <signal.h>
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

int main() {
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    std::cout << "Kierownik: Potrzebuję pracownika żeby działać" << std::endl;
    while (pam->pracownik_pid == 0) {
        usleep(100000);
    }
    semaforOpusc(sem_id, SEM_MAIN);
    pid_t pracownik_pid = pam->pracownik_pid;
    semaforPodnies(sem_id, SEM_MAIN);
    std::cout << "Kierownik: Pracownik się pojawił: " << pracownik_pid << std::endl;

    semaforOpusc(sem_id, SEM_STOLIKI);
    stanStolikow(pam);
    semaforPodnies(sem_id, SEM_STOLIKI);
    
    int wybor;
    while (true) {
        std::cout << "Opcje Kierownika" << std::endl;
        std::cout << "1 - podwojenie liczby stolików X3 (SIGUSR1)" << std::endl;
        std::cout << "2 - rezerwacja stałej wartości stolików (SIGUSR2)" << std::endl;
        std::cout << "3 - pożar i ewakuacja (SIGTERM)" << std::endl;
        std::cout << "4 - aktualny stan stolików" << std::endl;
        std::cout << "0 - wyjście" << std::endl;

        std::cout << "Podaj wybraną opcję: ";

        if (!(std::cin >> wybor)) {
            std::cout << "niepoprawne dane wejściowe: ";
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }
        
        if (wybor == 1) {
            logger("Kierownik: Wysyłam do pracownika SIGUSR1 - rozkaz podwojenia liczby stolików X3");
            kill(pracownik_pid, SIGUSR1);
        }

        else if (wybor == 2) {
            logger("Kierownik: Wysyłam do pracownika SIGUSR2 - rezerwację ustaloną w kodzie pracownika");
            kill(pracownik_pid, SIGUSR2);
        }

        else if (wybor == 3) {
            logger("Kierownik: Ogłaszam pożar i ewakuację!");
            semaforOpusc(sem_id, SEM_MAIN);
            pam->pozar = true;
            pid_t gid = pam->pgid_grupy;
            semaforPodnies(sem_id, SEM_MAIN);

            kill(pracownik_pid, SIGTERM);
            usleep(10000);

            if (gid > 0) {
                kill(-gid, SIGTERM);
            }

            logger("Kierownik: Czekam aż wszyscy bezpiecznie opuszczą bar");

            while (kill(pracownik_pid, 0) == 0) {
                usleep(100000);
            }

            usleep(200000);
            logger("Kierownik: Lokal pusty, wszyscy ewakuowani. Uciekam i zamykam");
            break;
        }

        else if (wybor == 4) {
            semaforOpusc(sem_id, SEM_STOLIKI);
            stanStolikow(pam);
            semaforPodnies(sem_id, SEM_STOLIKI);
        }

        else if (wybor == 0) {
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