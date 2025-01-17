CC = gcc
BISON = bison
OBJDUMP = objdump

CFLAGS = -Wall -Wextra -Wpedantic -Werror

LINK_FLAGS = -L../bin

CFLAGS += -ggdb
LINK_FLAGS += -ggdb

# Debug generated Bison parser.
CFLAGS += -DYYDEBUG=1

# CFLAGS += -O2 -DNDEBUG
# LINK_FLAGS += -O2

OUTPUT_NAME = cu

OBJ_FILES = main.o log.o scanner.o parser.o ast.o

all: mkdirs $(OBJ_FILES)
	$(CC) $(OBJ_FILES:%=../bin/int/%) -o ../bin/$(OUTPUT_NAME) $(LINK_FLAGS)
	$(OBJDUMP) -dCM intel ../bin/$(OUTPUT_NAME) > ../debug/$(OUTPUT_NAME).dump

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o ../bin/int/$@

mkdirs:
	mkdir -p ../debug
	mkdir -p ../bin/int

parser.c: parser.y
	$(BISON) --output parser.c parser.y