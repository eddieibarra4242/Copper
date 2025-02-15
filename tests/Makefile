CC = gcc

CFLAGS = -I../Unity/src/ -I../src/ -I../test_utils
LINK_FLAGS = -L../bin -lunity -ltestutils

TESTS = scanner.test.o parser.test.o

all: mkdirs $(TESTS)
	$(CC) \
		../bin/int/scanner.o \
		../bin/int/scanner.test.o \
		../bin/int/scanner.runner.o \
		-o ../bin/tests/scanner.test $(LINK_FLAGS)
	$(CC) \
		../bin/int/scanner.o \
		../bin/int/parser.o \
		../bin/int/tree.o \
		../bin/int/parser.test.o \
		../bin/int/parser.runner.o \
		-o ../bin/tests/parser.test $(LINK_FLAGS)
	$(CC) \
		run_all_inputs.c \
		-o ../bin/tests/run_all_inputs.test

%.test.o: %.test.c
	ruby ../Unity/auto/generate_test_runner.rb $< $(<:%.test.c=%.runner.c)
	$(CC) -c $(CFLAGS) $< -o ../bin/int/$@
	$(CC) -c $(CFLAGS) $(<:%.test.c=%.runner.c) -o ../bin/int/$(@:%.test.o=%.runner.o)

mkdirs:
	mkdir -p ../bin/tests