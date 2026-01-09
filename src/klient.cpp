//klient.cpp - proces i logika klienta

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main(int argc, char* argv[]) {
    srand(time(NULL) ^ getpid());

    int wielkosc_grupy = 1;
    if (argc > 1) {
        wielkosc_grupy = std::stoi(argv[1]);
    }

    int pam_id = polaczPamiec();
    int sem_id = polaczSemafor();
    int kol_id = polaczKolejke();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    semaforOpusc(sem_id, SEM_MAIN);
    pam->liczba_klientow++;
    semaforPodnies(sem_id, SEM_MAIN);

    //oglądacze
    if ((rand() % 100) < 5) {
        std::string log = "Klient: "+ std::to_string(wielkosc_grupy) + " osoba/y tylko ogląda i wychodzi";
        logger(log);

        semaforOpusc(sem_id, SEM_MAIN);
        pam->liczba_klientow--;
        semaforPodnies(sem_id, SEM_MAIN);
        odlaczPamiec(pam);
        return 0;
    }

    //standardowe zachowanie
    logger("Klient: wchodzę do baru " + std::to_string(wielkosc_grupy) + " osoba/y");
    wyslijKomunikat(kol_id, TYP_KLIENT_KOLEJKA, getpid(), wielkosc_grupy, 0, 0);

    int aktualny_id_stolika = -1;
    int aktualny_typ_stolika = 0;

    bool obsluzony = false;
    Komunikat odp;

    while (!obsluzony) {
        if (pam->pozar) {
            logger("Klient: Pożar! Uciekam z kolejki");
            semaforOpusc(sem_id, SEM_MAIN);
            pam->liczba_klientow--;
            semaforPodnies(sem_id, SEM_MAIN);
            odlaczPamiec(pam);
            return 0;
        }    
    
        if (odbierzKomunikat(kol_id, getpid(), &odp, false)) {
            aktualny_id_stolika = odp.id_stolika;
            aktualny_typ_stolika = odp.typ_stolika;
            std::string log = "Klient: " + std::to_string(getpid()) + " dostałem jedzenie i stolik: " + std::to_string(aktualny_id_stolika) +
                                " typ: " + std::to_string(aktualny_typ_stolika);
            logger(log);
            obsluzony = true;
        }
        else {
            usleep(50000);
        }
    }

    //jedzenie
    usleep(8000000 + (rand() % 5000000));
    if (pam->pozar) {
        logger("Klient: Pożar! Zostawiam naczynia i uciekam");
        semaforOpusc(sem_id, SEM_MAIN);
        pam->liczba_klientow--;
        semaforPodnies(sem_id, SEM_MAIN);
        odlaczPamiec(pam);
        return 0;
    }

    //zwrot naczyń
    std::string zwrot = "Klient: " + std::to_string(getpid()) + " idę zwrócić naczynia";
    logger(zwrot);
    wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), 0, 0, 200);

    Komunikat potwierdzenie;
    bool naczynia_oddane = false;
    while (!naczynia_oddane) {
        if (pam->pozar) {
            logger("Klient: Pożar! Rzucam naczynia i uciekam");
            semaforOpusc(sem_id, SEM_MAIN);
            pam->liczba_klientow--;
            semaforPodnies(sem_id, SEM_MAIN);
            odlaczPamiec(pam);
            return 0;
        }

        if (odbierzKomunikat(kol_id, getpid(), &potwierdzenie, false)) {
            naczynia_oddane = true;
        }
        else {
            usleep(50000);
        }
    }

    //oddanie stolika
    semaforOpusc(sem_id, SEM_STOLIKI);
    if (aktualny_id_stolika != -1) {
        Stolik *s = nullptr;
        if (aktualny_typ_stolika == 1) {
            s = &pam->stoliki_x1[aktualny_id_stolika];
        }
        else if (aktualny_typ_stolika == 2) {
            s = &pam->stoliki_x2[aktualny_id_stolika];
        }
        else if (aktualny_typ_stolika == 3) {
            s = &pam->stoliki_x3[aktualny_id_stolika];
        }
        else if (aktualny_typ_stolika == 4) {
            s = &pam->stoliki_x4[aktualny_id_stolika];
        }

        if (s != nullptr) {
            s->ile_zajetych_miejsc -= wielkosc_grupy;
            if (s->ile_zajetych_miejsc == 0) {
                s->wielkosc_grupy_siedzacej = 0;
            }
            std::string log = "Klient: " + std::to_string(getpid()) + " zwalniam miejsce i wychodzę";
            logger(log);
            usleep(1000000);
        }
    }

    semaforPodnies(sem_id, SEM_STOLIKI);
    semaforOpusc(sem_id, SEM_MAIN);
    pam->liczba_klientow--;
    semaforPodnies(sem_id, SEM_MAIN);
    odlaczPamiec(pam);
    return 0;
}