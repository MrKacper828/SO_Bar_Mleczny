//klient.cpp - proces i logika klienta

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/msg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <thread>
#include <mutex>
#include <numeric>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

//rozróżnienie grupy klientów w środku lokalu od tych na zewnątrz
PamiecDzielona* pam = nullptr;
int sem_id = -1;
pid_t g_pid = 0;
volatile bool g_w_srodku = false;
volatile sig_atomic_t g_zarejestrowany = 0;
volatile sig_atomic_t g_ewakuacja = 0;
static int g_fd_logow = -1;

//ogarnięcie logów klientów w trakcie ewakuacji żeby nie wystąpił deadlock
static void zapewnij_fd_logow() {
    if (g_fd_logow >= 0) return;
    g_fd_logow = open("logi_bar_mleczny.txt", O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
}
static void wypisz_pozar_i_exit(bool loguj) {
    if (loguj) {
        const char msg[] = "Klient: POŻAR! Uciekam z lokalu\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        if (g_fd_logow >= 0) {
            (void)write(g_fd_logow, msg, sizeof(msg) - 1);
        }
    }
    _exit(0);
}

void handler_ewakuacja(int signum) {
    if (signum == SIGTERM) {
        if (!g_ewakuacja) {
            g_ewakuacja = 1;
            wypisz_pozar_i_exit(g_w_srodku);
        }
        _exit(0);
    }
}

//funkcja do przymusowej ewakuacji i nie wyrzucania starych logów  w jej trakcie, tylko ucieczka
static inline void ewakuuj_jesli_pozar() {
    if (pam != nullptr && pam->pozar) {
        wypisz_pozar_i_exit(g_w_srodku);
    }
}

bool szansaNaStolik(PamiecDzielona* pam, int wielkosc_grupy) {
    if (wielkosc_grupy == 1) {
        for (int i=0; i<STOLIKI_X1; i++) {
            if (!pam->stoliki_x1[i].zarezerwowany) return true;
        }
        for (int i=0; i<STOLIKI_X2; i++) {
            if (!pam->stoliki_x2[i].zarezerwowany) return true;
        }
        for (int i=0; i<pam->aktualna_liczba_X3; i++) {
            if (!pam->stoliki_x3[i].zarezerwowany) return true;
        }
        for (int i=0; i<STOLIKI_X4; i++) {
            if (!pam->stoliki_x4[i].zarezerwowany) return true;
        }
    }
    else if (wielkosc_grupy == 2) {
        for (int i=0; i<STOLIKI_X2; i++) {
            if (!pam->stoliki_x2[i].zarezerwowany) return true;
        }
        for(int i=0; i<STOLIKI_X4; i++) {
            if (!pam->stoliki_x4[i].zarezerwowany) return true;
        }
    }
    else if (wielkosc_grupy == 3) {
        for (int i=0; i<pam->aktualna_liczba_X3; i++) {
            if (!pam->stoliki_x3[i].zarezerwowany) return true;
        }
    }
    else if (wielkosc_grupy == 4) {
        for (int i=0; i<STOLIKI_X4; i++) {
            if (!pam->stoliki_x4[i].zarezerwowany) return true;
        }
    }
    return false;
}

void watekOsoby(int id_osoby, int* koszt_dania) {
    int wybrane_id = (rand() % 10) + 1; //każda osoba z grupy wybiera sobie inne danie z menu
    *koszt_dania = MENU[wybrane_id];
}

int main(int argc, char* argv[]) {
    g_pid = getpid();
    zapewnij_fd_logow();
    signal(SIGTERM, handler_ewakuacja);
    int pam_id = polaczPamiec();
    sem_id = polaczSemafor();
    int kol_id = polaczKolejke();
    pam = dolaczPamiec(pam_id);
    srand(time(NULL) ^ getpid());

    ewakuuj_jesli_pozar();

    int wielkosc_grupy = 1;
    if (argc > 1) {
        wielkosc_grupy = std::stoi(argv[1]);
    }

    semaforOpusc(sem_id, SEM_LIMIT); //blokada przed zalaniem baru przez klientów

    ewakuuj_jesli_pozar();

    semaforOpusc(sem_id, SEM_MAIN);
    if (pam->pozar) {
        semaforPodnies(sem_id, SEM_MAIN);
        odlaczPamiec(pam);
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }
    pam->liczba_klientow++;
    g_zarejestrowany = 1;
    semaforPodnies(sem_id, SEM_MAIN);

    ewakuuj_jesli_pozar();

    //oglądacze
    if ((rand() % 100) < 5) {
        ewakuuj_jesli_pozar();
        logger("Klient: "+ std::to_string(wielkosc_grupy) + " osoba/y tylko ogląda i wychodzi");

        semaforOpusc(sem_id, SEM_MAIN);
        pam->liczba_klientow--;
        semaforPodnies(sem_id, SEM_MAIN);
        g_zarejestrowany = 0;
        odlaczPamiec(pam);
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }

    //dodatkowe sprawdzanie czy kolejka komunikatów jest przepełniona, jeżeli tak to klient nie wchodzi
    struct msqid_ds stan_kolejki;
    if (msgctl(kol_id, IPC_STAT, &stan_kolejki) == 0) {
        bool stan = (stan_kolejki.msg_qnum >= LIMIT_W_BARZE);
        if (stan) {
            semaforOpusc(sem_id, SEM_MAIN);
            pam->liczba_klientow--;
            semaforPodnies(sem_id, SEM_MAIN);
            g_zarejestrowany = 0;
            odlaczPamiec(pam);
            semaforPodnies(sem_id, SEM_LIMIT);
            return 0;
        }
    }

    //klient szuka czy będzie dla niego stolik który nie jest zarezerwowany
    semaforOpusc(sem_id, SEM_STOLIKI);
    bool szansa = szansaNaStolik(pam, wielkosc_grupy);
    semaforPodnies(sem_id, SEM_STOLIKI);

    if (!szansa) {
        ewakuuj_jesli_pozar();
        logger("Klient: Brak stolików (wszystko dla mnie zarezerwowane), wychodzę");

        semaforOpusc(sem_id, SEM_MAIN);
        pam->liczba_klientow--;
        semaforPodnies(sem_id, SEM_MAIN);
        g_zarejestrowany = 0;
        odlaczPamiec(pam);
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }

    g_w_srodku = true; //od tej chwili traktujemy klienta jako "w lokalu" dla logów ewakuacji
    semaforPodnies(sem_id, SEM_W_BARZE);

    ewakuuj_jesli_pozar();

    //standardowe zachowanie
    std::vector<std::thread> watki;
    std::vector<int> koszty(wielkosc_grupy);
    for (int i = 0; i < wielkosc_grupy; ++i) {
        watki.emplace_back(watekOsoby, i, &koszty[i]);
    }
    for (auto& t : watki) t.join();

    int suma_zamowienia = std::accumulate(koszty.begin(), koszty.end(), 0);

    ewakuuj_jesli_pozar();
    logger("Klient: wchodzę do baru " + std::to_string(wielkosc_grupy) + " osoba/y zamawiamy każdy sobie danie i musimy zapłacić " + std::to_string(suma_zamowienia));
    //komunikat do kasjera o zamówieniu grupy
    ewakuuj_jesli_pozar();
    wyslijKomunikat(kol_id, TYP_KLIENT_KOLEJKA, getpid(), wielkosc_grupy, 0, 0, suma_zamowienia);

    //odpowiedź od pracownika, dostanie jedzenia i kierowanie się do przydzielonego stolika
    Komunikat odp;
    ewakuuj_jesli_pozar();
    if (odbierzKomunikat(kol_id, getpid(), &odp, true)) {
        int id_stolika = odp.id_stolika;
        int typ_stolika = odp.typ_stolika;

        ewakuuj_jesli_pozar();
        logger("Klient: " + std::to_string(getpid()) + " dostałem jedzenie i stolik: " + std::to_string(id_stolika) +
                            " typ: " + std::to_string(typ_stolika) + ", pora jeść");
    

        //symulowanie czasu jedzenia
        usleep(8000000 + (rand() % 5000000));

        //zwrot naczyń
        ewakuuj_jesli_pozar();
        logger("Klient: " + std::to_string(getpid()) + " idę zwrócić naczynia");
        ewakuuj_jesli_pozar();
        wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), 0, 0, 200, 0);

        //potwierdzenie odebrania naczyń
        ewakuuj_jesli_pozar();
        odbierzKomunikat(kol_id, getpid(), &odp, true);

        //oddanie stolika
        semaforOpusc(sem_id, SEM_STOLIKI);
        Stolik *s = nullptr;
        if (typ_stolika == 1) {
            s = &pam->stoliki_x1[id_stolika];
        }
        else if (typ_stolika == 2) {
            s = &pam->stoliki_x2[id_stolika];
        }
        else if (typ_stolika == 3) {
            s = &pam->stoliki_x3[id_stolika];
        }
        else if (typ_stolika == 4) {
            s = &pam->stoliki_x4[id_stolika];
        }

        if (s != nullptr) {
            s->ile_zajetych_miejsc -= wielkosc_grupy;
            if (s->ile_zajetych_miejsc == 0) {
                s->ile_zajetych_miejsc = 0;
                s->wielkosc_grupy_siedzacej = 0;
            }
            semaforPodnies(sem_id, SEM_STOLIKI);

            ewakuuj_jesli_pozar();
            logger("Klient: " + std::to_string(getpid()) + " zwalniam miejsce i wychodzę");
        }
        else {
            semaforPodnies(sem_id, SEM_STOLIKI);
        }
    }
    if (g_w_srodku) {
        semaforOpusc(sem_id, SEM_W_BARZE);
        g_w_srodku = false;
    }

    semaforOpusc(sem_id, SEM_MAIN);
    pam->liczba_klientow--;
    semaforPodnies(sem_id, SEM_MAIN);
    g_zarejestrowany = 0;

    odlaczPamiec(pam);
    semaforPodnies(sem_id, SEM_LIMIT);
    return 0;
}