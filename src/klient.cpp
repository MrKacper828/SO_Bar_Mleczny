//klient.cpp - proces klienta

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

    //oglądacze
    if ((rand() % 100) < 5) {
        std::string log = "Klient: "+ std::to_string(wielkosc_grupy) + " osoba/y tylko ogląda i wychodzi";
        logger(log);
        odlaczPamiec(pam);
        return 0;
    }

    logger("Klient: wchodzę do baru " + std::to_string(wielkosc_grupy) + " osoba/y");
    wyslijKomunikat(kol_id, TYP_KLIENT_KOLEJKA, getpid(), wielkosc_grupy, 0, 0);

    int aktualny_id_stolika = -1;
    int aktualny_typ_stolika = 0;

    Komunikat odp;
    if (odbierzKomunikat(kol_id, getpid(), &odp, true)) {
        aktualny_id_stolika = odp.id_stolika;
        aktualny_typ_stolika = odp.typ_stolika;
        std::string log = "Klient: " + std::to_string(getpid()) + " dostałem jedzenie i stolik: " + std::to_string(aktualny_id_stolika) +
                            " typ: " + std::to_string(aktualny_typ_stolika);
        logger(log);
    }

    //jedzenie
    usleep(8000000 + (rand() % 5000000));

    //oddanie stolika
    semaforOpusc(sem_id);
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
            std::string log = "Klient: " + std::to_string(getpid()) + " oddaję naczynia zwalniam miejsce i wychodzę";
            logger(log);
            usleep(1000000);
        }
    }

    semaforPodnies(sem_id);
    odlaczPamiec(pam);
    return 0;
}