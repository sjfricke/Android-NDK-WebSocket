#ifndef ANDROID_NDK_WEBSOCKET_H
#define ANDROID_NDK_WEBSOCKET_H

#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE_BUFFER 1024

class WebSocket {

 public:
    
    // Constructor and destructor
    WebSocket();
    ~WebSocket();

    // used to setup and connect to server
    // returns 0 on success
    int connectSocket(std::string ip, int port);

    // sends message to all other users online
    // returns 0 on success
    int broadcast(int key, int option, std::string message);

 private:

    struct sockaddr_in server_addr; // socket struct object
    int socket_fd;                  // holds socket file discriptor
    std::string msg_buffer_out;     // outgoing messages
    pthread_t receive_thread;       // thread to block for messages

    // runs a seperate thread to wait for incoming request
    static void* messageThread(void* socket);
    
};

#endif // ANDROID_NDK_WEBSOCKET_H
