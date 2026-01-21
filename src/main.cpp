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

//flagi do poprawnej ewakuacji w logach
volatile bool kasjer_zakonczony = false;
volatile bool pracownik_zakonczony = false;

//usuwanie klientów zombie
void watekSprzatajacy() {
    while (!koniec || !kasjer_zakonczony || !pracownik_pid) {
        int status;
        while (true) {
            pid_t zakonczony = waitpid(-1, &status, WNOHANG);
            if (zakonczony > 0) {
                std::lock_guard<std::mutex> lock(mutex_procesy);

                if (zakonczony == kasjer_pid) {
                    kasjer_zakonczony = true;
                }
                else if (zakonczony == pracownik_pid) {
                    pracownik_zakonczony = true;
                }
                //zoptymalizowane usuwanie z wektora
                else {
                    for (size_t i = 0; i < procesy_potomne.size(); ++i) {
                        if (procesy_potomne[i] == zakonczony) {
                            procesy_potomne[i] = procesy_potomne.back();
                            procesy_potomne.pop_back();
                            liczba_aktywnych_klientow--;
                            break;
                        }
                    }
                }
            }
            else {
                break;
            }
            usleep(50000);

            if (koniec && kasjer_zakonczony && pracownik_zakonczony && procesy_potomne.empty()) {
                break;
            }
        }
    }
}

//handler sygnału przerwania symulacji
void przerwanie(int sig) {
    koniec = 1;
}

int main() {
    signal(SIGINT, przerwanie);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    srand(time(NULL));

    tabula_rasa();

    sem_id = utworzSemafor();
    pam_id = alokujPamiec();
    kol_id = utworzKolejke();

    PamiecDzielona* pam = dolaczPamiec(pam_id);
    
    //inicjalizacja elementów z pamięci dzielonej
    semaforOpusc(sem_id, SEM_STOLIKI);
    for (int i = 0; i < STOLIKI_X1; i++) {
        pam->stoliki_x1[i].id = i;
        pam->stoliki_x1[i].pojemnosc_max = 1;
        pam->stoliki_x1[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x1[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x1[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X2; i++) {
        pam->stoliki_x2[i].id = i;
        pam->stoliki_x2[i].pojemnosc_max = 2;
        pam->stoliki_x2[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x2[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x2[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X3; i++) {
        pam->stoliki_x3[i].id = i;
        pam->stoliki_x3[i].pojemnosc_max = 3;
        pam->stoliki_x3[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x3[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x3[i].zarezerwowany = false;
    }
    for (int i = 0; i < STOLIKI_X4; i++) {
        pam->stoliki_x4[i].id = i;
        pam->stoliki_x4[i].pojemnosc_max = 4;
        pam->stoliki_x4[i].ile_zajetych_miejsc = 0;
        pam->stoliki_x4[i].wielkosc_grupy_siedzacej = 0;
        pam->stoliki_x4[i].zarezerwowany = false;
    }
    semaforPodnies(sem_id, SEM_STOLIKI);

    semaforOpusc(sem_id, SEM_MAIN);
    pam->pozar = false;
    pam->podwojenie_X3 = false;
    pam->blokada_rezerwacyjna = false;
    pam->aktualna_liczba_X3 = STOLIKI_X3 / 2;
    pam->liczba_klientow = 0;
    pam->utarg = 0;
    pam->pracownik_pid = 0;
    pam->pgid_grupy = getpgrp();
    semaforPodnies(sem_id, SEM_MAIN);

    std::string zasoby = "Utworzono zasoby, start symulacji pgid: " + std::to_string(pam->pgid_grupy);
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

    usleep(500000); //opóźnienie dla estetyki w konsoli, żeby kasjer i pracownik byli pierwsi
    //klienci
    while(!koniec) {
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            semaforPodnies(sem_id, SEM_MAIN);
            koniec = 1;
            break;
        }
        else {
            semaforPodnies(sem_id, SEM_MAIN);
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
            }
        }
        else {
            usleep(50000);
        }
    }

    logger("Symulacja: Koniec tworzenia klientów. Oczekiwanie na koniec ewakuacji");

    //dla poprawności przy ewakuacji
    int odliczanie = 500;
    while ((!kasjer_zakonczony || !pracownik_zakonczony) && odliczanie > 0) {
        usleep(100000);
        odliczanie--;
    }

    //awaryjne usuwanie
    if (!kasjer_zakonczony) {
        kill(kasjer_pid, SIGKILL);
    }
    if (!pracownik_zakonczony) {
        kill(pracownik_pid, SIGKILL);
    }

    //sprzątanie po symulacji
    {
        std::lock_guard<std::mutex> lock(mutex_procesy);
        for (pid_t pid : procesy_potomne) {
            kill(pid, SIGKILL);
        }
    }

    usleep(1000000);

    semaforOpusc(sem_id, SEM_MAIN);
    long ostateczny_utarg = pam->utarg;
    semaforPodnies(sem_id, SEM_MAIN);

    usunSemafor(sem_id);
    usunKolejke(kol_id);
    zwolnijPamiec(pam_id);

    std::string utarg_info = "Koniec dnia. Utarg wyniósł: " + std::to_string(ostateczny_utarg) + " PLN";
    logger(utarg_info);
    logger("Symulacja zakończona i wszystko sprzątnięte");
    return 0;
}
