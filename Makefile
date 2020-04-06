## Source files
src = $(wildcard src/*.c)

## Source code of external libraries
lib = $(wildcard lib/gumbo/*.c) \
	$(wildcard lib/gumbo/*.c) \
	$(wildcard lib/log/*.c) \
	$(wildcard lib/map/*.c) \
	$(wildcard lib/sds/*.c) \
    $(wildcard lib/vec/*.c)

DEPS = $(wildcard lib/gumbo/*.h) \
	$(wildcard lib/gumbo/*.h) \
	$(wildcard lib/log/*.h) \
	$(wildcard lib/map/*.h) \
	$(wildcard lib/sds/*.h) \
    $(wildcard lib/vec/*.h) \
    $(wildcard src/*.h)

OBJ = $(src:.c=.o) $(lib:.c=.o)

EXE = crawler
CFLAGS = -g -Wall -O3 -std=gnu99
CC = gcc

## Create .o files from .c files. Searches for .c files with same .o names given in OBJ
$(EXE): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

## Create executable linked file from object files.
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Perform clean
clean:
	find . -name '*.o' -delete
	rm $(EXE)