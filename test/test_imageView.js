var assert = require("assert")
var fs     = require('fs');
var inspect = require('util').inspect;

var cloudcv = require("../build/Release/cloudcv");

describe('cv', function() {

    describe('ImageView', function() {
        it('Create from image', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-logo.jpg");
            assert.equal(599, imview.width());
            assert.equal(555, imview.height());
            done();
        });

        it('asJpegStream', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-logo.jpg");
            imview.asJpegStream(function(err, data) {
                assert.equal(null, err);
                assert.notEqual(null, data);
                //fs.writeFile('_opencv_log.jpg', new Buffer(data));
                done();
            });
        });

        it('asJpegDataUri', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-logo.jpg");
            imview.asJpegDataUri(function(err, data) {
                assert.equal(null, err);
                assert.notEqual(null, data);
                fs.writeFile('_opencv_log.html', new Buffer(data));
                done();
            });
        });

        it('asPngStream', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-logo.jpg");
            imview.asPngStream(function(err, data) {
                assert.equal(null, err);
                assert.notEqual(null, data);
                done();
            });
        });

        it('asPngDataUri', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-logo.jpg");
            imview.asPngDataUri(function(err, data) {
                assert.equal(null, err);
                assert.notEqual(null, data);
                done();
            });
        });

        it('asObject', function(done) {
            var imview = new cloudcv.ImageView("test/opencv-small.png");
            var object = imview.asObject();
            assert.notEqual(null, object);
            //console.log(inspect(object));
            done();
        });
    });
});