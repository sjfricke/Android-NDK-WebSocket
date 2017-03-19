#include <iostream>
#include "WebSocket.h"

using namespace std;

int main() {

  WebSocket client_socket;
  
  cout << "START" << endl;

  client_socket.connectSocket("192.168.1.105", 5000);

  string s;
  int n;

  while (true) {
    cin >> s;
    n = atoi(s.c_str());

    if (n == 1) {
      client_socket.broadcast(4, 0, "test message from main");
      continue;
    }
    else {

      continue;
    }
  } 
  
  
  
}
