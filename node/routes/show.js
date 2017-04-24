var express = require('express');

var router = express.Router();
var bus = require('../user/bus.js');

/* GET users listing. */
router
    .post('/', function (req, res) {
        var imageData = bus.upload(req, res, this);
    });


module.exports = router;