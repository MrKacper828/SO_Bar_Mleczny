CXX = g++

CXXFLAGS = -Wall -std=c++11 -Iinclude -pthread

OBJECTS = src/operacje.o src/logger.o
TARGETS = main klient kasjer pracownik kierownik

all: $(TARGETS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

main: src/main.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o main -lrt

klient: src/klient.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o klient -lrt

kasjer: src/kasjer.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o kasjer -lrt

pracownik: src/pracownik.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o pracownik -lrt

kierownik: src/kierownik.o $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o kierownik -lrt

clean:
	rm -f src/*.o $(TARGETS)