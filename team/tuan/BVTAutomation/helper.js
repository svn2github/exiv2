
function getUNIXTimestamp() {
	return Math.round((new Date()).getTime() / 1000);
}

exports.getUNIXTimestamp = getUNIXTimestamp;