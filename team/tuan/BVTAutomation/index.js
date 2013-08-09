/**
 * EXIV2 BVT
 * Nhu Dinh Tuan (2013)
 * Last update: 29/4/2013
 */
var fs = require('fs'),
    path = require('path');

    
/**
 * Get the port and config file name from the command line.
 */
var port = process.argv[2];
var cfgFile = process.argv[3];
if (typeof port != "undefined" && !isNaN(port) && typeof cfgFile != "undefined" && cfgFile != "") {
  GLOBAL._config = require('./config').get(port, cfgFile);
} else {
  var note_str = "ERROR - NOT ENOUGH PARAMETERS\n";
  note_str += "Usage:   node <bvt-dir>/index port_number config_file_name\n";
  note_str += "Example: node Exiv2/BVT/index.js 8888 gsoctrunk.\n";
  note_str += "Note:    8888 is the port for http server. User can run BVT & view the BVT result at here.\n";
  note_str += "         gsoctrunk is a unique name which is used for the generated config file. It should be the name of the svn branch."
  console.log(note_str);
  return;
}
    
// global variables
GLOBAL._server = require("./server");
GLOBAL._socketIO = ""; // start with http server
GLOBAL._isBVTRunning = false;
GLOBAL._emailFormat = fs.readFileSync(path.join(__dirname, "mailformat.txt")) //load email format

// start the server
_server.start();