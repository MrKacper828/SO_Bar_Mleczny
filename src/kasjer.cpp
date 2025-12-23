//kasjer.cpp - proces kasjera

#include <iostream>
#include <vector>
#include <unistd.h>
#include "operacje.h"
#include "struktury.h"
#include "logger.h"

struct Klient {
    pid_t pid;
    int wielkosc_grupy;
};

int main() {
    int kol_id = polaczKolejke();
    int sem_id = polaczSemafor();
    int pam_id = polaczPamiec();
    PamiecDzielona *pam = dolaczPamiec(pam_id);

    logger("Kasjer: Zaczynam pracę");

    std::vector<Klient> kolejka;
    Komunikat kom;
    while (true) {
        while (odbierzKomunikat(kol_id, TYP_KLIENT_KOLEJKA, &kom, false)) {
            Klient nowy;
            nowy.pid = kom.nadawca;
            nowy.wielkosc_grupy = kom.dane;
            kolejka.push_back(nowy);
            std::string log = "Kasjer: mam klienta: " + std::to_string(nowy.pid) +
                                " grupa " + std::to_string(nowy.wielkosc_grupy) + " osobowa";
            logger(log);
        }
    

        if (!kolejka.empty()) {
            semaforOpusc(sem_id);
            for (size_t i = 0; i < kolejka.size(); i++) {
                Klient k = kolejka[i];
                int przypisane_id = -1;
                int przypisany_typ = 0;

                Stolik *tablica = nullptr;
                
                //grupa 1 osobowa
                if (k.wielkosc_grupy == 1) {
                    //stoliki X1 dla 1 osoby
                    tablica = pam->stoliki_x1;
                    for (int j = 0; j < STOLIKI_X1; j++) {
                        if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 1) 
                            && (tablica[j].ile_zajetych_miejsc + 1 <= tablica[j].pojemnosc_max)) {
                            przypisane_id = j;
                            przypisany_typ = 1;
                            break;
                        }
                    }
                    //stoliki X2 dla 1 osoby
                    if (przypisane_id == -1) {
                        tablica = pam->stoliki_x2;
                        for (int j = 0; j < STOLIKI_X2; j++) {
                            if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 1) 
                                && (tablica[j].ile_zajetych_miejsc + 1 <= tablica[j].pojemnosc_max)) {
                                przypisane_id = j;
                                przypisany_typ = 2;
                                break;
                            }
                        }
                    }
                    //stoliki X3 dla 1 osoby
                    if (przypisane_id == -1) {
                        tablica = pam->stoliki_x3;
                        for (int j = 0; j < STOLIKI_X3; j++) {
                            if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 1) 
                                && (tablica[j].ile_zajetych_miejsc + 1 <= tablica[j].pojemnosc_max)) {
                                przypisane_id = j;
                                przypisany_typ = 3;
                                break;
                            }
                        }
                    }
                    //stoliki X4 dla 1 osoby
                    if (przypisane_id == -1) {
                        tablica = pam->stoliki_x4;
                        for (int j = 0; j < STOLIKI_X4; j++) {
                            if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 1) 
                                && (tablica[j].ile_zajetych_miejsc + 1 <= tablica[j].pojemnosc_max)) {
                                przypisane_id = j;
                                przypisany_typ = 4;
                                break;
                            }
                        }
                    }
                }
                //grupa 2 osobowa
                else if (k.wielkosc_grupy == 2) {
                    //stoliki X2 dla 2 osób
                    tablica = pam->stoliki_x2;
                    for (int j = 0; j < STOLIKI_X2; j++) {
                        if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 2) 
                            && (tablica[j].ile_zajetych_miejsc + 2 <= tablica[j].pojemnosc_max)) {
                            przypisane_id = j;
                            przypisany_typ = 2;
                            break;
                        }
                    }
                    //stoliki X4 dla 2 osób
                    tablica = pam->stoliki_x4;
                    for (int j = 0; j < STOLIKI_X4; j++) {
                        if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 2) 
                            && (tablica[j].ile_zajetych_miejsc + 2 <= tablica[j].pojemnosc_max)) {
                            przypisane_id = j;
                            przypisany_typ = 4;
                            break;
                        }
                    }
                }
                //grupa 3 osobowa
                else if (k.wielkosc_grupy == 3) {
                    //stoliki X3 dla 3 osób
                    tablica = pam->stoliki_x3;
                    for (int j = 0; j < STOLIKI_X3; j++) {
                        if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 3) 
                            && (tablica[j].ile_zajetych_miejsc + 3 <= tablica[j].pojemnosc_max)) {
                            przypisane_id = j;
                            przypisany_typ = 3;
                            break;
                        }
                    }
                }
                //grupa 4 osobowa
                else if (k.wielkosc_grupy == 4) {
                    //stoliki X4 dla 4 osób
                    tablica = pam->stoliki_x4;
                    for (int j = 0; j < STOLIKI_X4; j++) {
                        if ((tablica[j].ile_zajetych_miejsc == 0 || tablica[j].wielkosc_grupy_siedzacej == 4) 
                            && (tablica[j].ile_zajetych_miejsc + 4 <= tablica[j].pojemnosc_max)) {
                            przypisane_id = j;
                            przypisany_typ = 4;
                            break;
                        }
                    }
                }

                //po znalezieniu miejsca
                if (przypisane_id != -1) {
                    Stolik *s = nullptr;
                    if (przypisany_typ == 1) {
                        s = &pam->stoliki_x1[przypisane_id];
                    }
                    else if (przypisany_typ == 2) {
                        s = &pam->stoliki_x2[przypisane_id];
                    }
                    else if (przypisany_typ == 3) {
                        s = &pam->stoliki_x3[przypisane_id];
                    }
                    else if (przypisany_typ == 4) {
                        s = &pam->stoliki_x4[przypisane_id];
                    }

                    if (s != nullptr) {
                        s->ile_zajetych_miejsc += k.wielkosc_grupy;
                        if (s->ile_zajetych_miejsc == k.wielkosc_grupy) {
                            s->wielkosc_grupy_siedzacej = k.wielkosc_grupy;
                        }
                    
                        std::string log = "Kasjer: Przypisano stolik " + std::to_string(przypisane_id) + " typu: " + std::to_string(przypisany_typ) + " dla: " + std::to_string(k.pid);
                        logger(log);

                        wyslijKomunikat(kol_id, TYP_PRACOWNIK, k.pid, k.wielkosc_grupy, przypisany_typ, przypisane_id);

                        kolejka.erase(kolejka.begin() + i);
                        break;
                    }
                }
            }
            semaforPodnies(sem_id);
       }
       usleep(1000000);
    }
    odlaczPamiec(pam);
    return 0;
}