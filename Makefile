all: hackrpath hello

CFLAGS = -Wall -Wpedantic -std=gnu99 -O2

clean:
	rm -f *.o hackrpath hello
