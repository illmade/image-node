var Busboy = require('busboy');
var http = require('http');
var inspect = require('util').inspect;
var arrayUtils = require('./../common/util.js');

const classifier = require('../build/Release/classifier.node');

exports.imageForm = function(req, res) {
    res.render('upload', {
        title: 'Upload Images'
    });
};

exports.uploadImage = function(req, res, next){
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

        console.time("classify");
        var hrstart = process.hrtime();
        var boxes = classifier.box(img, fileType);
        var classifications = classifier.classify(img, fileType);
        var faces = classifier.align(img, fileType);
        var detections = classifier.detect(img, fileType);
        hrend = process.hrtime(hrstart);
        var timeTaken = hrend[1]/1000000;
        var secs = hrend[0] + "" + timeTaken;
        console.timeEnd("classify");
        console.log(secs);

        console.log('Boxes value: ' + boxes);
        console.log('Classifications value: ' + classifications);
        console.log('Faces value: ' + faces);
        console.log('Detections value: ' + detections);

        res.render('classify', { title: 'Classify', imageData: imgString,
            imageJson: boxes, classificationJson: classifications, 
            faceJson: faces, detectJson: detections, classifyTime: secs}, function(err, html) {
            res.send(html);
        });
    });
    req.pipe(busboy);
};