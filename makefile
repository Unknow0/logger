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
	ar rcs $(PROG).a $^
$(PROG).so: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(PROG).so $^

%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean install uninstall

clean:
	rm $(OBJECTS)

install: build
	cp $(PROG)* $(PREFIX)/lib/
	cp $(INCLUDE) $(PREFIX)/include/

uninstall:
	rm $(PREFIX)/lib/$(PROG)*
	rm -r $(PREFIX)/include/$(INCLUDE)
