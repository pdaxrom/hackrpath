all: hackrpath hello

CFLAGS = -Wall -Wpedantic -std=gnu99 -fpic -g

hackrpath.o: elf.h

hackrpath: hackrpath.o
	$(CC) -o $@ $^

libhello/libhello.so: libhello/libhello.o
	$(CC) -shared -o $@ $^

hello: hello.o libhello/libhello.so
	$(CC) -o $@ hello.o -Llibhello -lhello

clean:
	rm -f *.o hackrpath hello libhello/libhello.o
