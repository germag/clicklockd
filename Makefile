CC=c99
CFLAGS=-c -O -Wall -D_GNU_SOURCE
LDFLAGS= -ludev
SOURCES=$(wildcard *.c)
HEADERS=$(wildcard *.h)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=clicklockd

all: $(SOURCES) $(HEADERS) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

depend: .depend

.depend: $(SOURCES)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^ >> ./.depend

include .depend

clean clear:
	rm -f $(OBJECTS) $(EXECUTABLE) ./.depend
