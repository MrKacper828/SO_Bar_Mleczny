//struktury.h - struktury w pamięci dzielonej i kolejki

#ifndef STRUKTURY_H
#define STRUKTURY_H

#include <sys/types.h>

//struktury w pamięci dzielonej
//definicje kluczy
//struktury wiadomości do kolejek komunikatów

//klucze 
const int KLUCZ_PD = 123;
const int KLUCZ_SEM = 234;

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