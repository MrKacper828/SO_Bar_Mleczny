//main.cpp - główny plik programu

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <signal.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int sem_id, kol_id, pam_id;
std::vector<pid_t> procesy_potomne;

void zakonczenie(int sig) {
    std::cout << "Zakończenie symulacji" << std::endl;

    for (pid_t pid : procesy_potomne) {
        kill(pid, SIGKILL);
    }

    usunSemafor(sem_id);
    usunKolejke(kol_id);
    zwolnijPamiec(pam_id);

    std::cout << "Wszystko zwolnione, koniec symulacji" << std::endl;
    exit(0);
}

int main() {
    signal(SIGINT, zakonczenie);
    srand(time(NULL));

    sem_id = utworzSemafor();
    pam_id = alokujPamiec();
    kol_id = utworzKolejke();

    PamiecDzielona* pam = dolaczPamiec(pam_id);
    pam->wolne_x1 = STOLIKI_X1;
    pam->wolne_x2 = STOLIKI_X2;
    pam->wolne_x3 = STOLIKI_X3;
    pam->wolne_x4 = STOLIKI_X4;
    pam->pozar = false;
    pam->podwojenie_X3 = false;

    std::cout << "Utworzono zasoby" << std::endl;
    sleep(10);

    zakonczenie(0);
    return 0;
}