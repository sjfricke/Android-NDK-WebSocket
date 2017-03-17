CC = gcc
CFLAGS = -Wall -Werror

main: Server

Server:
	$(CC) server/server.c $(CFLAGS) -o server/server

clean:
	rm server/server
