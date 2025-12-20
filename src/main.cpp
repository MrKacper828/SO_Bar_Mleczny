//main.cpp - główny plik programu

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
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

    tabula_rasa();

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
    
    pid_t pid;

    //kasjer
    pid = fork();
    if (pid == 0) {
        execl("./kasjer", "kasjer", NULL);
        perror("Błąd execl kasjer");
        exit(1);
    }
    else {
        procesy_potomne.push_back(pid);
    }

    //klient
    pid = fork();
    if (pid == 0) {
        execl("./klient", "klient", NULL);
        perror("Błąd execl klient");
        exit(1);
    }
    else {
        procesy_potomne.push_back(pid);
    }

    //klienci
    int liczba_klientow = 15;
    while (liczba_klientow > 0) {
        int wielkosc_grupy = (rand() % 4) + 1;
        std::string wielkosc = std::to_string(wielkosc_grupy);

        pid = fork();
        if (pid == 0) {
            execl("./klient", "klient", wielkosc.c_str(), NULL);
            perror("Błąd execl klient");
            exit(1);
        }
        else {
            procesy_potomne.push_back(pid);
        }
        usleep(500000 + (rand() % 1000000));
        liczba_klientow--;
    }

    std::cout << "Symulacja koniec" << std::endl;
    while (wait(NULL) > 0);

    zakonczenie(0);
    return 0;
}