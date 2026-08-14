CC = aarch64-linux-gnu-gcc

CFLAGS = -Wall -Wextra -Wpedantic -g \
         -I./include \
         -I$(HOME)/imx8mn/cross_test/tinyalsa/build-imx8/tinyalsa-install/usr/include

LDFLAGS = -L$(HOME)/imx8mn/cross_test/tinyalsa/build-imx8/tinyalsa-install/usr/lib

LDLIBS = -ltinyalsa -lm

TARGET = build/audio_hal

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build/*