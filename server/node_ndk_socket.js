const PORT = 6419;
const VERBOSE = false;
var key = "";
var option = "";
var value = "";
var nl_1 = -1; //new line 1
var nl_2 = -1; 

var vec = [];
var data_chunks = ""; // concatination of all chunks
var chunks = false;

var fs = require('fs');
var net = require('net');
var server = net.createServer( (connection) => {
    console.log('client connected');

    connection.on('close', () => {
	console.log('client disconnected');
    });

    connection.on('data', (data) =>  {
	
	// getting breaks in protocol
	//data = data.toString().replace(/\0[\s\S]*$/g,'');
	data = data.toString();
	nl_1 = data.indexOf("\n");

	nl_2 = data.substr(nl_1+1).indexOf("\n") + (nl_1 + 1);
	// getting broken up parts
	key = data.substring(0,nl_1);

	// if nl_1 is -1 then its being sent as data chunks
	// TODO find why it can start wit a \n
	if (nl_1 == -1 || key.length >  8) {
	    chunks = true;
	    data_chunks += data;
	} else {
	    	    
	    option = data.substring(nl_1+1, nl_2);
	    
	    // if it doesn't null terminate then it would be a empty string
	    if (key == 3) {
		value = data.substr(nl_2+1, data.indexOf("&"));
	    } else if (data.indexOf("\0") == -1) {
		value = data.substr(nl_2+1);
	    } else {
		value = data.substring(nl_2+1, data.indexOf("\0"));
	    }
	
	    if ( data.length > 0 && key != 1) {

		//check if data was being chunked
		if (chunks) {
		    if (VERBOSE) console.log("binding chunks");
		    chunks = false;
		    value = data_chunks + value;
		    data_chunks = "";
		}

		// Parse data - Format: X/Y/Z, ( ex: 3.2/4.1/7.5, == x:3.2, y:4.1, z:7.5 )
		let points = value.split(",");
		for (let i = 0; i < points.length; i++) {
		    let point = points[i].split("/");
		    
		    x_f = parseFloat(point[0]);
		    y_f = parseFloat(point[1]);
		    z_f = parseFloat(point[2]);

		    if (x_f == null || y_f == null || z_f == null ) { continue; } //skip if invalid

		    vec.push( { "x" : x_f, "y" : y_f, "z" : z_f } );
		}
		console.log("pushed vecs");

		if (key == 3) {
		    console.log("key == 3");
		    fs.writeFileSync("test", JSON.stringify(vec), 'utf8');
		    console.log("file write!");
		    vec = [];
		}
	    }
	    
	    if (VERBOSE) console.log("key: ", key, "\toption: ", option);
	    
	}
	    
	connection.write("0"); //Sended response to client
	
//	connection.end();
//	console.log('Disconnected the client.');
    });

    connection.on('end', () => {
	console.log('END');
    });

    connection.on('error', (err) => {
	console.log('ERROR: ', err);
    });

});

server.listen(PORT, () =>  {
    console.log('NodeJS Socket Server Ready - Port ' + PORT);
}); 
