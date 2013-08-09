/**
 * Nhu Dinh Tuan (2013)
 * This file contains the functions which do the actual work when a request is received.
 * Based on the router, the associated functions will be called.
 *
 * Last update: 9/8/2013
 */

var fs = require('fs'),
	path = require('path'),
	exec = require('child_process').exec,
	spawn = require('child_process').spawn,
	helper = require("./helper.js"),
	HTMLtemplate = require("./HTMLtemplate.js");
	
// Global variable
var log_path = null,
    bvt_log = null,
    command_index = 0,
    commands = new Array();

// List all commands which are run for BVT.
// Add new commands in the format 
//		commands[i] = new Array(name, exe, args, isPrintOutput);
// 		name: the name for the command, it's used to print in ourput
//		exe : the execute
//		args: the arguments
//		isPrintOutput: true if you want the command output is printed in the report.
    commands[0] = new Array('$ svn cleanup', 'svn', ['cleanup'], 0);
    commands[1] = new Array('$ svn revert --quiet -R .', 'svn', ['revert', '--quiet', '-R', '.'], 0);
    commands[2] = new Array('$ svn update --revision %x', 'svn', ['update', '--revision', '%x'], 1);
    commands[3] = new Array('$ make distclean', 'make', ['distclean'], 0);
    commands[4] = new Array('$ make config', 'make', ['config'], 1);
    commands[5] = new Array('$ ./configure --disable-nls', './configure', ['--disable-nls'], 0);
    commands[6] = new Array('$ make', 'make', [], 0);
    commands[7] = new Array('$ sudo make install', 'sudo', ['make', 'install'], 1);
    commands[8] = new Array('$ make smaples','make', ['samples'], 1);
    commands[9] = new Array('$ make tests','make', ['tests'], 1);
    commands[9] = new Array('$ make testx','make', ['testx'], 1);
    

// The BVT is only run for some revision. The revision whose commit log message contains the BUILD_KEY_WORD or "all" will be bvted.
var BUILD_KEY_WORD = "linux";

// support function
function checkBVT(expectedRevision, callback) {
	
	function checkSVNInfo(revision) {
		// set new build version
		if (revision > _config.LastBuild) {
			_config.LastBuild = revision;
			_config.LastBVTDate = helper.getUNIXTimestamp();
			require('./config').save(_config);
		}
		
		var logData = "";
		if (expectedRevision == "") {	
			logData = "$ svn info -r 'HEAD' | grep '^Revision'\n";
		}
		//check if developers want BVT to run for this revision
		exec("svn log -r " + revision + " | grep -o '\\[build:.*\\]' | grep -o 'all\\|"+BUILD_KEY_WORD+"'", function (error2, stdout2, stderr2) {
			if (stdout2 != "" && !error2) {
				console.log(" BVT is running in 3 seconds.");
				_isBVTRunning = new Array(helper.getUNIXTimestamp(), revision);
				logData += "###Run BVT in 3 seconds....\n";
				return callback(logData, revision, true);
			} else {
				console.log (" Not Running for the revision " + revision);  
				_isBVTRunning = false;
				// create the log file
				fs.writeFileSync(path.join(__dirname, "logs", _config.Name, revision +".txt"), "DONT NEED TO RUN BVT FOR THIS REVISION " + revision);
				logData += "###Don't need to run BVT for this revision " + revision + " \n";
				return callback(logData, revision, false);
			}
		})
	}
	
	if (expectedRevision != "") {
		// check the log	
		if (fs.existsSync(path.join(__dirname, "logs", _config.Name, expectedRevision +".txt"))) {
			console.log(" The BVT has been already checked for this build[" + expectedRevision + "].\n");
			_isBVTRunning = false;
			return callback("###The BVT has been already checked for this build. Please check the log file.\n", expectedRevision, false);
		} else {
			console.log(" The BVT has never run for this build[" + expectedRevision + "]. Check SVN info");
			checkSVNInfo(expectedRevision);
		}
	} else {
		exec("svn info -r 'HEAD' | grep '^Revision'", function (error, stdout, stderr) {
			var svn_info = stdout;
			var logData = "$ svn info -r 'HEAD' | grep '^Revision'\n";
			if (svn_info == "") {
				logData += error;
				console.log(" Error $ svn info: " + error);
				_isBVTRunning = false;
				return callback(logData, _config.LastBuild, false);
			} else {
				logData += svn_info + "\n";
				var latest_revision = svn_info.split(": "); 
				latest_revision = parseInt(latest_revision[1]);
				// don't need to run BVT
				if (latest_revision == _config.LastBuild) {
					console.log(" The BVT has been already checked for this build[" + latest_revision + "]. Please check the log file.");
					logData += "###The BVT has been already checked for this build[" + latest_revision + "]. Please check the log file.\n";
					_isBVTRunning = false;
					return callback(logData, latest_revision, false);
				} else {
					checkSVNInfo(latest_revision);
				}
				
			}
			
		});
	}
}

var commandCallBack = function(code) {
	command_index++;
	if (code == 0 && command_index < commands.length) { // false
		runCommands(commandCallBack);
		return;
	} 

	// remove flag file which prevents BVT of other build running at the same time
	fs.writeFileSync(path.join(__dirname, "IsBVTRunning.txt"), "");
	
	var emailBody = "";
	var bvtStartAt = new Date(_isBVTRunning[0] * 1000);
	var totalTime = parseFloat(parseInt(helper.getUNIXTimestamp() - _isBVTRunning[0])/60).toFixed(2);
	if (code != 0) {
		console.log(" Failed at " + commands[command_index-1][0]);
		_socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('fail', commands[command_index-1][0]);
		emailBody = "The Exiv2 BVT has been run on the ubuntu machine at " + bvtStartAt;
		emailBody += "\nName: " + _config.Name;
		emailBody += "\nSVN Version: " + _isBVTRunning[1];
		emailBody += "\nResult: FAIL AT " + commands[command_index-1][0];
		emailBody += "\nTotal run time: " + totalTime + " minutes";
		emailBody += "\nMore details are available at http://54.251.248.216:" + _config.Port + "/archive/" + _isBVTRunning[1];
	} else {
		console.log(" Completed all.");
		_socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('finish', totalTime);
		emailBody = "The Exiv2 bvt has been run on the ubuntu machine at " + bvtStartAt;
		emailBody += "\nName: " + _config.Name;
		emailBody += "\nSVN Version: " + _isBVTRunning[1];
		emailBody += "\nResult: RUN ALL COMMANDS SUCCESSFULLY ";
		emailBody += "\nTotal run time: " + totalTime + " minutes";
		emailBody += "\nIt may not ensure that the build passed all test cases. Please take a look at http://54.251.248.216:" + _config.Port + "/archive/" + _isBVTRunning[1] + " for more details";
	}
	
	fs.closeSync(bvt_log);
	
	
	// email the result
        var mailcontent = _emailFormat + "";
	var emailSpawn = spawn('ssmtp', ['-t']);
	if (code != 0) {
        	mailcontent = mailcontent.replace(/%subject/g, "FAIL - EXIV2 UBUNTU BVT " + _config.Name + " - VERSION " + _isBVTRunning[1]);
	} else {
		mailcontent = mailcontent.replace(/%subject/g, "OK - EXIV2 UBUNTU BVT " + _config.Name + " - VERSION " + _isBVTRunning[1]);
	}
	mailcontent = mailcontent.replace(/%body/g, emailBody);
        var bufferMailcontent = new Buffer(mailcontent);
        emailSpawn.stdin.write(bufferMailcontent);
        emailSpawn.stdin.end();

        emailSpawn.stderr.on('data', function (data) {
	    console.log(data);
        });
        emailSpawn.on('exit', function (code) {
	    if (code !== 0) {
		console.log(' Unable to email the result with code ' + code);
	    } else {
		console.log(' Email is sent to developers.');
	    }
        });
	
	_isBVTRunning = false;
}	


function runBVT(buildVersion) {
	console.log(" Run BVT ");
	
	// set flag file to prevent BVT of other build running at the same time
	fs.writeFileSync(path.join(__dirname, "IsBVTRunning.txt"), _config.Name + "_" + buildVersion + " http://54.251.248.216:" + _config.Port);
	
	// create the log file
	log_path = path.join(__dirname, "logs", _config.Name, buildVersion +".txt");
	bvt_log = fs.openSync(log_path, 'a');

	// reset
	command_index = 0;
	
	// set the revision
	commands[2][0] = commands[2][0].replace(/%x/g, buildVersion);
	commands[2][2][2] = buildVersion + "";
	
	// run commands
	runCommands(commandCallBack);
}


/**
 * COMMANDS IN ORDER
 */
function runCommands(callback) {
	var str = '---------------------------- ' + commands[command_index][0] + ' ----------------------------\n';
	var buffer = new Buffer(str);
	var socketString = str;
	_socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', socketString);
	console.log(" Run " + commands[command_index][0]);
	fs.writeSync(bvt_log, buffer, 0, buffer.length);

	var commandSpawn = spawn(commands[command_index][1], commands[command_index][2]);
	
	if (commands[command_index][3] == 0) {
	  _socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', '###The ouput of this command is skiped. Please check log file later\n');
	}
	commandSpawn.stdout.on('data', function (data) {
		fs.writeSync(bvt_log, data, 0, data.length);
		if (commands[command_index][3] == 1) {
		  socketString = data.toString('utf8');
		  _socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', socketString);
		} else {
		  _socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', '. ');
		}
		
	});

	commandSpawn.stderr.on('data', function (data) {
		fs.writeSync(bvt_log, data, 0, data.length);
		if (commands[command_index][3] == 1) {
		    socketString = data.toString('utf8');
		    _socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', socketString);
		} else {
		    //
		}
		
	});

	commandSpawn.on('exit', function (code) {
		console.log(" Exit with code " + code);
		str = '\n----------------------------  Exited with code ' + code + '\n\n';
		buffer = new Buffer(str);
		fs.writeSync(bvt_log, buffer, 0, buffer.length);
		_socketIO.sockets.in('V_' + _isBVTRunning[1]).emit('stdout', str);
		return callback(code);
	});
	
	
	commandSpawn.on('error', function (code) {
	  console.log(" Error with code " + code);
	});
}



/**
 * Introduction page
 */
function intro(response, requestData) {
	console.log(" Request handler 'intro' was called.");
	
	// this function is called after loading the HTML file
	function callback(data) {
		if (data == null) {
			response.writeHead(400, {"Content-Type": "text/plain"});
			response.write("Error: Unable to load the HTML template");
			console.log(" Error: Unable to load the HTML template intro.html");
		} else {
			console.log(" Load the HTML template intro.html");
			response.writeHead(200, {"Content-Type": "text/html"});
			
			var str = data + "";
			str = str.replace(/%BVTName/g, _config.Name);
			str = str.replace(/%LastBuild/g, _config.LastBuild);
			if (_config.LastBuild != 0) {
				var date = new Date(_config.LastBVTDate * 1000);
				str = str.replace(/%LastBVT/g, "at " + date);
			} else {
				str = str.replace(/%LastBVT/g, "never run");
			}
			response.write(str);
		}
		response.end();
	}
	
	// load the introduction HTML file
	HTMLtemplate.load("intro.html", callback);
}

function bvt(response, requestData) {
	console.log(" Request handler 'bvt' was called.");

	var expectedRevision = "";
	if (typeof requestData.parts[2] != "undefined") expectedRevision = parseInt(requestData.parts[2]);
	
	// prevent users running the bvt for the non-exist revision
	if (expectedRevision != "" && expectedRevision > _config.LastBuild) {
		response.writeHead(200, {"Content-Type": "text/plain"});
		response.write("Unable to check BVT because the revision [" + expectedRevision+ "] you want to run > the latest revision [" + _config.LastBuild + "] \n");
		console.log("Unable to check BVT now because the revision [" + expectedRevision+ "] you want to run > the latest revision [" + _config.LastBuild + "] \n");
		response.end();
		return;
	}
	
	// check whether there is any other BVT of other builds is running. Reason: performance concern
	if (_isBVTRunning == false) {
		BVTFlagFileContent = "";
		try {
			BVTFlagFileContent = fs.readFileSync(path.join(__dirname, "IsBVTRunning.txt"), 'utf8');
		} catch (err) {
		// pass
		}
		if (BVTFlagFileContent != "") {
			response.writeHead(200, {"Content-Type": "text/plain"});
			response.write("Unable to check BVT now because BVT for other build is running on this machine\n" + BVTFlagFileContent);
			console.log("Unable to check BVT now because BVT for other build is running on this machine\n" + BVTFlagFileContent);
			response.end();
			return;
		}
	}
	
	
	// this function is called after loading the HTML file
	function callback(data) {
		if (data == null) {
			response.writeHead(400, {"Content-Type": "text/plain"});
			response.write("Error: Unable to load the HTML template");
			console.log(" Error: Unable to load the HTML template lastBVT.html");
			response.end();
		} else {
			console.log(" Load the HTML template lastBVT.html");
			response.writeHead(200, {"Content-Type": "text/html"});
			var str = data + "";
			
			if (_isBVTRunning == false) {		  
			    console.log(" Check BVT.");
			    checkBVT(expectedRevision, function(logData, version, needBVT){
				    str = str.replace(/%LastBuild/g, version);
				    str = str.replace(/%LogContent/g, logData);
				    response.write(str);
				    response.end();
				    
				    if (needBVT) setTimeout(function(){runBVT(version);}, 3000);
			    });
			} else {
			    console.log(" BVT is running.");
			    if (_isBVTRunning[1] == expectedRevision || (expectedRevision == "" && _isBVTRunning[1] == _config.LastBuild)) {
			    	console.log(" the same build revision");
				var date = new Date(_isBVTRunning[0] * 1000);
			    	str = str.replace(/%LastBuild/g, _isBVTRunning[1]);
			    	str = str.replace(/%LogContent/g, "###BVT is running. It started at " + date + ". \n###The latest output is shown below:\n");
			    } else {
			    	console.log(" the running build revision is not the same as the expected revision.");
				str = "Unable to check BVT for the revision [" + expectedRevision + "] because BVT for other build [" + _isBVTRunning[1] + "] is running on this machine";
			    }
			    
			    response.write(str);
			    response.end();
			}
		}
	}
	
	// load the lastBVT HTML file
	HTMLtemplate.load("runBVT.html", callback);
	
	
}

function archive(response, requestData) {
	console.log(" Request handler 'archive' was called.");
	var buildVersion = "", logData = "";
	if (typeof requestData.parts[2] != "undefined") buildVersion = requestData.parts[2];
	if (buildVersion == "latest") buildVersion = _config.LastBuild;
	
	try {
		var filePath = path.join(__dirname, "logs", _config.Name, buildVersion +".txt");
		logData = fs.readFileSync(filePath);
	} catch(err) {
		console.log(" No log file for the build " + buildVersion);
		logData = "No log file for the build " + buildVersion;
	}

	response.writeHead(200, {"Content-Type": "text/plain"});
	response.write(logData);
	response.end();
}

function reset(response, requestData) {
	console.log(" Request handler 'reset' was called.");
	response.writeHead(200, {"Content-Type": "text/plain"});

	if (_isBVTRunning != false) {
		console.log(" Can not reset the server because the BVT is running.");
		response.write("Can not reset the server because the BVT is running.");
		response.end();
		return;
	}


	// remove all files under the directory
	/*
	rmContentDir = function(dirPath) {
	    try { var files = fs.readdirSync(dirPath); }
	    catch(e) { return; }
	    if (files.length > 0)
		for (var i = 0; i < files.length; i++) {
		    var filePath = dirPath + '/' + files[i];
		    if (fs.statSync(filePath).isFile())
			fs.unlinkSync(filePath);
		    else
			rmDir(filePath);
		}
	};*/


	// remove log files
	//logsDir = path.join(__dirname, "logs");
	//rmContentDir(logsDir);
	var BVTlogFile = path.join(__dirname, "logs", _config.Name, _config.LastBuild + ".txt");
	try {
	    if (fs.statSync(BVTlogFile).isFile())
	    fs.unlinkSync(BVTlogFile);
	} catch (err) {	
	    console.log(" Error remove the log " + _config.LastBuild + ".txt: " + err);
	}

	// set new build version
	_config.LastBuild = 0;
	_config.LastBVTDate = 0;
	require('./config').save(_config);

	response.write("The server is reset successfully.");
	response.end();
}

function test(response, requestData) {
  console.log(" Request handler 'test' was called.");
  response.writeHead(200, {"Content-Type": "text/plain"});
  response.end();
}

exports.intro = intro;
exports.bvt = bvt;
exports.archive = archive;
exports.reset = reset;
exports.test = test;