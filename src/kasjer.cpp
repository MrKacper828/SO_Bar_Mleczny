//kasjer.cpp - proces kasjera

#include <iostream>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    int kol_id = polaczKolejke();

    logger("Kasjer: Zaczynam pracę");

    Komunikat kom;
    while (true) {
        if (odbierzKomunikat(kol_id, 0, &kom, true)) {
            std::string log = "Kasjer: mam klienta - PID: " + std::to_string(kom.nadawca) +
                                " grupa " + std::to_string(kom.kod) + " osobowa";
            logger(log);
        }
    }
    return 0;
}