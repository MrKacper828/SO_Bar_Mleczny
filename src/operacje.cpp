//operacje.cpp - funkcje systemowe z operacje.h i obsługa błędów perror

#include "operacje.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>


//definicje z operacje.h
//obsługa błędów perror w tym pliku

key_t stworzKlucz(int klucz_struktury) {
    key_t klucz = ftok(KLUCZ_SCIEZKA, klucz_struktury);
    if (klucz == -1) {
        perror("Błąd wczytania klucza(ftok)");
        exit(1);
    }
    return klucz;
}


//semafor
int utworzSemafor() {
    key_t klucz = stworzKlucz(KLUCZ_SEM);
    int sem_id = semget(klucz, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Błąd tworzenia semafora(semget IPC_CREAT)");
        exit(1);
    }

    if (semctl(sem_id, 0, SETVAL, 1) == -1) {
        perror("Błąd ustawinia semafora na otwarty(semctl SETVAL)");
        exit(1);
    }
    return sem_id;
}

void usunSemafor(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) {
        perror("Nieudane usuniecie semafora(semctl IPC_RMID)");
    }
}

void semaforPodnies(int sem_id) { //V
    struct sembuf operacje;
    operacje.sem_num = 0;
    operacje.sem_op = 1;
    operacje.sem_flg = 0;

    if (semop(sem_id, &operacje, 1) == -1) {
        perror("Błąd poniesienia semafora(semop 1)");
    }
}

void semaforOpusc(int sem_id) { //P
    struct sembuf operacje;
    operacje.sem_num = 0;
    operacje.sem_op = -1;
    operacje.sem_flg = 0;

    if (semop(sem_id, &operacje, 1) == -1) {
        perror("Błąd opuszczenia semafora(semop -1)");
    }
}

int polaczSemafor() {
    key_t klucz = stworzKlucz(KLUCZ_SEM);
    int sem_id = semget(klucz, 0, 0666);
    if (sem_id == -1) {
        perror("Błąd połączenia semafora(semget)");
        exit(1);
    }
    return sem_id;
}


//pamięć dzielona
int alokujPamiec() {
    key_t klucz = stworzKlucz(KLUCZ_PD);
    int pam_id = shmget(klucz, sizeof(PamiecDzielona), IPC_CREAT | 0666);
    if (pam_id == -1) {
        perror("Błąd tworzenia pamięci dzielonej(shmget IPC_CREAT)");
        exit(1);
    }
    return pam_id;
}

void zwolnijPamiec(int pam_id) {
    if (shmctl(pam_id, IPC_RMID, NULL) == -1) {
        perror("Nieudane usuniecie pamięci dzielonej(shmctl IPC_RMID)");
    }
}

PamiecDzielona* dolaczPamiec() {
    key_t klucz = stworzKlucz(KLUCZ_PD);
    int pam_id = shmget(klucz, sizeof(PamiecDzielona), 0666);
    if (pam_id == -1) {
        perror("Błąd połączenia pamięci dzielonej(shmget)");
        exit(1);
    }
    void* pam = shmat(pam_id, NULL, 0);
    if (pam == (void*)-1) {
        perror("Błąd łączenia(shmat)");
        exit(1);
    }
    return (PamiecDzielona*)pam;
}

void odlaczPamiec(PamiecDzielona* adres) {
    if (shmdt(adres) == -1) {
        perror("Nieudane odłączenie pamięci dzielonej(shmdt)");
    }
}


//kolejki
int utworzKolejke() {
    key_t klucz = stworzKlucz(KLUCZ_KOL);
    int kol_id = msgget(klucz, IPC_CREAT | 0666);
    if (kol_id == -1) {
        perror("Błąd tworzenia kolejki(msgget IPC_CREAT)");
        exit(1);
    }
    return kol_id;
}

void usunKolejke(int kol_id) {
    if (msgctl(kol_id, IPC_RMID, NULL) == -1) {
        perror("Nieudane usuniecie kolejki(msgctl IPC_RMID)");
    }
}

int polaczKolejke() {
    key_t klucz = stworzKlucz(KLUCZ_KOL);
    int kol_id = msgget(klucz, 0666);
    if (kol_id == -1) {
        perror("Błąd połączenia kolejki(msgget)");
        exit(1);
    }
    return kol_id;
}