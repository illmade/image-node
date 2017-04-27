var express = require('express');

var router = express.Router();
var batch = require('../user/batch.js');

const classifier = require('../build/Release/classifier.node');
/* GET users listing. */
router
    .post('/', function (req, res, proxy) {
        var complete = batch.upload(req, res, this, new function(){
        	this.classifyImage = function(buffer, fileType, output){
        		var results = classifier.classifyAsync(buffer, fileType, function(inputs, results) {
        			output(results);
        		});
        	}
        });
    });

module.exports = router;