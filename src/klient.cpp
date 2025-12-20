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
    int kol_id = polaczKolejke();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    //oglądacze
    if ((rand() % 100) < 5) {
        std::string log = "Klient: "+ std::to_string(wielkosc_grupy) + " osób tylko ogląda i wychodzi";
        logger(log);
        odlaczPamiec(pam);
        return 0;
    }

    logger("Klient: wchodzę do baru " + std::to_string(wielkosc_grupy) + "osób");
    wyslijKomunikat(kol_id, 1, getpid(), wielkosc_grupy);

    odlaczPamiec(pam);
    return 0;
}