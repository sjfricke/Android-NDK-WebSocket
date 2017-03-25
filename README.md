# Android-NDK-WebSocket
For a low level WebSocket for your NDK application

This is used to allow for easy addition of WebSockets for any NDK application. So now if you have multiple devices and want a small, easy to use way of adding real time communication you can have it setup in no time.

## Server

The server side of this made to be a single file that you copy to a Linux server, compile, and run with ease.

1. Clone this repo
   * `git clone https://github.com/sjfricke/Android-NDK-WebSocket.git`
2. Build the server
   * `make`
	 * **NOTE:** Make sure you did a `cd` into the folder you just cloned
3. Run the server
   * `./server/server`
4. (Optional)
   * There are different options to let you customize it to your likin
   * See `./server/server --help` as well
	
  ```
  Server Options:

	-p, --port <port>    Port to host server [Default: 5000]
	-v, --verbose        Enable verbose output on stdout
	-?, --help           Output usage information
  ```

## The WebSocket Protocol
* All data is sent in three parts in a key-value fasion
```
< int value of key from (0 to MAX_KEYS) >
< options >
< value >
```
* This allows you to set what each key value is in charge of sending

Here is an example of sending over the color for what has been set as data set 4 with no options
```
4
0
red
```
also sent as `4\n0\nred`

This means that all clients are aware that key 4 is used for sending color data. The [Client Section](#client) explains more about how to set it up.

* **NOTE:** Both `-1` and `-2` are reserved for join and leave event respecfully
  * `-1` gets sent on a join of a client
  * `-2` gets sent on a leave of a client

## Client

The client side is a single .cpp file and header that can be added to your CMake or ndk-build in one line.

* Made to allow you to set your WebSocket events and callback functions
* Super low overhead to your application
* No long, grueling, hard process of adding it to an exisiting applicaiton.

### How to add to my CMake build project

### How to add to my ndk-build project

### How to use
* Add both [WebSocket.cpp](./client/WebSocket.cpp) and [WebSocket.h](./client/WebSocket.h) to your current set of C++ files in your project
* Next thing is add `#include "WebSocket.h"`
  * If it is in a different folder use `#include "path/to/WebSocket.h"` instead
* Create an instance of the WebSocket class either
  * By declaring it: `WebSocket mySocket;`
  * By dynamically calling it `WebSocket* mySocket = new WebSocket();`
* Setup the connection with the server
  * `mySocket.connectSocket( "IP Address", PORT_NUMBER, MAX_KEYS );`
  * An example with the server on a `192.168.1.100` and port `5000` allowing only 16 event keys
	* `mySocket.connectSocket( "192.168.1.100", 5000, 16 );`

#### Broadcast message to others
* The broadcast takes three parameters
  * `int broadcast(int key, int option, std::string message);`
  * `int key` - The key of the WebSocket message (explained above in [protocol section](#the-websocket-protocol)
  * `int options` - Set options (See [options section](#options))
  * `std::string message` - The message to send to be broadcasted
  * Returns 0 if succeeded to send to server, otherwise a non-zero value on error
* Lets say we set key 4 as our message for updating color and we want to update it to red, we would call:
  * `mySocket.broadcast( 4, 0, "red");`
  
#### Handling Receiving a Message
* You need to first set an **Event** so that a function gets called apond receiving the message
* Example:
  * Lets say we want to call our function `void onNewColor( char* newColor );` when someone sends a message on key 4
  * To do this we need to set this function to key 4 event
	* `mySocket.setEvent( 4, onNewColor );`
  * Now every time someone sends a broadcast on key 4 this function gets called
* **NOTE:** all *Event* functions need to have a single parameter of `char*` and return `void`
  
### Error values
// TOOD
