TARGET = lama-interpreter
CC = gcc
COMMON_FLAGS=-O2 -m32 -g2 -fstack-protector-all

all: build build/runtime.o build/gc.o build/vm.o
	$(CC) $(COMMON_FLAGS) build/runtime.o build/gc.o build/vm.o -o build/$(TARGET)

build/vm.o: build src/vm.c
	$(CC) -I headers $(COMMON_FLAGS) -c src/vm.c -o build/vm.o

build/runtime.o: build src/runtime src/runtime
	$(CC) $(COMMON_FLAGS) -c src/runtime/runtime.c -o build/runtime.o

build/gc.o: build src/runtime src/runtime
	$(CC) $(COMMON_FLAGS) -c src/runtime/gc.c -o build/gc.o

build:
	mkdir build

clean:
	$(RM) -f $(TARGET) *.o -r build