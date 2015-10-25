CC=gcc
CFLAGS=-pthread

all: ./server/server.c ./client/client.c ./common/common.c
	mkdir -p ../bin/
	$(CC) $(CFLAGS) -o ../bin/server.o ./server/server.c ./common/common.c
	$(CC) $(CFLAGS) -o ../bin/client.o ./client/client.c ./common/common.c

clean:
	rm ../bin/server.o ../bin/client.o
