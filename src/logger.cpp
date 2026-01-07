//logger.cpp - tworzenie logów z symulacji

#include <iostream>
#include <fstream>
#include <unistd.h>
#include "logger.h"

const std::string LOGI_PLIK = "logi_bar_mleczny.txt";

//kto, co - wypis na ekran i do pliku
void logger(std::string wiadomosc) {
    pid_t pid = getpid();
    std::string kolor = "\033[0m"; //zwykły kolor
    if (wiadomosc.find("Kasjer:") == 0) {
        kolor = "\033[32m"; //zielony
    }
    else if (wiadomosc.find("Pracownik:") == 0) {
        kolor = "\033[0;33m"; //brązowy
    }

    //wypis w konsoli
    std::string log = std::to_string(pid) + "): " + wiadomosc;
    std::cout << kolor << log << "\033[0m" << std::endl;

    //zapis do pliku
    std::ofstream plik;
    plik.open(LOGI_PLIK, std::ios::app);
    if (plik.good()) {
        plik << log << std::endl;
    }
    plik.close();
}

//czyszczenie pliku logów dla nowego wywołania programu
void tabula_rasa() {
    std::ofstream plik(LOGI_PLIK);
    plik << "Otwarcie baru mlecznego\n";
    plik.close();
}