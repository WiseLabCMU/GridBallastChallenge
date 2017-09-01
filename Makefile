CFLAGS += -Wall -ggdb

.PHONY: all clean

all: interrogate
msg.o: msg.c msg.h
interrogate: interrogate.c msg.o

clean:
	$(RM) interrogate *.o
