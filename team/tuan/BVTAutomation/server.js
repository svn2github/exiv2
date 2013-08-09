/**
 * Nhu Dinh Tuan (2013)
 * The code for a basic HTTP server with two main functions: Start & Restart http server.
 *
 * Last update: 29/4/2013
 */

var querystring = require("querystring"),
	http = require("http"),
	url = require("url"),
	router = require("./router"),
	socketIO = require('socket.io');
	
function start() {
	var port = _config.Port;
	
	function onRequest(request, response) {
		request.setEncoding("utf8");
		var requestData = {};
		
		// when the request is made.
		requestData.requestAt = (new Date()).toUTCString();
		
		// get request header
		requestData.headers = request.headers;
		
		// get parts of url
		var pathname = url.parse(request.url).pathname;
		if (pathname != "/favicon.ico") console.log("\nREQUEST TO " + pathname);
		requestData.parts = pathname.split("/");
		
		// get the method of the request.
		requestData.method = request.method;
		
		// get data via GET method
		var urlString = request.url;
		urlString = urlString.replace("+", "%2B");
		var url_parts = url.parse(urlString,true);
		requestData.getData = url_parts.query;
		
		
		// get data via POST method
		var postData = "";
		request.addListener("data", function(postDataChunk) {
			postData += postDataChunk;
		});
		request.addListener("end", function() {
			requestData.postData = querystring.parse(postData);
			try {
				router.route(response, requestData);
			} catch(err) {
				console.log(" SERIOUS ERROR: " + err);
				response.writeHead(400, {"Content-Type": "text/plain"});
				response.write("SERIOUS ERROR - " + err);
				response.end();
			}
		});
		
		// set the timeout for the connection.
		request.connection.setTimeout(10000);
	}
	
	// start the HTTP server
	serverInstance = http.createServer(onRequest).listen(port);
	
	// start socket.io for real time update
	_socketIO = socketIO.listen(serverInstance);
	_socketIO.set('log level', 1);
	
	_socketIO.sockets.on('connection', function (socket) {
		// add clients to the room (the same userID with the same request type is added to the same room)
		socket.on('register', function (data) {
			if (typeof data.buildVersion != 'undefined') {
				socket.join('V_' + data.buildVersion);
			}
		});
	})
	
	console.log("Server has started at port " + port);
}


exports.start = start;