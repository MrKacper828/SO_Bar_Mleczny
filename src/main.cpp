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
    std::string zakonczenie = "Zakończenie symulacji";
    logger(zakonczenie);

    for (pid_t pid : procesy_potomne) {
        kill(pid, SIGKILL);
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
    }
    for (int i = 0; i < STOLIKI_X2; i++) {
        pam->stoliki_x2[i].id = i + 1;
        pam->stoliki_x2[i].pojemnosc_max = 2;
        pam->stoliki_x2[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x2[i].wielkosc_grupy_siedzacej = 0;
    }
    for (int i = 0; i < STOLIKI_X3; i++) {
        pam->stoliki_x3[i].id = i + 1;
        pam->stoliki_x3[i].pojemnosc_max = 3;
        pam->stoliki_x3[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x3[i].wielkosc_grupy_siedzacej = 0;
    }
    for (int i = 0; i < STOLIKI_X4; i++) {
        pam->stoliki_x4[i].id = i + 1;
        pam->stoliki_x4[i].pojemnosc_max = 4;
        pam->stoliki_x4[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x4[i].wielkosc_grupy_siedzacej = 0;
    }

    pam->pozar = false;
    pam->podwojenie_X3 = false;
    odlaczPamiec(pam);

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
        procesy_potomne.push_back(pid);
    }

    //pracownik
    pid = fork();
    if (pid == 0) {
        execl("./pracownik", "pracownik", NULL);
        perror("Błąd execl pracownik");
        exit(1);
    }
    else {
        procesy_potomne.push_back(pid);
    }

    //kierownik odpalany w osobnej konsoli manualnie dla przejrzystości

    sleep(1);
    //klienci
    int liczba_klientow = 100;
    while (liczba_klientow > 0) {
        int wielkosc_grupy = (rand() % 4) + 1;
        //int wielkosc_grupy = 1;
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

    for (int i = 0; i < liczba_klientow; i++) {
        wait(NULL);
    }
    sleep(liczba_klientow);
    std::string sym_koniec = "Wszyscy klienci obsłużeni symulacja koniec";
    logger(sym_koniec);
    while (true) {
        sleep(1);
    }

    zakonczenie(0);
    return 0;
}