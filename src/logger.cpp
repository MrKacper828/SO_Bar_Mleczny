//logger.cpp - tworzenie logów z symulacji

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "logger.h"

const std::string PLIK_LOGOW = "logi_bar_mleczny.txt";

//zoptymalizaowanie logów
//zapewnienie poprawnego zapisu całych logów
static int deskryptor_pliku_logow = -1;
static bool zapisz_wszystko(int deskryptor, const char* dane, size_t rozmiar) {
    while (rozmiar > 0) {
        ssize_t zapisano = write(deskryptor, dane, rozmiar);
        if (zapisano < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }
        dane += static_cast<size_t>(zapisano);
        rozmiar -= static_cast<size_t>(zapisano);
    }
    return true;
}
static int zapewnij_deskryptor_pliku_logow() {
    if (deskryptor_pliku_logow >= 0) {
        return deskryptor_pliku_logow;
    }
    deskryptor_pliku_logow = open(PLIK_LOGOW.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    return deskryptor_pliku_logow;
}

//kto, co - wypis na ekran i do pliku
void logger(const std::string& wiadomosc) {
    pid_t pid = getpid();
    std::string kolor = "\033[0m"; //zwykły kolor
    if (wiadomosc.find("Kasjer:") == 0) {
        kolor = "\033[32m"; //zielony
    }
    else if (wiadomosc.find("Pracownik:") == 0) {
        kolor = "\033[0;33m"; //brązowy
    }
    else if (wiadomosc.find("Kierownik:") == 0) {
        kolor = "\033[31m"; //czerwony
    } 

    std::string log = std::to_string(pid) + "): " + wiadomosc;

    //zabezpieczony zapis w konsoli
    std::string console_line = kolor + log + "\033[0m\n";
    (void)zapisz_wszystko(STDOUT_FILENO, console_line.c_str(), console_line.size());

    //zabezpieczony zapis do pliku
    int deskryptor = zapewnij_deskryptor_pliku_logow();
    if (deskryptor >= 0) {
        std::string file_line = log + "\n";
        (void)zapisz_wszystko(deskryptor, file_line.c_str(), file_line.size());
    }
}

//czyszczenie pliku logów dla nowego wywołania programu
void tabula_rasa() {
    int deskryptor = open(PLIK_LOGOW.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (deskryptor < 0) {
        return;
    }
    const char* naglowek = "Otwarcie baru mlecznego\n";
    (void)zapisz_wszystko(deskryptor, naglowek, std::strlen(naglowek));
    close(deskryptor);
}