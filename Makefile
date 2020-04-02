src = $(wildcard src/*.c)
lib = $(wildcard lib/gumbo/*.c) \
	$(wildcard lib/gumbo/*.c) \
	$(wildcard lib/log/*.c) \
	$(wildcard lib/map/*.c) \
	$(wildcard lib/sds/*.c) \
    $(wildcard lib/vec/*.c)
obj = $(src:.c=.o) $(lib:.c=.o)
OBJDIR := obj
TARGET := crawler
CFLAGS = -g  -Wall -O3
CC = gcc

crawler: $(obj)
	$(CC) -o $@ $^ $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<