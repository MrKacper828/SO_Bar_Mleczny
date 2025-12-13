CXX = g++

CXXFLAGS = -Wall -std=c++11 -Iinclude -pthread

OBJECTS = src/operacje.o src/procesy.o src/logger.o
TARGETS = main klient kasjer pracownik kierownik

all: $(TARGETS)

main: src/main.o $(OBJECTS)
	$(CXX) src/main.o $(OBJECTS) -o main -lrt

klient: src/klient.o $(OBJECTS)
	$(CXX) src/klient.o $(OBJECTS) -o klient -lrt

kasjer: src/kasjer.o $(OBJECTS)
	$(CXX) src/kasjer.o $(OBJECTS) -o kasjer -lrt

pracownik: src/pracownik.o $(OBJECTS)
	$(CXX) src/pracownik.o $(OBJECTS) -o pracownik -lrt

kierownik: src/kierownik.o $(OBJECTS)
	$(CXX) src/kierownik.o $(OBJECTS) -o kierownik -lrt

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGETS)