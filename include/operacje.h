//operacje.h - funkcje semaforów, pamieci dzielonej i kolejek

#ifndef OPERACJE_H
#define OPERACJE_H

//semafor
int utworzSemafor();
void usunSemafor();

//pamięć dzielona
int alokujPamiec();
void zwolnijPamiec();

//kolejki
int utworzKolejke();
void usunKolejke();

#endif