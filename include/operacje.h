//operacje.h - funkcje semaforów, pamieci dzielonej i kolejek

#ifndef OPERACJE_H
#define OPERACJE_H

#include "struktury.h"

key_t stworzKlucz(int klucz_struktury);
//semafor
int utworzSemafor();
void usunSemafor(int sem_id);
void semaforPodnies(int sem_id); //V
void semaforOpusc(int sem_id); //P

//pamięć dzielona
int alokujPamiec();
void zwolnijPamiec(int shm_id);

//kolejki
int utworzKolejke();
void usunKolejke();

#endif