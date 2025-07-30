# Makefile para Pong con SDL2

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LIBS = -lSDL2 -lSDL2_mixer -lm
TARGET = pong
SOURCES = main.cpp

# Detectar flags de SDL2 automáticamente
SDL2_CFLAGS = $(shell pkg-config --cflags sdl2)
SDL2_LIBS = $(shell pkg-config --libs sdl2) -lSDL2_mixer

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SDL2_CFLAGS) -o $(TARGET) $(SOURCES) $(SDL2_LIBS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

install-deps:
	sudo apt update
	sudo apt install -y libsdl2-dev libsdl2-mixer-dev build-essential pkg-config

.PHONY: all clean run install-deps
