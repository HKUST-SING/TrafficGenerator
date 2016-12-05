CC = gcc
CFLAGS = -c -Wall -pthread -lm -lrt
LDFLAGS = -pthread -lm -lrt
TARGETS = client incast-client simple-client server
CLIENT_OBJS = common.o cdf.o conn.o client.o
INCAST_CLIENT_OBJS = common.o cdf.o conn.o incast-client.o
SIMPLE_CLIENT_OBJS = common.o simple-client.o
SERVER_OBJS = common.o server.o
BIN_DIR = bin
RESULT_DIR = result
CLIENT_DIR = src/client
COMMON_DIR = src/common
SERVER_DIR = src/server
SCRIPT_DIR = src/script

all: $(TARGETS) move

move:
	mkdir -p $(RESULT_DIR)
	mkdir -p $(BIN_DIR)
	mv *.o $(TARGETS) $(BIN_DIR)
	cp $(SCRIPT_DIR)/* $(BIN_DIR)

client: $(CLIENT_OBJS)
	$(CC) $(CLIENT_OBJS) -o client $(LDFLAGS)

incast-client: $(INCAST_CLIENT_OBJS)
	$(CC) $(INCAST_CLIENT_OBJS) -o incast-client $(LDFLAGS)

simple-client: $(SIMPLE_CLIENT_OBJS)
	$(CC) $(SIMPLE_CLIENT_OBJS) -o simple-client $(LDFLAGS)

server: $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o server $(LDFLAGS)

%.o: $(CLIENT_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(SERVER_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(COMMON_DIR)/%.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BIN_DIR)/*
