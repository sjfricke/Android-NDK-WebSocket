#include "WebSocket.h"

#include <stdio.h> //TODO:  remove with debugging class
#include <iostream>

WebSocket::WebSocket() { }

WebSocket::~WebSocket() {
  //server_addr = NULL; // prevents old info from being saved
  //close(socket_fd);
  //pthread_exit(NULL); //TODO: Only the class thread
}

// used to setup and connect to server
// returns 0 on success
int WebSocket::connectSocket( std::string ip, int port ) {

  int status; // used to check status returns
  
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) { printf("socket() ERROR"); return 1; }

  server_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // sets IP of server
  server_addr.sin_family = AF_INET; // uses internet address domain
  server_addr.sin_port = htons(port); // sets PORT on server

  status = connect(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr));
  if (status < 0) { printf("connect() ERROR"); return 2; }

  status = pthread_create(&receive_thread, NULL, messageThread, (void*)(intptr_t) socket_fd);
  if (status) { printf("pthreadCreate() ERROR"); return 3; }
  else if (status == -1) { printf("socket closed"); return 4; }

  printf("end connect, %d\n", status);
  return 0;
}

// sends message to all other users online
// returns 0 on success
int WebSocket::broadcast( int key, int option, std::string message ) {
  int status;

  sprintf(msg_buffer_out,"%d\n%d\n%s", key, option, message.c_str());
  
  status = sendto(socket_fd, msg_buffer_out, MAX_MESSAGE_BUFFER, 0, (struct sockaddr*) &server_addr, sizeof(server_addr));
  if (status < 0) {
    printf("sendto() ERROR"); return 1;
  } else {
    return 0;
  }
}

void* WebSocket::messageThread( void* socket ) {

  printf("message thread started");

  int socket_fd;
  int status;
  char msg_buffer_in[MAX_MESSAGE_BUFFER];
  std::string msg_body;
  
  socket_fd = (intptr_t) socket;

  for(;;) {
    memset(&msg_buffer_in, 0, MAX_MESSAGE_BUFFER);
    status = recvfrom(socket_fd , msg_buffer_in, MAX_MESSAGE_BUFFER, 0, NULL, NULL);
    // 0 is used for when server closes... server should always return at least 1 byte
    if (status <= 0) {
      printf("recvfrom() ERROR");
      close(socket_fd);
      return (void*) -1; // returns error
    }
    else {
      msg_body = msg_buffer_in;
      std::cout << msg_body << std::endl;
    }
  }
}
