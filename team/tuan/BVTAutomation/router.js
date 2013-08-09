/**
 * Nhu Dinh Tuan (2013)
 * The code provides the routing function.
 * "Routing" means, we want to handle requests to different URLs differently
 *
 * Last update: 29/4/2013
 */

var requestHandlers = require("./requestHandlers");

var handle = {};
handle["/"] = requestHandlers.intro;
handle["/bvt"] = requestHandlers.bvt;
handle["/archive"] = requestHandlers.archive;
handle["/reset"] = requestHandlers.reset;
handle["/test"] = requestHandlers.test;

// map request handlers to which function to execute 
function route(response, requestData) {
	var pathname = "/" + requestData.parts[1];
	
	if (pathname == "/favicon.ico") {response.end(); return;}
	
	// routing to the associated function.
	if (typeof handle[pathname] == 'function') {
		handle[pathname](response, requestData);
	} else {
		console.log("Error 404: No request handler found for " + pathname);
		response.writeHead(404, {"Content-Type": "text/plain"});
		response.write("404 Not found");
		response.end();
	}
}

exports.route = route;