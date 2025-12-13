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

//semafor
int utworzSemafor() {
    int sem_id = semget(KLUCZ_SEM, 1, IPC_CREAT | 0666);
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
    if (semctl(sem_id, 0, IPC_RMID) == -1) {
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

//pamięć dzielona
int alokujPamiec() {
    return 0;
}

void zwolnijPamiec() {

}

//kolejki
int utworzKolejke() {
    return 0;
}

void usunKolejke() {

}