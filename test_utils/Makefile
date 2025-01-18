CC = gcc
AR = ar
OBJDUMP = objdump

CFLAGS = -Wall -Wextra -Wpedantic -Werror -I../Unity/src/ -I../src/

#CFLAGS += -ggdb
CFLAGS += -O2

LINK_FLAGS = rcs

OBJ_FILES = tokens.o log_utils.o

all: mkdirs $(OBJ_FILES)
	$(AR) $(LINK_FLAGS) ../bin/libtestutils.a $(OBJ_FILES:%=../bin/int/test_utils/%)
	$(OBJDUMP) -dC ../bin/libtestutils.a > ../debug/test_utils.dump

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o ../bin/int/test_utils/$@

mkdirs:
	mkdir -p ../bin/int/test_utils