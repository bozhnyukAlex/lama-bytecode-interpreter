TARGET = lama-interpreter
CC = gcc
COMMON_FLAGS=-O2 -g -m32 -fstack-protector-all

all: build build/gc_runtime.o build/runtime.o build/vm.o
	$(CC) $(COMMON_FLAGS) build/gc_runtime.o build/runtime.o build/vm.o -o build/$(TARGET)

build/vm.o: build src/vm.c src/headers/vm.h src/headers/value.h src/headers/common.h
	$(CC) $(COMMON_FLAGS) -I src/headers -c src/vm.c -o build/vm.o

build/gc_runtime.o: build src/runtime/gc_runtime.s
	$(CC) $(COMMON_FLAGS) -I src/headers -c src/runtime/gc_runtime.s -o build/gc_runtime.o

build/runtime.o: build src/runtime/runtime.c src/runtime/runtime.h
	$(CC) $(COMMON_FLAGS) -c src/runtime/runtime.c -o build/runtime.o

build:
	mkdir build
	touch build/.gitignore

clean:
	$(RM) -f $(TARGET) *.o *.bc -r build