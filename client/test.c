#include <stdio.h>  
#include <stdlib.h>  
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
  
#define DEFAULT_PORT 5000
#define MESSAGE_BUFFER 1024

// wrapper for throwing error
void error(char* msg) {
  perror(msg);
  exit(1);
}

void * receiveMessage(void * socket) {
  int socket_fd;
  int status;
  char buffer[MESSAGE_BUFFER]; 

  socket_fd = (intptr_t) socket;

  memset(buffer, 0, MESSAGE_BUFFER);  
  for (;;) {
    status = recvfrom(socket_fd, buffer, MESSAGE_BUFFER, 0, NULL, NULL);  
    if (status < 0) {  
      printf("Error receiving data!\n");    
    } else {
      printf("server: ");
      fputs(buffer, stdout);
      printf("\n");
    }  
  }
}

int main(int argc, char**argv) {  
  struct sockaddr_in server_addr;
  int socket_fd;
  int port;
  int status;  
  char buffer[MESSAGE_BUFFER]; 
  char * server_IP;
  pthread_t rThread;

  if (argc < 2) { error("usage: client < ip address >\n"); }

  //TODO: add IP check
  server_IP = argv[1];
  
  // see if passed port in argument
  if (argc == 3) {
    port = atoi(argv[2]);
  } else {
    port = DEFAULT_PORT;
  }
 
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);  
  if (socket_fd < 0) { error("need IP and Port"); }  
  else  printf("Socket created...\n");   

  memset(&server_addr, 0, sizeof(server_addr));  
  server_addr.sin_family = AF_INET;  
  server_addr.sin_addr.s_addr = inet_addr(server_IP);
  server_addr.sin_port = port;

  status = connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));  
  if (status < 0) { error("Error connecting to the server!\n"); }  
  else { printf("Connected to the server...\n"); }

  memset(buffer, 0, MESSAGE_BUFFER);
  printf("Enter your messages one by one and press return key!\n");

  //creating a new thread for receiving messages from the server
  status = pthread_create(&rThread, NULL, receiveMessage, (void *)(intptr_t) socket_fd);
  if (status) {
    printf("ERROR: Return Code from pthread_create() is %d\n", status);
    exit(1);
  }

  // loops for message input
  while (fgets(buffer, MESSAGE_BUFFER, stdin) != NULL) {
    status = sendto(socket_fd, buffer, MESSAGE_BUFFER, 0, (struct sockaddr *) &server_addr, sizeof(server_addr));  
    if (status < 0) {  
      printf("Error sending data!\n\t-%s", buffer);  
    }
  }

  close(socket_fd);
  pthread_exit(NULL);
 
  return 0;    
}  
