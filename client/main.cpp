#include <iostream>
#include "WebSocket.h"

using namespace std;

void on_new_color() {}

int main() {

  WebSocket client_socket;
  
  cout << "START" << endl;

  client_socket.connectSocket("192.168.1.105", 5000);

  string s;
  string p;
  int n;

  while (true) {
    cin >> s;
    cin >> p;
    n = atoi(s.c_str());

    if (n != -1) {
      client_socket.broadcast(n, 0, p);
      continue;
    }
    else {

      continue;
    }
  } 
  
  
  
}
