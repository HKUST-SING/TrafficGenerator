CC = gcc
CFLAGS = -c -Wall -pthread -lm
LDFLAGS = -pthread -lm
TARGETS = client simple-client server
CLIENT_OBJS = common.o cdf.o conn.o client.o
SIMPLE_CLIENT_OBJS = common.o simple-client.o
SERVER_OBJS = common.o server.o
BIN_DIR = bin
CLIENT_DIR = src/client
COMMON_DIR = src/common
SERVER_DIR = src/server

all: $(TARGETS) move

move:
	mkdir -p $(BIN_DIR)
	mv *.o $(TARGETS) $(BIN_DIR)

client: $(CLIENT_OBJS)
	$(CC) $(LDFLAGS) $(CLIENT_OBJS) -o client

simple-client: $(SIMPLE_CLIENT_OBJS)
	$(CC) $(LDFLAGS) $(SIMPLE_CLIENT_OBJS) -o simple-client

server: $(SERVER_OBJS)
	$(CC) $(LDFLAGS) $(SERVER_OBJS) -o server

%.o: $(CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(SERVER_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf *.o $(TARGETS) $(BIN_DIR)
