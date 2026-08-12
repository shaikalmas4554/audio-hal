CC = gcc

CFLAGS = -Wall -Wextra -g

TARGET = build/audio_hal

SRC = $(wildcard src/*.c)

OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build/*
