CC=gcc
LD=$(CC)
CPPFLAGS=-g -std=gnu11 -Wpedantic -Wall -Wextra
CFLAGS=-I.
LDFLAGS=
LDLIBS=-lm -lrt -lpthread
PROGRAM=timing

all: $(PROGRAM)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(PROGRAM).o
	$(LD) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) -o $@ $<

.PHONY:
clean:
	rm -f *.o $(PROGRAM)
