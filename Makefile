CC = gcc
CFLAGS = -Wall -Werror

main: Server Client

Server:
	$(CC) server/server.c $(CFLAGS) -lpthread -o server/server

Client:
	$(CC) client/test.c $(CFLAGS) -lpthread -o client/test

clean:
	rm server/server
	rm client/test
