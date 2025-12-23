//pracownik.cpp - proces pracownika

#include <iostream>
#include <unistd.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    int kol_id = polaczKolejke();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Pracownik: Zaczynam pracę");

    Komunikat zamowienie;
    while (true) {
        if (odbierzKomunikat(kol_id, TYP_PRACOWNIK, &zamowienie, true)) {
            pid_t klient_pid = zamowienie.nadawca;
            int stolik_id = zamowienie.id_stolika;
            int stolik_typ = zamowienie.typ_stolika;

            std::string log = "Pracownik: Posiłek gotowy dla " + std::to_string(klient_pid) + " można kierować się do stolika";
            logger(log);

            wyslijKomunikat(kol_id, klient_pid, getpid(), 1, stolik_typ, stolik_id);
        }
    }

    odlaczPamiec(pam);
    return 0;
}