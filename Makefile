CC=gcc
CFLAGS=-std=c11 -O3 -g -Wall -Wextra -Werror -pedantic -Wno-unused-parameter
CFLAGS+=$(shell pkg-config --cflags sdl3)
LDFLAGS=$(shell pkg-config --libs sdl3)

BINDIR=bin
SRCDIR=src
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:$(SRCDIR)/%.c=$(BINDIR)/%.o)

all: $(BINDIR)/chip8

$(BINDIR)/chip8: $(OBJS)
	@mkdir -p $(BINDIR)
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINDIR)

.PHONY: all clean
