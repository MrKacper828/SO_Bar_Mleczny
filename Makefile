CC = g++

CFLAGS = -Wall -std=c++17 -Iinclude -pthread

SOURCES = src/main.cpp src/operacje.cpp src/procesy.cpp src/logger.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = bar_mleczny

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) -lrt

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(EXECUTABLE)