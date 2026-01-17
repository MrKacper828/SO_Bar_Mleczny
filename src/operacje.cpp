//operacje.cpp - funkcje semaforów, pamięci dzielonej i kolejek komunikatów
//                  z obsługą błędów perror

#include "operacje.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <cstdio>
#include <cstdlib>

//tworzenie poprawnych kluczy
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
    int sem_id = semget(klucz, 3, IPC_CREAT | 0600);
    if (sem_id == -1) {
        perror("Błąd tworzenia semafora(semget IPC_CREAT)");
        exit(1);
    }

    if (semctl(sem_id, SEM_MAIN, SETVAL, 1) == -1) {
        perror("Błąd ustawinia semafora main na otwarty(semctl SETVAL)");
        exit(1);
    }
    if (semctl(sem_id, SEM_STOLIKI, SETVAL, 1) == -1) {
        perror("Błąd ustawinia semafora stoliki na otwarty(semctl SETVAL)");
        exit(1);
    }
    if (semctl(sem_id, SEM_LIMIT, SETVAL, LIMIT_W_BARZE) == -1) {
        perror("Błąd ustawinia semafora limitu(semctl SETVAL)");
        exit(1);
    }
    return sem_id;
}

void usunSemafor(int sem_id) {
    if (semctl(sem_id, 0, IPC_RMID, NULL) == -1) {
        perror("Nieudane usuniecie semafora(semctl IPC_RMID)");
    }
}

void semaforPodnies(int sem_id, int sem_num) { //V
    struct sembuf operacje;
    operacje.sem_num = sem_num;
    operacje.sem_op = 1;
    operacje.sem_flg = 0;

    while (semop(sem_id, &operacje, 1) == -1) {
        if (errno == EINTR) {
            continue;
        }
        else {
            perror("Błąd poniesienia semafora(semop 1)");
            break;
        }
    }
}

void semaforOpusc(int sem_id, int sem_num) { //P
    struct sembuf operacje;
    operacje.sem_num = sem_num;
    operacje.sem_op = -1;
    operacje.sem_flg = 0;

    while (semop(sem_id, &operacje, 1) == -1) {
        if (errno == EINTR) {
            continue;
        }
        else {
            perror("Błąd opuszczenia semafora(semop -1)");
            break;
        }
    }
}

int polaczSemafor() {
    key_t klucz = stworzKlucz(KLUCZ_SEM);
    int sem_id = semget(klucz, 0, 0600);
    if (sem_id == -1) {
        perror("Błąd połączenia semafora(semget)");
        exit(1);
    }
    return sem_id;
}


//pamięć dzielona
int alokujPamiec() {
    key_t klucz = stworzKlucz(KLUCZ_PD);
    int pam_id = shmget(klucz, sizeof(PamiecDzielona), IPC_CREAT | 0600);
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

PamiecDzielona* dolaczPamiec(int pam_id) {
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

int polaczPamiec() {
    key_t klucz = stworzKlucz(KLUCZ_PD);
    int kol_id = shmget(klucz, sizeof(PamiecDzielona), 0600);
    if (kol_id == -1) {
        perror("Błąd połączenia pamięci dzielonej(shmget)");
        exit(1);
    }
    return kol_id;
}


//kolejki
int utworzKolejke() {
    key_t klucz = stworzKlucz(KLUCZ_KOL);
    int kol_id = msgget(klucz, IPC_CREAT | 0600);
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
    int kol_id = msgget(klucz, 0600);
    if (kol_id == -1) {
        perror("Błąd połączenia kolejki(msgget)");
        exit(1);
    }
    return kol_id;
}

//komunikaty
void wyslijKomunikat(int kol_id, long mtyp, pid_t nadawca, int dane, int typ_stolika, int id_stolika, int id_dania) {
    Komunikat kom;
    kom.mtype = mtyp;
    kom.nadawca = nadawca;
    kom.dane = dane;
    kom.typ_stolika = typ_stolika;
    kom.id_stolika = id_stolika;
    kom.id_dania = id_dania;
    if (msgsnd(kol_id, &kom, ROZMIAR_KOM, 0) == -1) {
        perror("Nieudana próba wysłania wiadomości(msgsnd)");
    }
}

bool odbierzKomunikat(int kol_id, long mtyp, Komunikat* buf, bool czekaj) {
    int flaga = 0;
    if (!czekaj) {
        flaga = IPC_NOWAIT;
    }

    if (msgrcv(kol_id, buf, ROZMIAR_KOM, mtyp, flaga) != -1) {
        return true;
    }
    return false;
}