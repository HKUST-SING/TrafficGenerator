CC = gcc
CFLAGS = -pthread
TARGETS = server client
BIN_DIR = bin
SRC_DIR = src

all: directory $(TARGETS)

directory:
	mkdir -p $(BIN_DIR)

client: $(SRC_DIR)/client/client.c $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/client.o $(SRC_DIR)/client/client.c $(SRC_DIR)/common/common.c

server: $(SRC_DIR)/server/server.c $(SRC_DIR)/common/common.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/server.o $(SRC_DIR)/server/server.c $(SRC_DIR)/common/common.c

clean:
	rm -r $(BIN_DIR)
