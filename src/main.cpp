//main.cpp - główny plik symulacji

#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int sem_id, kol_id, pam_id;
std::vector<pid_t> procesy_potomne;
std::mutex mutex_procesy;
pid_t kasjer_pid = 0;
pid_t pracownik_pid = 0;
volatile int koniec = 0; //flaga do handera
std::atomic<int> liczba_aktywnych_klientow{0};

//usuwanie klientów zombie
void watekSprzatajacy() {
    while(true) {
        if (koniec) {
            return;
        }

        int status;
        pid_t zakonczony = waitpid(-1, &status, WNOHANG);

        if (zakonczony > 0) {
            std::lock_guard<std::mutex> lock(mutex_procesy);
            auto it = std::remove(procesy_potomne.begin(), procesy_potomne.end(), zakonczony);
            if (it != procesy_potomne.end()) {
                procesy_potomne.erase(it, procesy_potomne.end());
            }
            liczba_aktywnych_klientow--;
        }
        else {
            usleep(50000);
        }
    }
}

//handler sygnału przerwania symulacji
void przerwanie(int sig) {
    koniec = 1;
}

int main() {
    signal(SIGINT, przerwanie);
    signal(SIGTERM, przerwanie);
    srand(time(NULL));

    tabula_rasa();

    sem_id = utworzSemafor();
    pam_id = alokujPamiec();
    kol_id = utworzKolejke();

    PamiecDzielona* pam = dolaczPamiec(pam_id);
    
    //inicjalizacja elementów z pamięci dzielonej
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
    pam->utarg = 0;

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

    //sprzątanie procesów zombie jako osobny wątek
    std::thread t(watekSprzatajacy);
    t.detach();

    //kierownik odpalany manualnie w osobnej konsoli do wydawania poleceń

    usleep(1000000); //opóźnienie dla estetyki w konsoli, żeby kasjer i pracownik byli pierwsi
    //klienci
    while(!koniec) {

        if (pam->pozar) {
            break;
        }

        if (liczba_aktywnych_klientow < ILOSC_KLIENTOW) {
            int wielkosc_grupy = (rand() % 4) + 1;
            std::string wielkosc = std::to_string(wielkosc_grupy);

            pid = fork();
            if (pid == 0) {
                execl("./klient", "klient", wielkosc.c_str(), NULL);
                perror("Błąd execl klient");
                exit(1);
            }
            else if (pid > 0) {
                std::lock_guard<std::mutex> lock(mutex_procesy);
                procesy_potomne.push_back(pid);
                liczba_aktywnych_klientow++;
                usleep(1000);
            }
            else {
                if (errno == EAGAIN) {
                    usleep(50000);
                    continue;
                }
                else {
                    perror("Błąd fork");
                    break;
                }
            }
        }
        else {
            usleep(10000);
        }
    }

    //oczekiwanie na sygnały
    while(true) {
        if (koniec) {
            break;
        }
        if (pam->pozar) {
            int status_kasjer = kill(kasjer_pid, 0);
            int status_pracownik = kill(pracownik_pid, 0);
            if (status_kasjer == -1 && errno == ESRCH &&
                status_pracownik == -1 && errno == ESRCH) {
                    usleep(500000);
                    break;
                }
        }
        usleep(1000000);
    }

    long ostateczny_utarg = pam->utarg;

    //sprzątanie po symulacji
    {
        std::lock_guard<std::mutex> lock(mutex_procesy);
        for (pid_t pid : procesy_potomne) {
            kill(pid, SIGKILL);
        }
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

    std::string utarg_info = "Koniec dnia. Utarg wyniósł: " + std::to_string(ostateczny_utarg) + " PLN";
    logger(utarg_info);
    logger("Symulacja zakończona i wszystko sprzątnięte");
    return 0;
}
