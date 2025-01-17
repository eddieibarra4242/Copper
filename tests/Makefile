CC = gcc

CFLAGS = -I../Unity/src/ -I../src/
LINK_FLAGS = -L../bin -lunity

TESTS = scanner.test.o

all: mkdirs Unity compiler $(TESTS)
	$(CC) ../bin/int/scanner.o ../bin/int/scanner.test.o ../bin/int/scanner.runner.o -o ../bin/tests/scanner.test $(LINK_FLAGS)

%.test.o: %.test.c
	ruby ../Unity/auto/generate_test_runner.rb $< $(<:%.test.c=%.runner.c)
	$(CC) -c $(CFLAGS) $< -o ../bin/int/$@
	$(CC) -c $(CFLAGS) $(<:%.test.c=%.runner.c) -o ../bin/int/$(@:%.test.o=%.runner.o)

compiler:
	cd ../src && $(MAKE)

mkdirs:
	mkdir -p ../bin/tests

Unity:
	cd ../Unity && meson setup builddir
	cd ../Unity/builddir && meson compile
	cp ../Unity/builddir/libunity.a ../bin