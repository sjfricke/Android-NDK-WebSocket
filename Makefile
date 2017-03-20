CC = gcc
CFLAGS = -Wall -Werror

main: Server Client

Server:
	$(CC) server/server.c $(CFLAGS) -lpthread -o server/server

Client:
	$(CC) client/test.c $(CFLAGS) -lpthread -o client/test
	g++ client/main.cpp client/WebSocket.cpp -Wall -Werror -std=c++11 -pthread -o test2

clean:
	rm server/server
	rm client/test
	rm test2
