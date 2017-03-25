//gcc -Wall -Werror server.c -O2 -lpthread -o server
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
#include <argp.h> // used for command line arguments
#include <stdint.h> // platform agnostic variable type

//////////////////
// GLOBAL SETUP //
//////////////////

#define MAX_CLIENTS 16
#define DEFAULT_PORT 5000
#define MAX_MESSAGE_BUFFER 1024
#define DEFAULT_MAX_KEYS 16

static unsigned int client_count = 0; // Global client count
static int uid = 10;

char **message_map; // used to map messages by int in a 2D array 
pthread_mutex_t message_map_lock;

// Flag global variables
struct arguments {
  uint8_t VERBOSE;
  uint16_t PORT; // Default port
  uint16_t MAX_KEYS; // Defauly max key
};

struct arguments arguments;
// Client structure
typedef struct {
  struct sockaddr_in addr; // Client remote address
  int connect_fd;	   // Connection file descriptor
  int uid;	           // Client unique identifier
  char name[32];	   // Client name
} client_t;

// Stack declaration, TODO: move to heap
client_t *clients[MAX_CLIENTS];

// wrapper for throwing error
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Program documentation
static char doc[] = "Android-NDK-WebSocket Server -- server part of the WebSocket";

//arguments we accept.
static char args_doc[] = "TODO";

// The options we understand
static struct argp_option options[] = {
  {"verbose",  'v', 0,      0,   "Produce verbose output" },
  {"port",     'p', "PORT", 0,   "Port to use [Default: 5000]" },
  {"keys",     'k', "MAX_KEYS",0,"Max number of protocol message keys to map" },
  { 0 }
};

// Parse a single option
static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  // Get the input argument from argp_parse, which we know is a pointer to our arguments structure
  struct arguments *arguments = state->input;

  switch (key)
    {
    case 'v':
      arguments->VERBOSE = 1;
      break;
    case 'p':
      arguments->PORT = atoi(arg);
      if (arguments->PORT <= 1024) { error("Invalid Port, needs to be 1024 - 65535"); }
      break;
    case 'k':
      arguments->MAX_KEYS = atoi(arg);
      break;
 /*
    case ARGP_KEY_ARG:
      if (state->arg_num >= 2)
	// Too many arguments
	argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 2)
	// Not enough arguments
	argp_usage (state);
      break;
 */
      
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

// Our argp parse
static struct argp argp = { options, parse_opt, args_doc, doc };


/////////////////////////////
// Client Helper Functions //
/////////////////////////////

/*
 * Add client to queue
 * @param client struct to add
 */
void queue_add(client_t *current_client) {
  int i;
  for ( i=0; i < MAX_CLIENTS; i++ ) {
    if ( !clients[i] ) {
      clients[i] = current_client;
      return;
    }
  }
}

/*
 * Delete client from queue
 * @param client uid to delete
 */
void queue_delete(int uid) {
  int i;
  for (i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i]) {
      if (clients[i]->uid == uid) {
	clients[i] = NULL;
	return;
      }
    }
  }
}

/*
 * Send message to all clients
 * @param body of message to send
 * @param id of sender
 */
void send_message_all(char *body, int uid) {
  int i;
  for(i = 0; i < MAX_CLIENTS; i++) {
    // prevent from sending back to sender
    if(clients[i] && clients[i]->uid != uid) {
      write(clients[i]->connect_fd, body, strlen(body));
    }
  }
}

/*
 * Send message to back to sender
 * @param body of message to send
 * @param file descriptor of sender to use to write back
 */
void send_message_self(const char *body, int connect_fd) {
  write(connect_fd, body, strlen(body));
}

/*
 * Send message to client
 * @param body of message to send
 * @param id of sender
 */
void send_message_client(char *body, int uid) {
  int i;
  for(i = 0; i < MAX_CLIENTS; i++) {
    if(clients[i] && clients[i]->uid == uid) {
	write(clients[i]->connect_fd, body, strlen(body));
    }
  }
}

/*
 * Send list of active clients
 * @param client to send info to
 */
void send_active_clients(int connect_fd) {
  int i;
  char body[64]; // max size need for now
  for(i = 0; i < MAX_CLIENTS; i++) {
    if(clients[i]) {
      sprintf(body, "Client_uid: %d\tClient_name: %s\n",
	      clients[i]->uid, clients[i]->name);
      send_message_self(body, connect_fd);
    }
  }
}

/*
 * Print ip address
 * @param socket struct info
 */
void print_client_addr(struct sockaddr_in addr) {
  printf("%d.%d.%d.%d",
          addr.sin_addr.s_addr & 0xFF,
	 (addr.sin_addr.s_addr & 0xFF00)>>8,
	 (addr.sin_addr.s_addr & 0xFF0000)>>16,
	 (addr.sin_addr.s_addr & 0xFF000000)>>24);
}

/*
 * Handle all communication with the client
 * Called from each new thread
 * passes in a client_t
 */
void *handle_client(void *arg) {

  char buffer_out[MAX_MESSAGE_BUFFER];
  char buffer_in[MAX_MESSAGE_BUFFER];
  char *message_key_token;
  int message_key;
  char *message_options;
  char *message_body;
  int return_size;
  char *end_ptr;
  
  client_count++; // global count
  client_t *cli = (client_t *)arg; // safely type cast

  print_client_addr(cli->addr);
  printf(" has connected with uid: %d\n", cli->uid);

  // send join with uid
  sprintf(buffer_out, "-1\n%d\n", cli->uid);
  send_message_all(buffer_out, cli->uid);

  // Receive input from client
  while((return_size = read(cli->connect_fd, buffer_in, sizeof(buffer_in)-1)) > 0) {
    buffer_in[return_size] = '\0'; // caps off anything larger then max buffer size
    buffer_out[0] = '\0';

    // Ignore empty buffer
    if( strlen(buffer_in) <= 1 ) { continue; }
    if (arguments.VERBOSE) { printf("\nbuffer_in length: %d\n",(int)strlen(buffer_in)); }
    
    message_key_token = strtok(buffer_in, "\n"); // gets int key value
    message_options = strtok(NULL, "\n"); // gets options
    message_body = strtok (NULL, "\0"); // gets actual message value
    
    if (arguments.VERBOSE) { printf("key_token: %s\n", message_key_token); }
    if (arguments.VERBOSE) { printf("option: %s\n", message_options); }
    if (arguments.VERBOSE) { printf("body: %s\n", message_body); }

    // TODO: can strtol handle if its NULL already cause this check is redundent
    //if (message_key_token == NULL) {  }
    
    message_key = strtol(message_key_token, &end_ptr, 10);
    if (arguments.VERBOSE) { printf("message_key: %d\n", message_key);}
    if (arguments.VERBOSE) { printf("MAX_KEYS: %d\n", arguments.MAX_KEYS);}
    
    if (errno == ERANGE || message_key_token == end_ptr) {
      sprintf(buffer_out, "Invalid message map key as int\n");
      send_message_self(buffer_out, cli->connect_fd);
      if (arguments.VERBOSE) { printf("ERROR: message key not an int\n"); }
      continue;
      
    } else if (message_key < 0 || message_key > arguments.MAX_KEYS) {
      sprintf(buffer_out, "Invalid message map key range, should be between 0 and %d\n", arguments.MAX_KEYS);
      send_message_self(buffer_out, cli->connect_fd);
      if (arguments.VERBOSE) { printf("ERROR: out of message key range\n"); }
      continue; 
    }

    if (message_options != NULL) { } //TODO

    if (message_body == NULL) {
      sprintf(buffer_out, "Invalid message body\n");
      send_message_self(buffer_out, cli->connect_fd);
      if (arguments.VERBOSE) { printf("ERROR: message body == null\n"); }
      continue; 
    }
    
    pthread_mutex_lock(&message_map_lock);
    *message_map[message_key] = *message_body;
    pthread_mutex_unlock(&message_map_lock);
    
    sprintf(buffer_out, "%d\n%s\n", message_key, message_body); // used body instead of map too prevent another mutex lock
    //send_message_self(buffer_out, cli->connect_fd);
    send_message_all(buffer_out, cli->uid);

  }

  // Close connection
  // send leave event to client
  close(cli->connect_fd);
  sprintf(buffer_out, "-2\n%d\n", cli->uid);
  send_message_all(buffer_out, cli->uid);

  // Delete client from queue and yeild thread
  queue_delete(cli->uid);
  print_client_addr(cli->addr);
  printf(" left and freed uid: %d\n", cli->uid);
  free(cli);
  client_count--;
  pthread_detach(pthread_self());
	
  return NULL;
}

/* 
 * Main function
 */
int main(int argc, char *argv[]) {
  int socket_fd = 0; // socket file descriptor
  int connect_fd = 0; // used to set new client thread connect_fd
  int status; // used to check status of c functions return values
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  pthread_t tid;
  socklen_t client_size = sizeof(client_addr);
  int i; // for loop var
  
  ///////////
  // SETUP //
  ///////////
  
  // Default values
  arguments.VERBOSE = 0;
  arguments.PORT = DEFAULT_PORT;
  arguments.MAX_KEYS = DEFAULT_MAX_KEYS;
  
  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  // Allocate the message map
  message_map = malloc( arguments.MAX_KEYS * sizeof(char*) );
  if (message_map == NULL) { error("ERROR: Failed to allocate the message_map\n"); }
  for (i = 0 ; i < arguments.MAX_KEYS ; i++) {
    message_map[i] = malloc( MAX_MESSAGE_BUFFER * sizeof(char) );
    if (message_map[i] == NULL) { error("ERROR: Failed to allocate the message_map\n"); }
    *message_map[i] = (char) 0; // so we know if its the first time someone is writing to value
  }
  
  // Socket settings
  server_addr.sin_family = AF_INET; // sets to use IP
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // sets our local IP address
  server_addr.sin_port = htons(arguments.PORT); // sets the server port number 

  // Create Socket
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); // creates IP based TCP socket
  if (socket_fd < 0) { error("ERROR: Opening socket\n"); }
  else if (arguments.VERBOSE) { printf("TCP Socket Created!\n"); }
  
  // Bind Socket
  status = bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

  // checks for TIME_WAIT socket
  // when daemon is closed there is a delay to make sure all TCP data is propagated
  if (status < 0) {
    printf("ERROR binding socket: %d , possible TIME_WAIT\n", status);
    printf("USE: netstat -ant|grep %d to find out\n", arguments.PORT);
    exit(1);
  } else if (arguments.VERBOSE) {
    printf("Socket Binded!\n");
  }
  
  // Listen to Socket
  status = listen(socket_fd, 10);

  if (status < 0) { error("ERROR: Socket listening failed\n"); }
  else { printf("Server started on port %d!\n", arguments.PORT); }

  ///////////////////
  // Accept Daemon //
  ///////////////////

  while(1) {

    // blocks until client sends connection
    connect_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &client_size);

    // Check if max clients is reached
    if ( (client_count+1) == MAX_CLIENTS ) {
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
