var http = require('http');
var Busboy = require('busboy');
var arrayUtils = require('./../common/util.js');

const classifier = require('../build/Release/classifier.node');

exports.ssdUpload = function(req, res, next){
	
	var busboy = new Busboy({ headers: req.headers });

    var buffer = new Uint8Array(0);

    var fileType = 0;

    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {

        if (mimetype == "image/jpeg"){
            fileType = 1;
        }
        else if (mimetype == "image/png"){
            fileType = 2;
        }
        else if (mimetype == "image/gif"){
            fileType = 3;
        }

        console.log('File [' + fieldname + ']: filename: ' + filename + ', encoding: ' + encoding + ', mimetype: ' + mimetype);
        
        file.on('data', function(data) {
            console.log('File [' + fieldname + '] got ' + data.length + ' bytes');
            buffer = arrayUtils.appendBuffer(buffer, data);
        });
        file.on('end', function() {
            console.log('File [' + fieldname + '] Finished');
            console.log('total length data length: ' + buffer.length);
        });
    });
    busboy.on('field', function(fieldname, val, fieldnameTruncated, valTruncated, encoding, mimetype) {
        console.log('Field [' + fieldname + ']' + encoding + mimetype);
        if (fieldname=='image_blob')
        	buffer = Buffer.from(val, 'base64');
        if (fieldname=='image_type')
        	fileType = parseInt(val);
    });
    busboy.on('finish', function() {
        console.log('Done parsing form');
        var re = /'/gi;
		var detections = classifier.detect(buffer, fileType);
		var jsoned = JSON.parse(detections.replace(re, "\""));
		
		res.json(jsoned);
    });

    req.pipe(busboy);
}