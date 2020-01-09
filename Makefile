all: hackrpath hello

CFLAGS = -Wall -Wpedantic -std=gnu99 -fpic -g
CXXFLAGS = -Wall -Wpedantic -fpic -g

hackrpath.o: elf.h

hackrpath: hackrpath.o
	$(CC) -o $@ $^

libhello/libhello.so: libhello/libhello.o
	$(CC) -shared -o $@ $^

hello: hello.o libhello/libhello.so
	$(CC) -o $@ hello.o -Llibhello -lhello

libhello/libhello++.so: libhello/libhello++.o
	$(CXX) -shared -o $@ $^

hello++: hello++.o libhello/libhello++.so
	$(CXX) -o $@ hello++.o -Llibhello -lhello++

test: hackrpath hello
	./hello || true
	./hackrpath --set-rpath \$$ORIGIN/libhello hello
	./hackrpath --print-rpath hello
	./hello
	@echo "All done!"

clean:
	rm -f *.o hackrpath hello libhello/libhello.o
