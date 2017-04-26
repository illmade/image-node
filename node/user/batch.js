var http = require('http');
var Busboy = require('busboy');
var arrayUtils = require('./../common/util.js');

const classifier = require('../build/Release/classifier.node');

exports.upload = function(req, res, next, proxy){
	
	var busboy = new Busboy({ headers: req.headers });

	var fileHash = {};
    
    busboy.on('file', function(fieldname, file, filename, encoding, mimetype) {
    	
    	var fileType = 0;

        if (mimetype == "image/jpeg"){
            fileType = 1;
        }
        else if (mimetype == "image/png"){
            fileType = 2;
        }
        else if (mimetype == "image/gif"){
            fileType = 3;
        }
        
        var buffer = new Uint8Array(0);
        
        var hashName = filename + mimetype;
        
        fileHash[hashName] = buffer;

        console.log('File [' + fieldname + ']: filename: ' + filename + ', encoding: ' + encoding + ', mimetype: ' + mimetype);
        
        file.on('data', function(data) {
            console.log('File [' + fieldname + ", " + filename + '] got ' + data.length + ' bytes');
            buffer = fileHash[hashName];
            buffer = arrayUtils.appendBuffer(buffer, data);
            fileHash[hashName] = buffer;
        });
        file.on('end', function() {
            console.log('File [' + fileHash + '] Finished');
            buffer = fileHash[hashName];
            
            console.log('total length data length: ' + buffer.length);
            
            var asynced = function(classifications){
            	if (classifications) {

            		console.log(classifications);
        			//var jsoned = JSON.parse(classifications.replace(re, "\""));
        			
        			//res.json(jsoned);
        		}
        		else {
        			//res.json({});
        		}
            }
            
    		var classifications = proxy.classifyImage(buffer, fileType, asynced);
        });
    });
    req.pipe(busboy);
}