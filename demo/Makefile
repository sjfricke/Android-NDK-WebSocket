CC = gcc
CFLAGS = -Wall -Werror

main: Demo

Demo:
	g++ demo.cpp ../client/WebSocket.cpp -Wall -Werror -std=c++11 -pthread -o demo

clean:
	rm demo/demo
