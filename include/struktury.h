//struktury.h - struktury w pamięci dzielonej i kolejki

#ifndef STRUKTURY_H
#define STRUKTURY_H

#include <sys/types.h>

//struktury w pamięci dzielonej
//definicje kluczy
//struktury wiadomości do kolejek komunikatów

//klucze 
key_t KLUCZ_PD = ftok(".", 'P'); //klucz dla pamięci dzielonej
key_t KLUCZ_SEM = ftok(".", 'S'); //klucz dla semafora

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