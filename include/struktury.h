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

//ilość stolików w restauracji
const int STOLIKI_X1 = 4;
const int STOLIKI_X2 = 4;
const int STOLIKI_X3 = 2;
const int STOLIKI_X4 = 2;

struct PamiecDzielona {
    int wolne_x1;
    int wolne_x2;
    int wolne_x3;
    int wolne_x4;

    bool pozar;
    bool podwojenie_X3;
    pid_t pid_pracownika;
    pid_t pid_kasjera;
};

//dla msgsnd
struct Komunikat {
    long mtype; //typ grupy (1, 2, 3, 4)
    pid_t nadawca;
    int kod;
};
const int ROZMIAR_KOM = sizeof(Komunikat) - sizeof(long);

#endif