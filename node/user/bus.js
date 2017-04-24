var http = require('http');
var Busboy = require('busboy');
var arrayUtils = require('./../common/util.js');

const classifier = require('../build/Release/classifier.node');

exports.upload = function(req, res, next){
	
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
        console.log('Field [' + fieldname + ']: value: ' + inspect(val));
    });
    busboy.on('finish', function() {
        console.log('Done parsing form!');

        var img = new Buffer(buffer, 'base64');
        var imgString = img.toString('base64');
	
        res.render('show', { title: 'Classify', imageData: imgString, imageType: fileType}, function(err, html) {
            res.send(html);
        });
    });

    req.pipe(busboy);
}