//klient.cpp - proces i logika klienta

#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/msg.h>
#include <signal.h>
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

void handler_ewakuacja(int signum) {
    if (signum == SIGTERM) {
        if (g_w_srodku) {
            std::string ucieczka = "Klient " + std::to_string(g_pid) + ": POŻAR! Uciekam z lokalu";
            logger(ucieczka);

            if (pam != nullptr && sem_id != -1) {
                semaforOpusc(sem_id, SEM_MAIN);
                pam->liczba_klientow--;
                semaforPodnies(sem_id, SEM_MAIN);
            }
        }
        exit(0);
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
    signal(SIGTERM, handler_ewakuacja);
    g_pid = getpid();
    int pam_id = polaczPamiec();
    sem_id = polaczSemafor();
    int kol_id = polaczKolejke();
    pam = dolaczPamiec(pam_id);
    srand(time(NULL) ^ getpid());

    int wielkosc_grupy = 1;
    if (argc > 1) {
        wielkosc_grupy = std::stoi(argv[1]);
    }

    semaforOpusc(sem_id, SEM_LIMIT); //blokada przed zalaniem baru przez klientów

    semaforOpusc(sem_id, SEM_MAIN);
    if (pam->pozar) {
        semaforPodnies(sem_id, SEM_MAIN);
        odlaczPamiec(pam);
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }
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
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }

    //dodatkowe sprawdzanie czy kolejka komunikatów jest przepełniona, jeżeli tak to klient rezygnuje
    struct msqid_ds stan_kolejki;
    if (msgctl(kol_id, IPC_STAT, &stan_kolejki) == 0) {
        bool stan = (stan_kolejki.msg_qnum >= LIMIT_W_BARZE);
        if (stan) {
            //logger("Klient: Duża kolejka, odpuszczam");
            semaforOpusc(sem_id, SEM_MAIN);
            pam->liczba_klientow--;
            semaforPodnies(sem_id, SEM_MAIN);
            odlaczPamiec(pam);
            semaforPodnies(sem_id, SEM_LIMIT);
            return 0;
        }
    }

    //sprawdzanie czy klient ma szansę na stolik
    //szuka czy będzie dla niego stolik który nie jest zarezerwowany
    semaforOpusc(sem_id, SEM_STOLIKI);
    bool szansa = szansaNaStolik(pam, wielkosc_grupy);
    semaforPodnies(sem_id, SEM_STOLIKI);

    if (!szansa) {
        std::string brak = "Klient: Brak stolików (wszystko dla mnie zarezerwowane), wychodzę";
        logger(brak);

        semaforOpusc(sem_id, SEM_MAIN);
        pam->liczba_klientow--;
        semaforPodnies(sem_id, SEM_MAIN);
        odlaczPamiec(pam);
        semaforPodnies(sem_id, SEM_LIMIT);
        return 0;
    }

    g_w_srodku = true; //klien jest już zaliczany jako ten w barze

    //standardowe zachowanie
    std::vector<std::thread> watki;
    std::vector<int> koszty(wielkosc_grupy);
    for (int i = 0; i < wielkosc_grupy; ++i) {
        watki.emplace_back(watekOsoby, i, &koszty[i]);
    }
    for (auto& t : watki) t.join();

    int suma_zamowienia = std::accumulate(koszty.begin(), koszty.end(), 0);
    logger("Klient: wchodzę do baru " + std::to_string(wielkosc_grupy) + " osoba/y zamawiamy każdy sobie danie i musimy zapłacić " + std::to_string(suma_zamowienia));
    //komunikat do kasjera o zamówieniu grupy
    wyslijKomunikat(kol_id, TYP_KLIENT_KOLEJKA, getpid(), wielkosc_grupy, 0, 0, suma_zamowienia);

    //odpowiedź od pracownika, dostanie jedzenia i kierowanie się do przydzielonego stolika
    Komunikat odp;
    if (odbierzKomunikat(kol_id, getpid(), &odp, true)) {
        int id_stolika = odp.id_stolika;
        int typ_stolika = odp.typ_stolika;
        std::string log = "Klient: " + std::to_string(getpid()) + " dostałem jedzenie i stolik: " + std::to_string(id_stolika) +
                            " typ: " + std::to_string(typ_stolika) + ", pora jeść";
        logger(log);
    

        //symulowanie czasu jedzenia
        usleep(8000000 + (rand() % 5000000));

        //zwrot naczyń
        std::string zwrot = "Klient: " + std::to_string(getpid()) + " idę zwrócić naczynia";
        logger(zwrot);
        wyslijKomunikat(kol_id, TYP_PRACOWNIK, getpid(), 0, 0, 200, 0);

        //potwierdzenie odebrania naczyń
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

            std::string log = "Klient: " + std::to_string(getpid()) + " zwalniam miejsce i wychodzę";
            logger(log);
        }
        else {
            semaforPodnies(sem_id, SEM_STOLIKI);
        }
    }
    g_w_srodku = false;

    semaforOpusc(sem_id, SEM_MAIN);
    pam->liczba_klientow--;
    semaforPodnies(sem_id, SEM_MAIN);

    odlaczPamiec(pam);
    semaforPodnies(sem_id, SEM_LIMIT);
    return 0;
}