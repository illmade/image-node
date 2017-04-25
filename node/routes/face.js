var express = require('express');

var router = express.Router();
var bus = require('../user/bus.js');

const classifier = require('../build/Release/classifier.node');

/* GET users listing. */
router
    .post('/', function (req, res) {
        var imageData = bus.processImage(req, res, this, new function(){
        	this.classifyImage = function(buffer, fileType, output){
        		var results = classifier.alignAsync(buffer, fileType, function(inputs, results) {
        			output(results);
        		});
        	}
        });
    });


module.exports = router;