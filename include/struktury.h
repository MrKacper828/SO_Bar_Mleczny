//struktury.h - klucze i struktury
//                  w pamięci dzielonej + komunikaty

#ifndef STRUKTURY_H
#define STRUKTURY_H

#include <sys/types.h>

//klucze
#define KLUCZ_SCIEZKA "."
const int KLUCZ_PD = 'P'; //klucz dla pamięci dzielonej
const int KLUCZ_SEM = 'S'; //klucz dla semafora
const int KLUCZ_KOL = 'K'; //klucz dla kolejek

//max ilość stolików w restauracji
const int STOLIKI_X1 = 2;
const int STOLIKI_X2 = 2;
//inicjalizować parzystą 2x większą liczbę żeby dało się użyć komendy podwojenia stolików X3
const int STOLIKI_X3 = 6;
const int STOLIKI_X4 = 4;

struct Stolik { 
    int id;
    int pojemnosc_max;
    int ile_zajetych_miejsc;
    int wielkosc_grupy_siedzacej;
    bool zarezerwowany;
};


struct PamiecDzielona {
    Stolik stoliki_x1[STOLIKI_X1];
    Stolik stoliki_x2[STOLIKI_X2];
    Stolik stoliki_x3[STOLIKI_X3];
    Stolik stoliki_x4[STOLIKI_X4];

    bool pozar;
    bool podwojenie_X3;
    bool blokada_rezerwacyjna;
    int aktualna_liczba_X3;
    int liczba_klientow;
};

//dla msg
const long TYP_KLIENT_KOLEJKA = 1;
const long TYP_PRACOWNIK = 2;
struct Komunikat {
    long mtype;
    pid_t nadawca;
    int dane;
    int id_stolika;
    int typ_stolika;
};
const int ROZMIAR_KOM = sizeof(Komunikat) - sizeof(long);

#endif