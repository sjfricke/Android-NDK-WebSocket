#include <iostream>
#include "WebSocket.h"

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

int main() {

  WebSocket client_socket;
  
  cout << "START" << endl;

  client_socket.connectSocket("192.168.1.105", 5000);

  client_socket.setEvent(1, on_new_type);
  client_socket.setEvent(2, on_new_color);
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
        client_socket.setEvent(3, on_new_test);
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
