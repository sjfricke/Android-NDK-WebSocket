CC = gcc
CFLAGS = -Wall -Werror

main: Server

Server:
	$(CC) server/server.c $(CFLAGS) -lpthread -o server/server

clean:
	rm server/server
