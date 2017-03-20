#ifndef ANDROID_NDK_WEBSOCKET_H
#define ANDROID_NDK_WEBSOCKET_H

#include <cstring>
#include <string>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <thread> // std threads instead of pthreads due to c++ member function issues

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
    char msg_buffer_out[MAX_MESSAGE_BUFFER];     // outgoing messages
    int max_message_keys;
    
    std::thread rec_thread; 
    // runs a seperate thread to wait for incoming request
    void messageThread();
    
};

#endif // ANDROID_NDK_WEBSOCKET_H
