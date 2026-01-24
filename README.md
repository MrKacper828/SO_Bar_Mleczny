## Projekt Systemy Operacyjne 2025/2026 - Temat 14: Bar mleczny
**Autor:** Kacper Korpacki

**Numer albumu:** 155192

**Repozytorium GitHub:** https://github.com/MrKacper828/SO_Bar_Mleczny

**Środowisko**: Ubuntu 24.04.3 LTS (VirtualBox)

**Kompilator**: gcc 13.3.0

---
### 1. Opis zadania i uruchamianie go:

 **1. Opis tematu:**
Projekt jest symulacją działania baru mlecznego w systemie Linux, wykorzystującą mechanizmy IPC (pamięć dzielona, semafory, kolejki komunikatów). Przechodzi ona przez cykl życia klienta, który wchodzi do baru, czeka w kolejce, dostaje stolik i jedzenie, je i opuszcza lokal. Uwzględnia również zachowanie pracowników lokalu jak i specjalne akcje zlecane przez kierownika.

**2. Uruchomienie symulacji:**
```
make
./main
```
Jeżeli chcę się również kontrolować symulację poprzez menu kierownika należy w drugiej osobnej konsoli wpisać:
```
./kierownik
```
Zakończenie symulacji następuje po naciśnięciu `ctrl+c`(nagłe zakończenie) lub po wysłaniu komunikatu pożar (opcja 3 w menu) przez Kierownika (zakończenie z ewakuacją).

---
### 2. Struktura kodu:
Wszystkie elementy symulacji (Kierownik, Pracownik, Kasjer, Klienci) są procesami tworzonymi przez `fork()`. Komunikacja między nimi dzieje się przez pamięć dzieloną i wiadomości w kolejce komunikatów. Dostępu do sekcji krytycznej i synchronizacji strzegą semafory.

-`main.cpp`: Główny proces macierzysty. Inicjalizuje struktury IPC, uruchamia procesy pracowników baru, generuje procesy klientów i tworzy wątek sprzątający zombie.

-`kasjer.cpp`: Proces kasjera przydziela dla klientów miejsca tak aby było to jak najbardziej optymalne dla dochodu. Weryfikuje dostępność stolików i zleca 
przygotowanie wybranego posiłku przez klientów.

-`pracownik.cpp`: Proces pracownika obsługuje wydawanie posiłków i odbiór naczyń od klientów. Dodatkowo zajmuje się specjalnymi sygnałami od kierownika (zwiększenie liczby stolików X3, rezerwacja).

-`kierownik.cpp`: Proces kierownika jest niezależnym procesem posiadającym interfejs sterujący symulacją w którym może wysyłać specjalne polecenia do pracownika, ogłaszać ewakuację lub sprawdzać stan dostępności stolików(rezerwacje).

-`klient.cpp`: Proces klienta symuluje zachowanie klienta(może to być grupa) baru: wejście, kolejka, oczekiwanie, jedzenie, zwrot naczyń i wyjście.

-`struktury.h`: Tutaj są zdefiniowane wszystkie zmienne i struktury wykorzystywane w symulacji. Można tu zmienić liczbę klientów, czy ilość miejsc w barze.

-`operacje.cpp` i `operacje.h`: W tych plikach znajdują się wrappery na funkcję zarządzania mechanizmami IPC.

-`logger.cpp` i `logger.h`: Służy do tworzenia i zapisywania logów z symulacji.

---
### 3. Pseudokod wybranych algorytmów:
**1.** Algorytm przydziału stolików (Kasjer):
```
//funkcja pomocnicza do wybierania miejsca w barze dla nowych klientów
FUNKCJA ZnajdzWolneMiejsce(ListaStolikow, WielkoscGrupy):
	DLA KAŻDEGO Stolika S w ListaStolikow:
		JEŻELI S jest Zarezerwowany:
			Pomiń ten stolik
		JEŻELI ((S nie jest pusty) ORAZ (S.wielkosc_grupy != WielkoscGrupy)):
			Pomiń ten stolik
		JEŻELI (S.zajete_miejsca + WielkoscGrupy <= S.pojemnosc_max):
			Zaktualizuj S.zajete_miejsca
			JEŻELI S byl pusty:
				Przypisz typ grupy do stolika
			ZWRÓĆ ID Stolika
	ZWRÓĆ Błąd(Brak Miejsca)

GŁÓWNA PĘTLA KASJERA:
	DOPÓKI KolejkaKlientow nie jest pusta:
		Pobierz Klienta K z kolejki
		[ZABEZPIECZENIE SEKCJI KRYTYCZNEJ SEMAFOREM STOLIKI]
			ID_Stolika = -1
			//Szukanie optymalnego dla zarobku miejsca dla grup klientów
			JEŻELI K.wielkosc == 1:
				ZnajdzWolneMiejsce kolejno dla stolików X1, X2, X3, X4
			JEŻELI K.wielkosc == 2:
				ZnajdzWolneMiejsce kolejno dla stolików X2, X4
			JEŻELI K.wielkosc == 3:
				ZnajdzWolneMiejsce kolejno dla stolików X3
			JEŻELI K.wielkosc == 4:
				ZnajdzWolneMiejsce kolejno dla stolików X4
			ID_Stolika = Wynik wyszukiwania
		[KONIEC SEKCJI KRYTYCZNEJ]
		JEŻELI ID_Stolika != -1:
			Wyślij komunikat do Pracownika o Kliencie
			Usuń Klienta K z listy
			Przerwij pętlę
```
**2.** Algorytm obsługi rezerwacji (Pracownik):
```
//Ustalona z góry rezerwacja dla 2 stolików X4 na sygnał
ZadanaIlosc = 2
JEŻELI Sygnał od Kierownika == flaga_rezerwacji:
	//Sprawdzenie czy rezerwacja w ogóle możliwa
	[ZABEZPIECZENIE SEKCJI KRYTYCZNEJ SEMAFOREM STOLIKI]
		Dostepne = Policz niezarezerwowane stoliki X4
	[KONIEC SEKCJI KRYTYCZNEJ]
	Limit = MIN(ZadanaIlosc, Dostepne)
	JEŻELI Limit == 0:
		Zakończ Rezerwację
	
	//Blokada wejścia nowych klientów i pętla rezerwacyjna
	flaga BLOKADA_REZERWACYJNA = true
	Zarezerwowano = 0
	DOPÓKI Zarezerwowano < Limit:
		JEŻELI Pożar: PRZERWIJ
		[ZABEZPIECZENIE SEKCJI KRYTYCZNEJ SEMAFOREM STOLIKI]
			Szukaj Stolika S który jest Wolny i Niezarezerwowany
				JEŻELI Znaleziono S:
					Oznacz S jako Zarezerwowany
					Zarezerwowano++
		[KONIEC SEKCJI KRYTYCZNEJ]
		
		//obsługa klientów w trakcie rezerwacji
		JEŻELI Zarezerwowano < Limit:
			DOPÓKI są wiadomośći w Kolejce:
				Pobierz Wiadomość W
				JEŻELI W == Zwrot Naczyń:
					Wyślij potwierdzenie
				JEŻELI W == Zamówienie:
					Wyślij potwierdzenie
	flaga BLOKADA_REZERWACYJNA = false	
```

---
### 4. Problematyczne rzeczy w projekcie z rozwiązaniem:
a) race condition (wyścig o zasoby):
- Problem: Jednoczesny dostęp do stolików przez Kasjera i Pracownika
- Rozwiązanie: Dostęp do struktur stolików z pamięci dzielonej jest zawsze zabezpieczony semaforem SEM_STOLIKI.

b) synchronizacja przy pożarze:

- Problem: Konieczność przerwania wszystkich procesów w poprawnej kolejności.
- Rozwiązanie: Zastosowanie sygnałów w połączeniu z globalną flagą `pozar` i synchronizacja ewakuacji semaforami poprzez oczekiwanie procesów obsługi baru na zniknięcie wszystkich procesów klientów.

c) Duża ilość klientów i klientów zombie:
- Problem: Przy dużej ilości klientów wchodzących od razu do baru i dużej ilości klientów procesów zombie po wyjściu z baru symulacja powoduje bardzo znaczący spadek wydajności.
- Rozwiązanie: Zastosowanie asynchronicznego wątku czyszczącego procesy zombie klientów i nie wpuszczanie klientów od razu do baru a pobieranie ich powoli z puli śpiących procesów (semafor bramkarz na wejściu).

---
### 5. Elementy wyróżniające:
- **Kolorowy logger**: `logger.cpp` wykorzystuje kody ANSI do kolorowania logów w zależności od nadawcy.
- **Menu Kierownika**: Kierownik posiada swoje menu włączane w osobnym terminalu. Z tego menu da się wysyłać specjalne sygnały i w każdym momencie podpatrzeć aktualną liczbę zarezerwowanych i dostępnych stolików.

---
### 6. Testy:
**Ważny detal w testach**: Zmiany do testów odnoszą się zawsze do podstawowej wersji programu z takimi wartościami jakie znajdują się na githubie.

<details>
<summary> <strong>1. Test1: Przerwanie spożywania posiłku sygnałem ewakuacja</strong> </summary>

Opis testu:  Test polega na zmianie czasu jedzenia na bardzo długi, np. 30 sekund. Następnie wpuszczamy do baru 10 klientów i czekamy aż w logach pojawi się, że cała 10 zacznie spożywać posiłek. Uruchamiamy wtedy sygnał 3 od Kierownika i rozpoczynamy ewakuację. Wszyscy klienci powinni niemal natychmiast przerwać jedzenie i się ewakuować.

Zmiany w kodzie:

W `struktury.h`: `const  int  ILOSC_KLIENTOW  =  10;`

W `klient.cpp`: `//symulowanie czasu jedzenia`
							  `usleep(30000000);`
							  
W `main.cpp`:

<details>
<summary>zmiana głównej pętli tworzenia klientów na:</summary>

```cpp
    int ile_juz_stworzono = 0;
    while(!koniec) {
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            semaforPodnies(sem_id, SEM_MAIN);
            koniec = 1;
            break;
        }
        semaforPodnies(sem_id, SEM_MAIN);
        if (ile_juz_stworzono < 10) {
            int wielkosc_grupy = (rand() % 4) + 1;
            std::string wielkosc = std::to_string(wielkosc_grupy);
            pid = fork();
            if (pid == 0) {
                execl("./klient", "klient", wielkosc.c_str(), NULL);
                perror("Błąd execl klient");
                exit(1);
            }
            else if (pid > 0) {
                std::lock_guard<std::mutex> lock(mutex_procesy);
                procesy_potomne.push_back(pid);
                liczba_aktywnych_klientow++;
                ile_juz_stworzono++;
            }
        } 
        else {
            usleep(100000); 
        }
    }
```

</details>

---

<details>
<summary>Logi potwierdzające działanie:</summary>

```
Otwarcie baru mlecznego
6459): Utworzono zasoby, start symulacji pgid: 6459
6461): Pracownik: Zaczynam pracę
6460): Kasjer: Zaczynam pracę
6463): Klient: wchodzę do baru 4 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 81
6470): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 43
6469): Klient: wchodzę do baru 1 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 8
6467): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 41
6465): Klient: wchodzę do baru 1 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 8
6466): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 76
6471): Klient: wchodzę do baru 1 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 35
6464): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 48
6468): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 40
6472): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 27
6460): Kasjer: mam klienta: 6463 grupa 4 osobowa, zapłacił 81
6460): Kasjer: mam klienta: 6470 grupa 2 osobowa, zapłacił 43
6460): Kasjer: mam klienta: 6465 grupa 1 osobowa, zapłacił 8
6460): Kasjer: mam klienta: 6466 grupa 3 osobowa, zapłacił 76
6460): Kasjer: mam klienta: 6469 grupa 1 osobowa, zapłacił 8
6460): Kasjer: mam klienta: 6467 grupa 2 osobowa, zapłacił 41
6460): Kasjer: mam klienta: 6471 grupa 1 osobowa, zapłacił 35
6460): Kasjer: mam klienta: 6464 grupa 3 osobowa, zapłacił 48
6460): Kasjer: mam klienta: 6468 grupa 2 osobowa, zapłacił 40
6460): Kasjer: mam klienta: 6472 grupa 3 osobowa, zapłacił 27
6460): Kasjer: Przypisano stolik 0 typu: 4 dla: 6463
6461): Pracownik: Wydaję posiłek dla 6463 (stolik 0)
6463): Klient: 6463 dostałem jedzenie i stolik: 0 typ: 4, pora jeść
6460): Kasjer: Przypisano stolik 0 typu: 2 dla: 6470
6461): Pracownik: Wydaję posiłek dla 6470 (stolik 0)
6470): Klient: 6470 dostałem jedzenie i stolik: 0 typ: 2, pora jeść
6460): Kasjer: Przypisano stolik 0 typu: 1 dla: 6465
6461): Pracownik: Wydaję posiłek dla 6465 (stolik 0)
6465): Klient: 6465 dostałem jedzenie i stolik: 0 typ: 1, pora jeść
6460): Kasjer: Przypisano stolik 0 typu: 3 dla: 6466
6461): Pracownik: Wydaję posiłek dla 6466 (stolik 0)
6466): Klient: 6466 dostałem jedzenie i stolik: 0 typ: 3, pora jeść
6460): Kasjer: Przypisano stolik 1 typu: 1 dla: 6469
6461): Pracownik: Wydaję posiłek dla 6469 (stolik 1)
6469): Klient: 6469 dostałem jedzenie i stolik: 1 typ: 1, pora jeść
6460): Kasjer: Przypisano stolik 1 typu: 2 dla: 6467
6461): Pracownik: Wydaję posiłek dla 6467 (stolik 1)
6467): Klient: 6467 dostałem jedzenie i stolik: 1 typ: 2, pora jeść
6460): Kasjer: Przypisano stolik 2 typu: 1 dla: 6471
6461): Pracownik: Wydaję posiłek dla 6471 (stolik 2)
6471): Klient: 6471 dostałem jedzenie i stolik: 2 typ: 1, pora jeść
6460): Kasjer: Przypisano stolik 1 typu: 3 dla: 6464
6461): Pracownik: Wydaję posiłek dla 6464 (stolik 1)
6464): Klient: 6464 dostałem jedzenie i stolik: 1 typ: 3, pora jeść
6460): Kasjer: Przypisano stolik 2 typu: 2 dla: 6468
6461): Pracownik: Wydaję posiłek dla 6468 (stolik 2)
6468): Klient: 6468 dostałem jedzenie i stolik: 2 typ: 2, pora jeść
6460): Kasjer: Przypisano stolik 2 typu: 3 dla: 6472
6461): Pracownik: Wydaję posiłek dla 6472 (stolik 2)
6472): Klient: 6472 dostałem jedzenie i stolik: 2 typ: 3, pora jeść
6495): Kierownik: Ogłaszam pożar i ewakuację!
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
6460): Kasjer: Wszyscy klienci ewakuowani. Zamykam kasę i uciekam.
6461): Pracownik: Pusto. Uciekam po Kasjerze
6495): Kierownik: Lokal pusty, wszyscy ewakuowani. Uciekam i zamykam
6459): Symulacja: Koniec tworzenia klientów. Oczekiwanie na koniec ewakuacji
6459): Koniec dnia. Utarg wyniósł: 407 PLN
6459): Symulacja zakończona i wszystko sprzątnięte
```

</details>

Trudno to przedstawić na logach, ale czas od naciśnięcia sygnału ewakuacji do ewakuacji klientów był niemal natychmiastowy, co oznacza pozytywne przejście testu.

</details>

---

<details>
<summary> <strong>2. Test2:Rezerwacja w pełnym lokalu</strong> </summary>

Opis testu: Sprawdzenie czy pracownik poradzi sobie kiedy jest pełne obłożenie w lokalu, klienci bardzo długo jedzą, a on musi wykonać rozkaz rezerwacji stolika. Kiedy logi symulacji staną przez czas jedzenia klientów należy wysłać sygnał rezerwacji i zobaczyć czy symulacja sobie z tym poradzi i się nie zdeadlockuje.

Zmiany w kodzie:

W `struktury.h`: `const  int  ILOSC_KLIENTOW  =  100;`
								 `STOLIKI_X4 = 3`

W `klient.cpp`: `//symulowanie czasu jedzenia`
							   `usleep(15000000);`
							  
W `main.cpp`: `zakomentować usleep w tworzeniu klientów`

---

<details>
<summary>Logi potwierdzające działanie:</summary>

```
.
.
.
9004): Klient: 9004 dostałem jedzenie i stolik: 8 typ: 2, pora jeść
*9164): Kierownik: Wysyłam do pracownika SIGUSR2 - rezerwację ustaloną w kodzie pracownika
*8796): Pracownik (SIGUSR2): Rezerwuję 2 stolików typu 4
8812): Klient: 8812 idę zwrócić naczynia
8798): Klient: 8798 idę zwrócić naczynia
8812): Klient: 8812 zwalniam miejsce i wychodzę
8798): Klient: 8798 zwalniam miejsce i wychodzę
9168): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 18
9167): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 52
8800): Klient: 8800 idę zwrócić naczynia
8801): Klient: 8801 idę zwrócić naczynia
8800): Klient: 8800 zwalniam miejsce i wychodzę
8801): Klient: 8801 zwalniam miejsce i wychodzę
9173): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 76
9174): Klient: wchodzę do baru 1 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 33
8809): Klient: 8809 idę zwrócić naczynia
8799): Klient: 8799 idę zwrócić naczynia
8813): Klient: 8813 idę zwrócić naczynia
8809): Klient: 8809 zwalniam miejsce i wychodzę
8799): Klient: 8799 zwalniam miejsce i wychodzę
8813): Klient: 8813 zwalniam miejsce i wychodzę
9179): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 64
9180): Klient: wchodzę do baru 4 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 56
9181): Klient: 3 osoba/y tylko ogląda i wychodzi
9189): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 20
*8796): Pracownik: Zakończono rezerwację (SIGUSR2)
8814): Klient: 8814 idę zwrócić naczynia
8796): Pracownik: Odbieram naczynia od 8814
.
.
.
9036): Klient: 9036 dostałem jedzenie i stolik: 8 typ: 2, pora jeść
*9164): Kierownik: Wysyłam do pracownika SIGUSR2 - rezerwację ustaloną w kodzie pracownika
*8796): Pracownik (SIGUSR2): Rezerwuję 1 stolików typu 4
8908): Klient: 8908 idę zwrócić naczynia
8815): Klient: 8815 idę zwrócić naczynia
8908): Klient: 8908 zwalniam miejsce i wychodzę
8815): Klient: 8815 zwalniam miejsce i wychodzę
9278): Klient: wchodzę do baru 3 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 82
9279): Klient: wchodzę do baru 2 osoba/y zamawiamy każdy sobie danie i musimy zapłacić 35
*8796): Pracownik: Zakończono rezerwację (SIGUSR2)
8795): Kasjer: mam klienta: 9278 grupa 3 osobowa, zapłacił 82
.
.
.
```

</details>

Jak można zobaczyć, pracownik nie zdeadlockował się mimo długiego czasu jedzenia klientów i oczekiwania na nich - był w stanie opróżnić wymagane miejsca żeby dokonać rezerwacji, a następnie wrócić do normalnej obsługi.

</details>

---

<details>
<summary> <strong>3. Test3:Ewakuacja przy przepełnionej kolejce komunikatów</strong> </summary>

Opis testu: Test polega na specjalnym wywołaniu przepełnienia w kolejce komunikatów przez ustawienie bardzo dużej liczby na semaforze chroniącym do niej dostęp. Po zawieszeniu się symulacji (można to sprawdzić wpisując ipcs -q i zobaczyć czy kolejka ma 16368 bajtów) wysyła się sygnał 3 od kierownika i powinna nastąpić poprawna ewakuacja.

Zmiany w kodzie:

W `struktury.h`: `const  int  ILOSC_KLIENTOW  =  5000;`
								 `const  int  LIMIT_W_BARZE  =  5000;`
								 `const  int  STOLIKI_X1  =  500;`
								`const  int  STOLIKI_X2  =  500;`
								`const  int  STOLIKI_X3  =  800;`
								`const  int  STOLIKI_X4  =  500;`

W `pracowik.cpp`: `dodanie zaraz na starcie pętli while(true):` `usleep(10000000);`
							  
W `main.cpp`: `zakomentować usleep w pętli generowania klientów`

---

<details>
<summary>Logi potwierdzające działanie:</summary>

```
.
.
.
81224): Kasjer: Przypisano stolik 173 typu: 2 dla: 82858
81224): Kasjer: Przypisano stolik 174 typu: 2 dla: 82865
81225): Pracownik: Wydaję posiłek dla 81250 (stolik 0)
81250): Klient: 81250 dostałem jedzenie i stolik: 0 typ: 4, pora jeść
81224): Kasjer: Przypisano stolik 155 typu: 3 dla: 82859
81224): Kasjer: Przypisano stolik 175 typu: 2 dla: 82862
81224): Kasjer: Przypisano stolik 162 typu: 1 dla: 82852
81224): Kasjer: Przypisano stolik 184 typu: 4 dla: 82899
81224): Kasjer: Przypisano stolik 163 typu: 1 dla: 82925
81224): Kasjer: Przypisano stolik 164 typu: 1 dla: 82874
81224): Kasjer: Przypisano stolik 156 typu: 3 dla: 82883
81224): Kasjer: Przypisano stolik 185 typu: 4 dla: 82924
81224): Kasjer: Przypisano stolik 165 typu: 1 dla: 82863
81249): Klient: 81249 idę zwrócić naczynia
81225): Pracownik: Wydaję posiłek dla 81248 (stolik 4)
81250): Klient: 81250 idę zwrócić naczynia
*86803): Kierownik: Ogłaszam pożar i ewakuację!
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
*Tu nastąpił deadlock i po wysłaniu sygnału rozpoczęła się ewakuacja
.
.
.
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
81224): Kasjer: Wszyscy klienci ewakuowani. Zamykam kasę i uciekam.
81225): Pracownik: Pusto. Uciekam po Kasjerze
86803): Kierownik: Lokal pusty, wszyscy ewakuowani. Uciekam i zamykam
81223): Koniec dnia. Utarg wyniósł: 257358 PLN
81223): Symulacja zakończona i wszystko sprzątnięte
```

</details>

Jak widać z logów, przed sygnałem kierownika nastąpił deadlock, a symulacja stanęła przez zapełnienie się kolejki komunikatów. Mimo tego po wysłaniu sygnału 3, symulacja poprawnie się zamknęła.

</details>

---

<details>
<summary> <strong>4.Test4:Poprawne liczenie + odporność na spam</strong> </summary>

Opis testu: Ten test ma za zadanie sprawdzić czy symulacja przy dużym obciążeniu i szybkości działania nadal poprawnie liczy przychód (czy nie są pomijane żadne wartości przez słabą synchronizację) i czy jest jednocześnie odporna na spam sygnałów 1 i 2 od kierownika. (po informacji o zakończeniu należy nacisnąć `ctrl+c` aby zobaczyć podsumowanie)

Zmiany w kodzie:

We wszystkich plikach: `należy zakomentować usleepy`

W `struktury.h`: `const  int  STOLIKI_X4  =  51;`

W `klient.cpp`: `należy zakomentować blok oglądaczy`
							   `należy zakomentować blok sprawdzania limitu kolejki`  
							   `int  suma_zamowienia  =  std::accumulate(koszty.begin(),  koszty.begin(), 1); //linia 198`
							  
W `main.cpp`:

<details>
<summary>zmiana głównej pętli tworzenia klientów na:</summary>

```cpp
    long wygenerowano_lacznie = 0;   
    while(!koniec) {
        semaforOpusc(sem_id, SEM_MAIN);
        if (pam->pozar) {
            semaforPodnies(sem_id, SEM_MAIN);
            koniec = 1;
            break;
        }
        semaforPodnies(sem_id, SEM_MAIN);
        if (wygenerowano_lacznie < ILOSC_KLIENTOW) {
            if (liczba_aktywnych_klientow > 500) {
                usleep(1000); 
                continue;
            }
            int wielkosc_grupy = (rand() % 3) + 1;
            std::string wielkosc = std::to_string(wielkosc_grupy);
            pid = fork();
            if (pid == 0) {
                execl("./klient", "klient", wielkosc.c_str(), NULL);
                perror("Błąd execl klient");
                exit(1);
            }
            else if (pid > 0) {
                std::lock_guard<std::mutex> lock(mutex_procesy);
                procesy_potomne.push_back(pid);
                liczba_aktywnych_klientow++;
                wygenerowano_lacznie++;
            }
        }
        else {
            if (liczba_aktywnych_klientow == 0) {
                logger("Symulacja: Wygenerowano i obsłużono " + 		     std::to_string(wygenerowano_lacznie) + " klientów. Koniec.");
                break;
            }
            usleep(10000);
        }
    }
```

</details>

---

<details>
<summary>Logi potwierdzające działanie:</summary>

```
.
.
.
273788): Pracownik: Odbieram naczynia od 302943
302943): Klient: 302943 zwalniam miejsce i wychodzę
273788): Pracownik: Odbieram naczynia od 302972
302972): Klient: 302972 zwalniam miejsce i wychodzę
273786): Symulacja: Wygenerowano i obsłużono 10000 klientów. Koniec.
273786): Symulacja: Koniec tworzenia klientów. Oczekiwanie na koniec ewakuacji
*273786): Koniec dnia. Utarg wyniósł: 10000 PLN
273786): Symulacja zakończona i wszystko sprzątnięte
```

</details>

Jak widać symulacja nie zawiesiła się mimo dużego obciążenia i ciągłych sygnałów wysyłanych do pracownika, a nawet przy tak dużym obciążeniu i szybkości wynik wyniósł poprawnie 10000, co oznacza dobrą synchronizację tego procesu.

</details>

---

<details>
<summary> <strong>5.Test5:Ewakuacja w trakcie rezerwacji + usuwanie tłumów przed barem</strong> </summary>

Opis testu: Ten test sprawdza czy ewakuacja poprawnie przerwie rezerwację i doprowadzi do końca programu i dodatkowo sprawdzenie czy wątek sprzątający poprawnie i odpowiednio szybko usuwa procesy. W momencie kiedy logi staną z powodu jedzenia klientów (można chwilę odczekać aby main dokończył tworzenie 10k procesów klientów co można sprawdzić 1)`ctrl+z` 2)`ipcs -a`, a następnie `fg` i wtedy wysłać te 2 sygnały) należy wysłać sygnał 2 od kierownika, a następnie od razu sygnał 3. Program powinien przerwać rezerwację i poprawnie przeprowadzić ewakuację czyszcząc wszystko po sobie.

Zmiany w kodzie:

W `struktury.h`: `const  int  LIMIT_W_BARZE  =  10;`
								 `const  int  STOLIKI_X4  =  3;`

W `klient.cpp`: `//symulowanie czasu jedzenia`
							   `usleep(15000000);`
							  
W `main.cpp`: `zakomentować usleep w tworzeniu klientów`

---

<details>
<summary>Logi potwierdzające działanie:</summary>

```
.
.
.
322650): Pracownik: Wydaję posiłek dla 332671 (stolik 2)
332671): Klient: 332671 dostałem jedzenie i stolik: 2 typ: 2, pora jeść
322649): Kasjer: Przypisano stolik 0 typu: 1 dla: 332669
322650): Pracownik: Wydaję posiłek dla 332669 (stolik 0)
332669): Klient: 332669 dostałem jedzenie i stolik: 0 typ: 1, pora jeść
*326395): Kierownik: Wysyłam do pracownika SIGUSR2 - rezerwację ustaloną w kodzie pracownika
*322650): Pracownik (SIGUSR2): Rezerwuję 2 stolików typu 4
*326395): Kierownik: Ogłaszam pożar i ewakuację!
322648): Symulacja: Koniec tworzenia klientów. Oczekiwanie na koniec ewakuacji
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
Klient: POŻAR! Uciekam z lokalu
322649): Kasjer: Wszyscy klienci ewakuowani. Zamykam kasę i uciekam.
322650): Pracownik: Pusto. Uciekam po Kasjerze
326395): Kierownik: Lokal pusty, wszyscy ewakuowani. Uciekam i zamykam
322648): Koniec dnia. Utarg wyniósł: 1113 PLN
322648): Symulacja zakończona i wszystko sprzątnięte
```

</details>

Symulacja "stanęła" na czas jedzenia klientów i wtedy dając trochę czasu (wykorzystałem zatrzymanie symulacji by sprawdzić kiedy będzie 10k procesów) wysłałem kierownikiem sygnał 2 i od razu 3, co jak widać przerwało rezerwację i przeprowadziło poprawną ewakuację, a w dodatku bardzo sprawnie pozbyło się wszystkich procesów klientów, którzy "czekali" przed barem. 

</details>

---
### 7. Funkcje systemowe z linkami do kodu:
Poniżej znajdują się odnośniki do miejsc w kodzie gdzie użyto wymaganych mechanizmów:

**a.** Tworzenie i obsługa plików:
- `open()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/logger.cpp#L34)
- `close()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/logger.cpp#L74)
- `write()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/logger.cpp#L18)

**b.** Tworzenie procesów:
- `fork()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/main.cpp#L136)
- `exit()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L19)

**c.** Tworzenie i obsługa wątków:
- `pthread_create()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/main.cpp#L158)
- `pthread_join()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/klient.cpp#L194)
- `pthread_detach()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/main.cpp#L159)
- `pthread_mutex_lock`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/main.cpp#L37)

**d.** Obsługa sygnałów:
- `kill()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/kierownik.cpp#L85)
- `signal()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/main.cpp#L74)
- `sigaction()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/pracownik.cpp#L39)

**e.** Synchronizacja procesów:
- `ftok()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L16)
- `semget()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L28)
- `semctl()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L34)
- `semop()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L73)

**g.** Segmenty pamięci dzielonej:
- `ftok()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L16)
- `shmget()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L160)
- `shmat()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L175)
- `shmdt()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L184)
- `shmctl()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L169)

**h.** Kolejki komunikatów:
- `ftok()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L16)
- `msgget()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L203)
- `msgsnd()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L236)
- `msgrcv()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L285)
- `msgctl()`: [zobacz w kodzie](https://github.com/MrKacper828/SO_Bar_Mleczny/blob/481c80c6a5bcff2865617c5c78a9c0112dc7022b/src/operacje.cpp#L212)

