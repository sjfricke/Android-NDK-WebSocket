const PORT = 6419;
const VERBOSE = true;
var key = "";
var option = "";
var value = "";
var nl_1 = -1; //new line 1
var nl_2 = -1; 

var net = require('net');
var server = net.createServer( (connection) => {
    if (VERBOSE) console.log('client connected');

    connection.on('close', function (){
	console.log('client disconnected');
    });

    connection.on('data', (data) =>  {
	// getting breaks in protocol
	data = data.toString();
	nl_1 = data.indexOf("\n");
	nl_2 = data.substr(nl_1+1).indexOf("\n") + (nl_1 + 1);
	// getting broken up parts
	key = data.substring(0,nl_1);
	option = data.substring(nl_1+1, nl_2);
	value = data.substring(nl_2+1, data.indexOf("\0"));

	if (VERBOSE) console.log("key: ", key, "\toption: ", option, "\tvalue: ", value);
	connection.write("0");
	if (VERBOSE) console.log('Sended responst to client');
//	connection.end();
//	console.log('Disconnected the client.');
    });
});

server.listen(PORT, () =>  {
    console.log('NodeJS Socket Server Ready - Port ' + PORT);
}); 
