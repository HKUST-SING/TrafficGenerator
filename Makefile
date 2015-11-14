CC = gcc
CFLAGS = -c -Wall -pthread -lm
LDFLAGS = -pthread -lm
TARGETS = poission-client simple-client server
BIN_DIR = bin
SRC_DIR = src

all: directory $(TARGETS) move

directory:
	mkdir -p $(BIN_DIR)

move:
	mv *.o $(TARGETS) $(BIN_DIR)

poission-client: common.o cdf.o conn.o poission-client.o
	$(CC) $(LDFLAGS) common.o cdf.o conn.o poission-client.o -o poission-client

simple-client: common.o simple-client.o
	$(CC) $(LDFLAGS) common.o simple-client.o -o simple-client

server: common.o server.o
	$(CC) $(LDFLAGS) common.o server.o -o server

common.o: $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) $(SRC_DIR)/common/common.c

cdf.o: $(SRC_DIR)/common/cdf.c
	$(CC) $(CFLAGS) $(SRC_DIR)/common/cdf.c

conn.o: $(SRC_DIR)/common/conn.c
	$(CC) $(CFLAGS) $(SRC_DIR)/common/conn.c

server.o: $(SRC_DIR)/server/server.c
	$(CC) $(CFLAGS) $(SRC_DIR)/server/server.c

simple-client.o: $(SRC_DIR)/client/simple-client.c
	$(CC) $(CFLAGS) $(SRC_DIR)/client/simple-client.c

poission-client.o: $(SRC_DIR)/client/poission-client.c
	$(CC) $(CFLAGS) $(SRC_DIR)/client/poission-client.c

clean:
	rm -rf $(BIN_DIR)
