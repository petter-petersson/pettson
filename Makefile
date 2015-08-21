-include config.mk

LDFLAGS=-L.
LIBS=-lpettson
#CFLAGS=-g

all: libpettson.a 

libpettson.a: json.o
	$(AR) rc $@ $^

%.o: %.c json.h
	$(CC) -c $(CFLAGS) $< -o $@

test: json_test
	./json_test

json_test: json-tests.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIBS) $< -o $@

json-tests.o: json-tests.c libpettson.a

clean:
	rm -rf *.o *.a json_test

.PHONY: all clean test

