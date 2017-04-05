var express = require('express');

var router = express.Router();
var upload = require('../user/common.js');

/* GET users listing. */
router
    .post('/', function (req, res) {
        var imageData = upload.uploadImage(req, res, this);
    });


module.exports = router;