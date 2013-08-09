/**
 * This is a very simple code to read the HTML file.
 *
 * Nhu Dinh Tuan (2013)
 * Last update: 29/4/2013
 */

var fs = require('fs'),
	path = require('path');

function load(fileName, callback) {
	filePath = path.join(__dirname, "HTMLtemplates", fileName);
	fs.readFile(filePath, function (err, data) {
		if (err) {
			return callback(null);
		} else {
			return callback(data);
		}
	});
}
exports.load = load;