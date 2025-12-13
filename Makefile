CXX = g++

CXXFLAGS = -Wall -std=c++11 -Iinclude -pthread

SOURCES = src/main.cpp src/operacje.cpp src/procesy.cpp src/logger.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = bar_mleczny

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) -lrt

%.O: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET)