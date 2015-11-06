CC = gcc
CFLAGS = -Wall -pthread
TARGETS = server client simple-client
BIN_DIR = bin
SRC_DIR = src

all: directory $(TARGETS)

directory:
	mkdir -p $(BIN_DIR)

server: $(SRC_DIR)/server/server.c $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) $(SRC_DIR)/server/server.c $(SRC_DIR)/common/common.c -o $(BIN_DIR)/server

client: $(SRC_DIR)/client/client.c $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) $(SRC_DIR)/client/client.c $(SRC_DIR)/common/common.c -o $(BIN_DIR)/client

simple-client: $(SRC_DIR)/client/simple-client.c $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) $(SRC_DIR)/client/simple-client.c $(SRC_DIR)/common/common.c -o $(BIN_DIR)/simple-client

clean:
	rm -rf $(BIN_DIR)
