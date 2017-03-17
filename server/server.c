// gcc -Wall -Werror server.c -O2 -lpthread -o server

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_CLIENTS 16
#define DEFAULT_PORT 5000
#define MESSAGE_BUFFER 1024

static unsigned int client_count = 0; // Global client count
static int uid = 10;

// Client structure
typedef struct {
  struct sockaddr_in addr; // Client remote address
  int connect_fd;	   // Connection file descriptor
  int uid;	           // Client unique identifier
  char name[32];	   // Client name
} client_t;

client_t *clients[MAX_CLIENTS];

// wrapper for throwing error
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Add client to queue
void queue_add(client_t *current_client){
  int i;
  for ( i=0; i < MAX_CLIENTS; i++ ) {
    if ( !clients[i] ) {
      clients[i] = current_client;
      return;
    }
  }
}

// Delete client from queue
void queue_delete(int uid){
  int i;
  for ( i=0; i < MAX_CLIENTS; i++ ) {
    if ( clients[i] ) {
      if ( clients[i]->uid == uid ) {
	clients[i] = NULL;
	return;
      }
    }
  }
}

/* Send message to all clients but the sender */
void send_message(char *s, int uid){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid != uid){
				write(clients[i]->connect_fd, s, strlen(s));
			}
		}
	}
}

/* Send message to all clients */
void send_message_all(char *s){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			write(clients[i]->connect_fd, s, strlen(s));
		}
	}
}

/* Send message to sender */
void send_message_self(const char *s, int connect_fd){
	write(connect_fd, s, strlen(s));
}

/* Send message to client */
void send_message_client(char *s, int uid){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			if(clients[i]->uid == uid){
				write(clients[i]->connect_fd, s, strlen(s));
			}
		}
	}
}

/* Send list of active clients */
void send_active_clients(int connect_fd){
	int i;
	char s[64];
	for(i=0;i<MAX_CLIENTS;i++){
		if(clients[i]){
			sprintf(s, "<<CLIENT %d | %s\r\n", clients[i]->uid, clients[i]->name);
			send_message_self(s, connect_fd);
		}
	}
}

// Strips all new line and carriage returns
void strip_newline(char* current_char){
  while ( *current_char != '\0' ) {
    if ( *current_char == '\r' || *current_char == '\n' ) {
      *current_char = '\0'; // TODO: best char to set too? More edge cases?
    }
    current_char++;
  }
}

// Print ip address
void print_client_addr(struct sockaddr_in addr){
  printf("%d.%d.%d.%d",
	  addr.sin_addr.s_addr & 0xFF,
	 (addr.sin_addr.s_addr & 0xFF00)>>8,
	 (addr.sin_addr.s_addr & 0xFF0000)>>16,
	 (addr.sin_addr.s_addr & 0xFF000000)>>24);
}

// Handle all communication with the client
// Called from each new thread
// passes in a client_t
void *handle_client(void *arg){

  char buffer_out[MESSAGE_BUFFER];
  char buffer_in[MESSAGE_BUFFER];
  int return_size;

  client_count++; // global count
  client_t *cli = (client_t *)arg; // safely type cast

  print_client_addr(cli->addr);
  printf(" has connected with uid: %d\n", cli->uid);

  // TODO: send welcome message?
  sprintf(buffer_out, "%s joined!\r\n", cli->name);
  send_message_all(buffer_out);

  // Receive input from client
  while((return_size = read(cli->connect_fd, buffer_in, sizeof(buffer_in)-1)) > 0){
    buffer_in[return_size] = '\0';
    buffer_out[0] = '\0';
    strip_newline(buffer_in);

    // Ignore empty buffer
    if( !strlen(buffer_in) ) { continue; }
	
    sprintf(buffer_out, "[%s] %s\r\n", cli->name, buffer_in);
    send_message_self(buffer_out, cli->connfd);

  }

  // Close connection
  close(cli->connect_fd);
  sprintf(buffer_out, "%s left!\r\n", cli->name);
  send_message_all(buffer_out);

  // Delete client from queue and yeild thread
  queue_delete(cli->uid);
  print_client_addr(cli->addr);
  printf(" left and freed uid: %d\n", cli->uid);
  free(cli);
  client_count--;
  pthread_detach(pthread_self());
	
  return NULL;
}

int main(int argc, char *argv[]){
  int socket_fd = 0; // socket file descriptor
  int connect_fd = 0; // used to set new client thread connect_fd
  int status; // used to check status of c functions return values
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  pthread_t tid;
  int port;
  socklen_t client_size = sizeof(client_addr);
  
  ///////////
  // SETUP //
  ///////////

  // see if passed port in argument
  if (argc == 2) {
    port = atoi(argv[1]);
  } else {
    port = DEFAULT_PORT;
  }
  
  // Socket settings
  server_addr.sin_family = AF_INET; // sets to use IP
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // sets our local IP address
  server_addr.sin_port = htons(port); // sets the server port number 

  // Create Socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); // creates IP based TCP socket
  if (socket_fd < 0) { error("ERROR: Opening socket\n"); }
  else { printf("TCP Socket Created!\n"); }
  
  // Bind Socket
  status = bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

  // checks for TIME_WAIT socket
  // when daemon is closed there is a delay to make sure all TCP data is propagated
  if (status < 0) {
    printf("ERROR binding socket: %d , possible TIME_WAIT\n", status);
    printf("USE: netstat -ant|grep %d to find out\n", port);
    exit(1);
  } else {
    printf("Socket Binded!\n");
  }
  
  // Listen to Socket
  status = listen(socket_fd, 10);

  if (status < 0){ error("Socket listening failed\n"); }
  else { printf("Server started on port %d!\n", port); }

  ///////////////////
  // Accept Daemon //
  ///////////////////

  while(1){

    // blocks until client sends connection
    connect_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_size);

    // Check if max clients is reached
    if ( (client_count+1) == MAX_CLIENTS ){
      printf("ERROR: \n");
      print_client_addr(client_addr);
      printf(" not connected due to Max Clients Reached\n");
      close(connect_fd);
      continue;
    }

    // Client settings
    client_t *cli = (client_t *)malloc(sizeof(client_t));
    cli->addr = client_addr;
    cli->connect_fd = connect_fd;
    cli->uid = uid++;
    sprintf(cli->name, "%d", cli->uid); // TODO: do we need a name and uid?

    // Add client to the queue and fork thread
    queue_add(cli);
    pthread_create(&tid, NULL, &handle_client, (void*)cli);

    // Reduce CPU usage
    sleep(1);
  }
}
