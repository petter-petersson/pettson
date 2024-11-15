-include config.mk             

LDFLAGS=-L.
LIBS=-lpettson
override CFLAGS+=-std=c99 -g

all: libpettson.a              

libpettson.a: json.o           
	$(AR) rc $@ $^         

%.o: %.c json.h                
	$(CC) -c $(CFLAGS) $< -o $@

test: json_test
	./json_test

json_test: json-tests.o libpettson.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

sample: functional/sample.o functional/store.o libpettson.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -rf **/*.o *.a json_test store_test

.PHONY: all clean test
