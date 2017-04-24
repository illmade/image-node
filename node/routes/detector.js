var express = require('express');

var router = express.Router();
var upload = require('../user/ssd.js');

/* GET users listing. */
router
    .post('/', function (req, res) {
        var imageData = upload.ssdUpload(req, res, this);
    });


module.exports = router;