#include "WebSocket.h"

#include <stdio.h> //TODO:  remove with debugging class
#include <iostream>

WebSocket::WebSocket() {
  max_message_keys = 16;

  // set NULL to know later if they have been set or not
  on_join = NULL;
  on_leave = NULL;
}

WebSocket::~WebSocket() {
  //server_addr = NULL; // prevents old info from being saved
  //close(socket_fd);
}

// used to setup and connect to server
// returns 0 on success
int WebSocket::connectSocket( std::string ip, int port ) {

  int status; // used to check status returns
  
  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) { printf("socket() ERROR\n"); return 1; }

  server_addr.sin_addr.s_addr = inet_addr(ip.c_str()); // sets IP of server
  server_addr.sin_family = AF_INET; // uses internet address domain
  server_addr.sin_port = htons(port); // sets PORT on server

  status = connect(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr));
  if (status < 0) { printf("connect() ERROR\n"); return 2; }

  rec_thread = std::thread(&WebSocket::messageThread, this);
  
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
    printf("sendto() ERROR\n"); return 1;
  } else {
    return 0;
  }
}

void WebSocket::messageThread( ) {

  printf("message thread started\n");

  //  int socket_fd;
  int status;
  char message_buffer_in[MAX_MESSAGE_BUFFER];

  // for the pasring
  char *message_body;
  char *message_key_token;
  int message_key;
  char *end_ptr;

  for(;;) {
    memset(&message_buffer_in, 0, MAX_MESSAGE_BUFFER);
    status = recvfrom(socket_fd, message_buffer_in, MAX_MESSAGE_BUFFER, 0, NULL, NULL);
    // 0 is used for when server closes... server should always return at least 1 byte
    if (status <= 0) {
      printf("recvfrom() ERROR\n");
      close(socket_fd);
      return;
    } else {

      // Parse message and validate
      message_key_token = strtok(message_buffer_in,"\n");
      message_body = strtok(NULL, "\0");

      message_key = strtol(message_key_token, &end_ptr, 10);
      if (errno == ERANGE || message_key_token == end_ptr || message_key < -2 || message_key > max_message_keys) {
	// if key_token == end_ptr means its empty
	// message_key can be -1/-2 for join/leave ack
	printf("TODO: Invalid receive message\n");
	continue; 
      }

      // -1 key reserved for join
      // -2 key reserved for leave
      if (message_key == -1) {

	if (*on_join == NULL)  continue; } // on_join not set
	
	int uid = strtol(message_body, &end_ptr, 10);
	if (errno == ERANGE || message_key_token == end_ptr || uid < 0) {
	  // should not have negative uid
	  printf("TODO: Invalid join message\n");
	  continue; 
	} else {
	  // valid uid
	  (*on_join)(uid);
	}
	
	
      } else if (message_key == -2) {

      	if (*on_leave == NULL)  continue; } // on_leave not set

	int uid = strtol(message_body, &end_ptr, 10);
	if (errno == ERANGE || message_key_token == end_ptr || uid < 0) {
	  // should not have negative uid
	  printf("TODO: Invalid join message\n");
	  continue; 
	} else {
	  // valid uid
	  (*on_leave)(uid);
	}
	
      } else if (message_key >= 0 && message_key < max_message_keys ) { 
	  response_map[message_key](message_body);
      }
      
      //      printf("TEST: key: %d\nbody: %s", message_key, message_body);
      
    }
  } // infinite for loop
}

int WebSocket::setEvent(int key, void (*callbackFunction)(char*)) {
  response_map[key] = callbackFunction;
  return 0;
  // error check TODO
}

int WebSocket::setJoinEvent(void (*callbackFunction)(int)) {
  on_join = callbackFunction;
  return 0;
  // TODO error check
}

int WebSocket::setLeaveEvent(void (*callbackFunction)(int)) {
  on_leave = callbackFunction;
  return 0;
  // TODO error check
}
