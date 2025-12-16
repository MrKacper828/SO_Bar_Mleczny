//struktury.h - struktury w pamięci dzielonej i kolejki

#ifndef STRUKTURY_H
#define STRUKTURY_H

#include <sys/types.h>

//struktury w pamięci dzielonej
//definicje kluczy
//struktury wiadomości do kolejek komunikatów

//klucze
#define KLUCZ_SCIEZKA "."
const int KLUCZ_PD = 'P'; //klucz dla pamięci dzielonej
const int KLUCZ_SEM = 'S'; //klucz dla semafora
const int KLUCZ_KOL = 'K'; //klucz dla kolejek

const int MAX_STOLIKOW = 100;

struct Stolik {
    int id;
    int pojemnosc;
    int ile_siedzi;
    int rozmiar_grupy;
    bool rezerwacja;
};

struct PamiecDzielona {
    Stolik stoliki[MAX_STOLIKOW];
    int liczba_stolikow;

    bool pozar;
    bool podwojenie_X3;
    pid_t pid_pracownika;
};

struct Komunikat {
    long mtype; //typ grupy (1, 2, 3, 4)
    pid_t pid_grupy;
};


#endif