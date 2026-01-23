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

const int SEM_MAIN = 0; //semafor od reszty zasobów pamięci dzielonej
const int SEM_STOLIKI = 1; //semafor od stolików
const int SEM_LIMIT = 2; //semafor do tworzenia klientów
//semafory do synchronizacji ewakuacji
const int SEM_W_BARZE = 3; //liczba klientów w lokalu
const int SEM_EWAK_KASJER_DONE = 4; //kasjer zamknął kasę i uciekł
const int SEM_EWAK_PRACOWNIK_DONE = 5; //pracownik uciekł po kasjerze

const int ILOSC_KLIENTOW = 10000; //ilość tworzonych procesów klientów
const int LIMIT_W_BARZE = 100; //limit klientów w barze (dokładniej w kolejce komunikatów do kasjera)

//max ilość stolików w restauracji
const int STOLIKI_X1 = 8;
const int STOLIKI_X2 = 10;
//inicjalizować parzystą 2x większą liczbę żeby dało się użyć komendy podwojenia stolików X3
const int STOLIKI_X3 = 18;
const int STOLIKI_X4 = 16;

struct Stolik { 
    int id;
    int pojemnosc_max;
    int ile_zajetych_miejsc;
    int wielkosc_grupy_siedzacej;
    bool zarezerwowany;
};

//menu baru jako 10 dań, a tutaj widoczna ich cena
const int MENU[11] = {0, 5, 6, 8, 10, 12, 29, 30, 33, 35, 40};
struct PamiecDzielona {
    Stolik stoliki_x1[STOLIKI_X1];
    Stolik stoliki_x2[STOLIKI_X2];
    Stolik stoliki_x3[STOLIKI_X3];
    Stolik stoliki_x4[STOLIKI_X4];

    pid_t pgid_grupy;
    pid_t pracownik_pid;

    bool pozar;
    bool podwojenie_X3;
    bool blokada_rezerwacyjna;
    int aktualna_liczba_X3;
    int liczba_klientow;
    long utarg;
};

//dla msg
const long TYP_KLIENT_KOLEJKA = 1;
const long TYP_PRACOWNIK = 2;
struct Komunikat {
    long mtype;
    pid_t nadawca;
    int dane;       //wielkość grupy lub ilość rezerwacji
    int id_stolika;
    int typ_stolika;
    int id_dania;   //suma cen lub kod polecenia
};
const int ROZMIAR_KOM = sizeof(Komunikat) - sizeof(long);

#endif
