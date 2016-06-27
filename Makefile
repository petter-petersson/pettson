-include config.mk             

LDFLAGS=-L.
LIBS=-lpettson
override CFLAGS+=-std=c99                

all: libpettson.a              

libpettson.a: json.o           
	$(AR) rc $@ $^         

%.o: %.c json.h                
	$(CC) -c $(CFLAGS) $< -o $@

test: json_test
	./json_test

json_test: json-tests.o libpettson.a
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	rm -rf *.o *.a json_test

.PHONY: all clean test
