#include <iostream>
#include "../client/WebSocket.h"

using namespace std;

void on_new_color(char* body) {
  printf("New color: %s", body);
}

void on_new_type(char* body) {
  printf("New type: %s", body);
}

void on_new_test(char* body) {
  printf("New test: %s", body);
}

void onJoin(int uid){ printf("Join: %d\n", uid); }
void onLeave(int uid){ printf("Leave: %d\n", uid); }

int main(int argc, char* argv[]) {

  if (argc < 3) {
    printf("\n/demo <ip> <port>\n\n");
    exit(1);
  }
  
  WebSocket client_socket;
  
  cout << "START" << endl;

  client_socket.connectSocket(argv[1], atoi(argv[2]));

  client_socket.setEvent(1, on_new_type);
  client_socket.setEvent(2, on_new_color);
  client_socket.setEvent(3, on_new_test);
  client_socket.setJoinEvent(onJoin);
  client_socket.setLeaveEvent(onLeave);
  
  string s;
  string p;
  int n;

  while (true) {
    cout << "key: ";
    cin >> s;
    cin >> p;
    n = atoi(s.c_str());

    if (n == 5){
	continue;
    } else if (n != -1) {
      client_socket.broadcast(n, 0, p);
      continue;
    }
    else {

      continue;
    }
  } 
  
  
  
}
