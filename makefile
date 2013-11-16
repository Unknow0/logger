PROG=liblogger
CC=gcc
CFLAGS=-g
LDFLAGS=-lcfg -lcontainer -ljson
PREFIX=/usr/local

SRC=$(filter-out main.c, $(wildcard *.c))
OBJECTS=$(SRC:.c=.o)
INCLUDE=$(wildcard *.h)

all: build test

build: $(PROG).a $(PROG).so

$(PROG).a: $(OBJECTS)
	rm -f $(PROG).a
	ar rcs $(PROG).a $^
$(PROG).so: $(OBJECTS)
	$(CC) $(LDFLAGS) -shared -o $(PROG).so $^

test: main.c $(PROG).a
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o test

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean install uninstall

clean:
	rm -f *.o
	rm -f test
	rm -f $(PROG)*

install: build
	cp $(PROG)* $(PREFIX)/lib/
	cp $(INCLUDE) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(PROG)*
	rm -rf $(PREFIX)/include/$(INCLUDE)
