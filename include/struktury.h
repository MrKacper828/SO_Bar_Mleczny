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

struct Stolik {
    int id;
    int pojemnosc;
    int ile_siedzi;
    bool rezerwacja;
};

struct PamiecDzielona {
    Stolik stoliki;
    int liczba_stolikow;

    bool pozar;
    bool podwojenie_X3;
};

struct Komunikat {
    int id_klienta;
};


#endif