var express = require('express');

var router = express.Router();
var show = require('../user/show.js');

/* GET users listing. */
router
    .post('/', function (req, res) {
        var imageData = show.upload(req, res, this);
    });


module.exports = router;