CC		= gcc
CFLAGS	= -Wall -Wextra -Werror -std=c11 -O2 -Iinclude
LDFLAGS = -lssl -lcrypto

SERVER = src/ftp_server
CLIENT = src/ftp_client

COMMON_SRCS = src/ftp_net.c src/ftp_protocol.c src/ftp_crypto.c src/ftp_crc.c
SERVER_SRCS = src/ftp_server.c src/ftp_auth.c src/ftp_transfer.c
CLIENT_SRCS = src/ftp_client.c

SERVER_OBJS = $(COMMON_SRCS:.c=.o) $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(COMMON_SRCS:.c=.o) $(CLIENT_SRCS:.c=.o)

.PHONY: all clean run-server run-client

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run-server: $(SERVER)
	./$(SERVER)

run-client: $(CLIENT)
	./$(CLIENT)

clean:
	rm -f src/*.o $(SERVER) $(CLIENT)