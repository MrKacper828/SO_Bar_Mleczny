//operacje.h - definicje funkcji semaforów, pamieci dzielonej i kolejek komunikatów z operacje.cpp

#ifndef OPERACJE_H
#define OPERACJE_H

#include "struktury.h"

key_t stworzKlucz(int klucz_struktury);
//semafor
int utworzSemafor();
void usunSemafor(int sem_id);
void semaforPodnies(int sem_id); //V
void semaforOpusc(int sem_id); //P
int polaczSemafor();

//pamięć dzielona
int alokujPamiec();
void zwolnijPamiec(int pam_id);
PamiecDzielona* dolaczPamiec(int pam_id);
void odlaczPamiec(PamiecDzielona* adres);
int polaczPamiec();

//kolejki
int utworzKolejke();
void usunKolejke(int kol_id);
int polaczKolejke();

//komunikaty
void wyslijKomunikat(int kol_id, long mtyp, pid_t nadawca, int dane, int typ_stolika, int id_stolika);
bool odbierzKomunikat(int kol_id, long mtyp, Komunikat* buf, bool czekaj);

#endif