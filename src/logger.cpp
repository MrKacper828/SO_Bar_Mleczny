//logger.cpp - tworzenie logów z symulacji

#include <iostream>
#include <fstream>
#include <unistd.h>
#include "logger.h"

const std::string LOGI_PLIK = "logi_bar_mleczny.txt";

//kto, co - wypis na ekran i do pliku
void logger(std::string wiadomosc) {
    pid_t pid = getpid();
    std::string log = std::to_string(pid) + "): " + wiadomosc;
    std::cout << log << std::endl;

    std::ofstream plik;
    plik.open(LOGI_PLIK, std::ios::app);
    if (plik.good()) {
        plik << log << std::endl;
    }
    plik.close();
}

void tabula_rasa() {
    std::ofstream plik(LOGI_PLIK);
    plik << "Otwarcie baru mlecznego\n";
    plik.close();
}