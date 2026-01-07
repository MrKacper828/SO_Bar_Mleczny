//main.cpp - główny plik symulacji

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
pid_t kasjer_pid = 0;
pid_t pracownik_pid = 0;

void zakonczenie(int sig) {
    std::string zakonczenie = "Zakończenie symulacji";
    logger(zakonczenie);

    for (pid_t pid : procesy_potomne) {
        kill(pid, SIGKILL);
    }
    if (kasjer_pid > 0) {
        kill(kasjer_pid, SIGKILL);
    }
    if (pracownik_pid > 0) {
        kill(pracownik_pid, SIGKILL);
    }

    usunSemafor(sem_id);
    usunKolejke(kol_id);
    zwolnijPamiec(pam_id);

    std::string koniec = "Wszystko zwolnione, koniec symulacji";
    logger(koniec);
    exit(0);
}

int main() {
    signal(SIGINT, zakonczenie);
    signal(SIGTERM, zakonczenie);
    srand(time(NULL));

    tabula_rasa();

    sem_id = utworzSemafor();
    pam_id = alokujPamiec();
    kol_id = utworzKolejke();

    PamiecDzielona* pam = dolaczPamiec(pam_id);
    
    for (int i = 0; i < STOLIKI_X1; i++) {
        pam->stoliki_x1[i].id = i + 1;
        pam->stoliki_x1[i].pojemnosc_max = 1;
        pam->stoliki_x1[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x1[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x1[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X2; i++) {
        pam->stoliki_x2[i].id = i + 1;
        pam->stoliki_x2[i].pojemnosc_max = 2;
        pam->stoliki_x2[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x2[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x2[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X3; i++) {
        pam->stoliki_x3[i].id = i + 1;
        pam->stoliki_x3[i].pojemnosc_max = 3;
        pam->stoliki_x3[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x3[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x3[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X4; i++) {
        pam->stoliki_x4[i].id = i + 1;
        pam->stoliki_x4[i].pojemnosc_max = 4;
        pam->stoliki_x4[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x4[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x4[i].zarezerwowany = false;
    }

    pam->pozar = false;
    pam->podwojenie_X3 = false;
    pam->blokada_rezerwacyjna = false;
    pam->aktualna_liczba_X3 = STOLIKI_X3 / 2;
    pam->liczba_klientow = 0;

    std::string zasoby = "Utworzono zasoby";
    logger(zasoby);
    
    pid_t pid;

    //kasjer
    pid = fork();
    if (pid == 0) {
        execl("./kasjer", "kasjer", NULL);
        perror("Błąd execl kasjer");
        exit(1);
    }
    else {
        kasjer_pid = pid;
    }

    //pracownik
    pid = fork();
    if (pid == 0) {
        execl("./pracownik", "pracownik", NULL);
        perror("Błąd execl pracownik");
        exit(1);
    }
    else {
        pracownik_pid = pid;
    }

    //kierownik odpalany manualnie w osobnej konsoli do wydawania poleceń

    sleep(1);
    //klienci
    while (true) {

        if (pam->pozar) {
            break;
        }
        
        //usuwanie klientów którzy już opuścili bar
        int status;
        pid_t zakonczony_proces;
        while ((zakonczony_proces = waitpid(-1, &status, WNOHANG)) > 0) {
            for (size_t i = 0; i < procesy_potomne.size(); i++) {
                if (procesy_potomne[i] == zakonczony_proces) {
                    procesy_potomne.erase(procesy_potomne.begin() + i);
                    break;
                }
            }
        }

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
    }

    for (size_t i = 0; i < procesy_potomne.size(); i++) {
        wait(NULL);
    }

    if (kasjer_pid > 0) {
        waitpid(kasjer_pid, NULL, 0);
    }
    if (pracownik_pid > 0) {
        waitpid(pracownik_pid, NULL, 0);
    }
    odlaczPamiec(pam);
    usunSemafor(sem_id);
    usunKolejke(kol_id);
    zwolnijPamiec(pam_id);
    logger("Symulacja zakończona");
    return 0;
}