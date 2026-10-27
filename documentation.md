# Dokumentacja Silnika Geometrycznego 2D (C++/Qt)

## 0. Użyte Technologie i Uruchomienie Projektu

### Technologie:
* **Język programowania:** C++ (standard C++17 lub nowszy)
* **Framework:** Qt (moduły: Qt Core, Qt GUI, Qt Widgets – obsługa okien, zdarzeń myszy oraz silnik rysowania `QPainter`)
* **System budowania:** CMake

---

### Sposób 1: Uruchomienie przez Qt Creator
1. Zainstaluj pakiet Qt wraz ze środowiskiem **Qt Creator** (przez oficjalny *Qt Online Installer*).
2. Uruchom program Qt Creator.
3. Wybierz opcję **File -> Open Project** i wskaż plik **`CMakeLists.txt`** znajdujący się w głównym katalogu projektu.
4. W oknie konfiguracji projektu wybierz dostępny zestaw narzędzi (tzw. *Kit*, np. *Desktop Qt 6.x / MSVC, MinGW lub GCC*) i kliknij **Configure Project**.

---

### Sposób 2: Budowanie z linii poleceń (CMake CLI)
Jeśli posiadasz zainstalowany sam pakiet Qt oraz kompilator z CMake bez środowiska Qt Creator:
1. Otwórz terminal w głównym katalogu projektu.
2. Skonfiguruj projekt, przekazując ścieżkę do katalogu z bibliotekami Qt za pomocą flagi `-DCMAKE_PREFIX_PATH`:
   ```bash
   cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"

## 1. Wprowadzenie i Założenia Projektowe
Aplikacja jest zaawansowanym silnikiem grafiki wektorowej typu CAD 2D, operującym na architekturze wierzchołków (Vertex), krawędzi (Edge) i poligonów (Polygon). Głównym celem projektu jest zapewnienie stabilnego i w czasie rzeczywistym reagującego interfejsu z obsługą dynamicznych ograniczeń (Constraints) – zarówno geometrycznych (długość, równoległość), jak i matematycznych ciągłości krzywych ($G0, G1, C1$).

### Główne założenia:
* **Relaksacyjne Rozwiązywanie Ograniczeń** Zmiana stanu dowolnego wierzchołka rozchodzi się po grafie zależności kaskadowo w oparciu o algorytm BFS. 
* **Hybrydowy model krawędzi:** Odcinek łączący dwa wierzchołki może być interpretowany jako prosta linia, krzywa Béziera (trzeciego stopnia) lub łuk kołowy. Niezależnie od wygięcia, więzy geometryczne (np. odległość, równoległość) operują na wirtualnej prostej linii bazowej łączącej wierzchołki główne.
* **Priorytet Stabilności nad Idealnością:** Algorytm rozwiązywania więzów ma określony maksymalny budżet czasowy (limit iteracji). Jeśli użytkownik nałoży sprzeczne relacje, system odrzuci transakcję, przywracając stabilny stan sceny, zamiast zablokować interfejs lub zniszczyć topologię.

---

## 2. Instrukcja Obsługi (Klawiszologia i Mysz)

Aplikacja opiera się w całości na interakcji kursorem.

### Akcje Lewego Przycisku Myszy (LPM):
* **Kliknięcie i przeciągnięcie wierzchołka głównego:** Zmienia geometrię poligonu. Podłączone więzy są przeliczane kaskadowo.
* **Kliknięcie i przeciągnięcie punktu kontrolnego (Czerwony kwadrat):** Modyfikuje wektory styczne krzywej Béziera. Posiadając ciągłość G1/C1, wymusi również modyfikację sąsiedniej krzywej.
* **Kliknięcie wnętrza figury i przeciągnięcie:** Przesuwa (translacja) cały poligon wraz ze wszystkimi jego krzywymi bez naruszania wewnętrznych więzów.
* **Podwójne kliknięcie (Double-Click) na krawędź:** Dzieli wybraną krawędź na pół i wstawia nowy wierzchołek w miejscu kliknięcia.

### Akcje Prawego Przycisku Myszy (PPM - Menu Kontekstowe):
* **Kliknięcie w Wierzchołek:** 
  * Otwiera opcję usunięcia wierzchołka (jeśli poligon posiada $>3$ wierzchołków).
  * Otwiera podmenu **Continuity** ($G0, G1, C1$) pozwalające wymusić matematyczną gładkość między schodzącymi się w nim krawędziami (opcja dostępna tylko jeśli co najmniej jedna z krawędzi to łuk lub Bézier).
* **Kliknięcie w Krawędź:**
  * **Length Constraint:** Dodawanie lub usuwanie sztywnego wymiaru (wymaga podania długości w oknie dialogowym). 
  * **Parallelism Constraint:** Wymuszanie równoległości względem innej krawędzi (uruchamia tryb selekcji drugiej krawędzi kliknięciem LPM).
  * **Edge Type:** Konwersja krawędzi pomiędzy Odcinkiem ("Line"), Krzywą Béziera a Łukiem Kołowym ("Circular Arc"). 

---

## 3. Architektura i Model Danych

Zastosowano model ustrukturyzowany z silnym naciskiem na cykl życia wskaźników (smart pointers).

### 3.1. Hierarchia Geometrii
* **`Vertex`:** Podstawowy kontener położenia (`QPointF point`). Nie posiada wiedzy o krawędziach. Współdzielony przez krawędzie poprzez `std::shared_ptr`.
* **`Edge`:** Obiekt łączący dwa węzły `Vertex`. Reprezentuje odcinek bazowy. Kompozycyjnie przetrzymuje geometrię pochodną poprzez wskaźniki `std::unique_ptr<BezierCurve>` oraz `std::unique_ptr<ArcCurve>`. 
  * `BezierCurve` przechowuje dwa dodatkowe unikalne obiekty `Vertex` pełniące rolę lokalnych punktów kontrolnych.
  * `ArcCurve` nie posiada własnych wierzchołków – definiuje geometrię za pomocą skalarnego promienia `radius` (ze znakiem definiującym wypukłość) oraz kąta `isMajorArc`.
* **`Polygon`:** Zbiór ułożonych sekwencyjnie krawędzi w `std::vector<std::unique_ptr<Edge>>`. Dba o translację, detekcję punktów (inside-test) oraz dodawanie/usuwanie krawędzi.
* **`Scene`:** Główny kontener przechowujący poligony oraz zarządzający systemem wiązań (`ConstraintManager`).

### 3.2. Zarządzanie Więzami (`ConstraintManager`)
Aplikacja utrzymuje globalny graf powiązań w strukturze:
`std::unordered_map<const Vertex*, std::vector<std::shared_ptr<AbstractConstraint>>> Constraints`

Każdy więz (`LengthConstraint`, `ParallelismConstraint`, `ContinuityConstraint`) dziedziczy z `AbstractConstraint`. Zastosowanie polimorfizmu zmusza konkretne klasy więzów do implementacji funkcji `Fix(immovableVertex)`, `getError()` oraz listy wierzchołków na które dany więz ma wpływ (`affectedVertices()`).

---

## 4. Zaimplementowane Algorytmy 

### 4.1. Solver Relaksacyjny (TrySolveConstraints / SolveConstraints)
Algorytm rozwiązujący zjawisko nakładających się relacji geometrycznych. Opiera się na algorytmie **BFS (Breadth-First Search)** w architekturze Gaussa-Seidela.
1. Punktem startowym kolejki jest zmodyfikowany `Vertex` (lub punkt kontrolny).
2. Algorytm wyciąga węzeł z kolejki, pobiera jego więzy i dla każdego, którego błąd `getError()` przekracza graniczną wartość tolerancji `maxError()`, wykonuje funkcję naprawczą `Fix()`.
3. Następnie wszystkie wierzchołki zgłoszone jako `affectedVertices` przez naprawiony więz trafiają na koniec kolejki (jeśli jeszcze w niej nie były w bieżącym kroku).
4. **Zabezpieczenie Nieskończonej Pętli:** System utrzymuje globalny oraz lokalny (per constraint) licznik iteracji (np. `maxIterationsForConstraint = 500`). Osiągnięcie limitu bez wyzerowania błędów (`success = false`) przerywa kolejkę, niszczy ostatnio dodany więz, a system przywraca pełen zrzut stanu współrzędnych i promieni krawędzi (`previous_state` rollback).

### 4.2. Matematyka Ciągłości ($G1$ i $C1$)
Wymuszanie gładkości między krawędziami dowolnego typu. Solver potrafi rzutować zmiany wektorów stycznych pomiędzy kombinacjami Odcinek ↔ Bézier, Bézier ↔ Bézier, Odcinek/Bézier ↔ Łuk oraz Łuk ↔ Łuk.

* **Pobieranie Tangenty (`getTangent`):** Zunifikowany mechanizm wyliczający gradient wyjściowy ze wspólnego wierzchołka z uwzględnieniem orientacji łuku w przestrzeni, znakowanego promienia oraz krzyżowego iloczynu wektorów (cross product).
* **Ciągłość G1 (Geometryczna):** Wymusza współliniowość z przeciwnym zwrotem. Algorytm `getError()` mierzy odchył norm obydwu wektorów stycznych. W przypadku łuku, algorytm potrafi zmodyfikować sam wektor promienia rzutując wektor sąsiedniej krzywej z powrotem na cięciwę łuku, rozwiązując analitycznie promień, który spełnia zadaną styczność.
* **Ciągłość C1 (Parametryczna):** Oprócz kierunku wymusza dokładną równość siły (np. odległość wierzchołków kontrolnych). Jeśli pociągnięty zostanie punkt kontrolny jednej krzywej Béziera, przeciwległy punkt natychmiast dostosuje swoją pozycję oraz odległość od wierzchołka węzłowego, dając efekt "sztywnej dźwigni".
* **Czyszczenie Relacji:** System inteligentnie filtruje stany przejścia. Usunięcie krawędzi lub zmiana jej geometrii (np. z Łuku na Odcinek) niszczy uprzednio nadane obustronne ciągłości, zabezpieczając pamięć przed wskaźnikami-duchami. (`ContinuityConstraintManager`)
