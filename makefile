PROG=liblogger
CC=gcc
CFLAGS=-g
LDFLAGS=-shared
PREFIX=/usr/local

SRC=$(wildcard *.c)
OBJECTS=$(SRC:.c=.o)
INCLUDE=$(wildcard *.h)

all: build

build: $(PROG).a $(PROG).so

$(PROG).a: $(OBJECTS)
	rm -f $(PROG).a
	ar rcs $(PROG).a $^
$(PROG).so: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(PROG).so $^

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean install uninstall

clean:
	rm -f $(OBJECTS)
	rm -f $(PROG)*

install: build
	cp $(PROG)* $(PREFIX)/lib/
	cp $(INCLUDE) $(PREFIX)/include/

uninstall:
	rm -f $(PREFIX)/lib/$(PROG)*
	rm -rf $(PREFIX)/include/$(INCLUDE)
