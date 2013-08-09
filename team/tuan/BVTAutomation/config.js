/**
 * Nhu Dinh Tuan (2013)
 * This code is to load and save the configuration settings
 *
 * Last update: 29/4/2013
 */
 
var fs = require('fs'),
	path = require('path');

// the default folder
var CONFIG_DIR = "configs";
var _cfgFile = "";

function get(port, cfgFile) {
	_cfgFile = cfgFile;
	var filePath = path.join(__dirname, CONFIG_DIR, _cfgFile);
	
	var configData = {};
	try {
		// read the file
		var data = fs.readFileSync(filePath, 'utf8');
		configData = JSON.parse(data);
		if (port != configData.Port) {
			configData.Port = port;	  
			save(configData);
		}
	} catch(err) {
		// the file doesn't exist
		// create a new file with the default settings
		configData.Name = cfgFile;
		configData.Port = port;
		configData.LastBVTDate = 0;
		configData.LastBuild = 0;
		
		// create a folder
		try {
			fs.mkdirSync(path.join(__dirname, "logs", _cfgFile));  
		} catch (err) {
			// pass
		}
		save(configData);
	}

	return configData;
}


/**
 * Save the configuration to the file.
 **/
function save(configData) {
	
	// update the file
	var str = JSON.stringify(configData);
	fs.writeFile(path.join(__dirname, CONFIG_DIR, _cfgFile), str, function() {
		//
	});
}

exports.get = get;
exports.save = save;