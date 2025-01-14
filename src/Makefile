CC = gcc
BISON = bison
OBJDUMP = objdump

CFLAGS = -Wall -Wextra -Wpedantic -Werror

# CFLAGS += -ggdb
CFLAGS += -O2

LINK_FLAGS = -L../bin

# LINK_FLAGS += -ggdb
LINK_FLAGS += -O2

OUTPUT_NAME = cu

OBJ_FILES = main.o

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