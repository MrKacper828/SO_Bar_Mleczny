# Temat 14: Bar Mleczny - Opis Projektu i Plan Testów

Autor: [Kacper Korpacki]

Nr albumu: [155192]

Link do repozytorium: https://github.com/MrKacper828/SO_Bar_Mleczny

---

## 1. Opis zadania

Celem projektu jest stworzenie symulacji funkcjonowania Baru Mlecznego z wykorzystaniem mechanizmów systemów operacyjnych (IPC). Symulacja modeluje zachowanie klientów, pracowników obsługi, kasjera oraz kierownika, zarządzając ograniczonymi zasobami (stoliki) i reagując na sygnały sterujące.


### 1.1. Procesy w systemie

1.  **Klient (Grupa):** Reprezentuje pojedynczego klienta lub grupę (2 do 4 osób). Klienci przychodzą w losowych odstępach czasu.

2.  **Pracownik obsługi:** Odpowiada za wydawanie posiłków oraz odbieranie poleceń od kierownika.

3.  **Kasjer:** Odpowiada za przyjmowanie płatności.

4.  **Kierownik:** Proces nadrzędny, mogący wysyłać sygnały sterujące mające wpływ na rozstawienie stolików w lokalu oraz specjalne akcje (pożar).

  
### 1.2. Zasoby i synchronizacja

Zasobem krytycznym są stoliki o różnej pojemności ($X_1, X_2, X_3, X_4$).

Mechanizmy synchronizacji muszą spełniać następujące zasady:

-  **Dostępność miejsc:** Klient nie może złożyć zamówienia i zapłacić, jeśli nie ma gwarancji wolnego miejsca przy stoliku (nie może czekać z gorącym daniem). Rezerwacja stolika następuje przed lub w trakcie płatności.

-  **Logika zajmowania stolików:** Różne grupy mogą siedzieć przy tym samym stoliku tylko wtedy, gdy są równoliczne (np. dwie grupy 2-osobowe przy stoliku 4-osobowym). W przeciwnym wypadku grupa zajmuje stolik na wyłączność.

-  **Oglądacze:** 5% klientów wchodzi, ale nie zamawia (zajmują zasoby wejścia, ale nie zajmują stolików).

-  **Kolejkowanie:** Kasa i punkt wydawania posiłków są zasobami, do których dostęp musi być synchronizowany (kolejkowanie).

  
### 1.3. Interakcja z Kierownikiem

System obsługuje sygnały od Kierownika wywoływane na życzenie:

-  **Sygnał 1:** Podwojenie liczby stolików 3-osobowych ($X_3$). Operacja jednorazowa.

-  **Sygnał 2:** Rezerwacja (wyłączenie z użytku) określonej liczby miejsc.

-  **Sygnał 3 :** Pożar, natychmiastowe przerwanie konsumpcji, ewakuacja klientów (bez zwrotu naczyń), zakończenie pracy obsługi i zamknięcie kasy.

----------

## 2. Plan testów symulacji

 
Poniższe testy mają na celu weryfikację poprawności działania mechanizmów synchronizacji oraz logiki działania baru.

 
### Test 1: Standardowa obsługa klienta i przydzielanie stolików

-  **Warunki początkowe:** Standardowa liczba stolików.

-  **Akcja:** Uruchomienie generatora klientów (grup 1, 2, 3, 4-osobowych).

-  **Oczekiwany rezultat:**

- Klienci pobierają zamówienia i siadają tylko wtedy, gdy są wolne miejsca.

- W logach brak sytuacji, w której klient czeka z jedzeniem.

- Zwolnienie stolika następuje po komunikacie o zwrocie naczyń.

 
### Test 2: Weryfikacja reguły równoliczności przy stolikach

-  **Warunki początkowe:** Ograniczona liczba stolików, duże nasycenie klientami.

-  **Akcja:** Wpuszczenie grupy 1-osobowej, a następnie grupy 3-osobowej do baru, gdzie dostępny jest tylko jeden duży stolik (4 os.).

-  **Oczekiwany rezultat:**

- System nie pozwala usiąść grupie 3-osobowej przy stoliku zajętym częściowo przez grupę 1-osobową (różna liczebność).

- System pozwala usiąść drugiej grupie 1-osobowej przy stoliku zajętym przez inną grupę 1-osobową (mowa oczywiście o tym większym stoliku).


### Test 3: Klienci niezamawiający (5%)

-  **Akcja:** Długotrwała symulacja przy dużej liczbie wygenerowanych wątków.

-  **Oczekiwany rezultat:**

- Około 5% pojawiających się klientów wchodzi do baru, ale nie generuje zdarzeń Zapłaty ani Zajęcia stolika, po czym opuszcza system.

 
### Test 4: Obsługa Sygnału 1 (Dostawienie stolików)

-  **Akcja:** Kierownik wysyła sygnał zwiększenia liczby stolików $X_3$. Po pewnym czasie wysyła go ponownie.

-  **Oczekiwany rezultat:**

- Po pierwszym sygnale pula dostępnych miejsc dla grup 3-osobowych wzrasta dwukrotnie.

- Drugi sygnał jest ignorowany lub zwraca komunikat o braku możliwości ponownego wykonania operacji.

  
### Test 5: Obsługa Sygnału 2 (Rezerwacja miejsc)

-  **Akcja:** Kierownik żąda rezerwacji $K$ miejsc w trakcie dużego obłożenia lokalu.

-  **Oczekiwany rezultat:**

- Liczba wolnych miejsc w systemie spada.

- Jeśli liczba żądanych miejsc przewyższa wolne, system blokuje wpuszczanie nowych klientów do momentu zwolnienia się wystarczającej liczby stolików, aby spełnić żądanie rezerwacji.

 
### Test 6: Obsługa Sygnału 3 (Pożar - Scenariusz kończący)

-  **Akcja:** W trakcie pełnego obłożenia baru Kierownik wysyła sygnał Pożar.

-  **Oczekiwany rezultat:**

- Wszyscy obecni klienci natychmiast przerywają wątki oczekiwania/jedzenia i logują wyjście (Ewakuacja).

- Brak logów o zwrocie naczyń po sygnale pożaru.

- Pracownicy obsługi i kasjer kończą pracę dopiero po wyjściu ostatniego klienta.

- Program kończy działanie.
