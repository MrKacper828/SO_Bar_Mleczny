//klient.cpp - proces klienta

#include <iostream>
#include <unistd.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

int main() {
    int kol_id = polaczKolejke();

    logger("Klient: wchodzę do baru");
    wyslijKomunikat(kol_id, 1, getpid(), 0);

    return 0;
}