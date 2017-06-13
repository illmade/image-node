function wrapResult(done){
	
	this.apply = function(json){
		console.log("applied");
		console.log(json);
		done();
	};
	
	this.fail = function(message){
		console.log("failed");
		console.log(json);
		done();
	};
	
};

var testPath = "/base/test/resources/images/dog.jpg";

describe("Asynchronous specs", function() {
	var value;

	beforeEach(function(done) {
		setTimeout(function() {
			value = 0;
			done();
		}, 1);
	});
	
	var consoleElement = document.createElement('div');
	consoleElement.id = "console";
	document.documentElement.appendChild(consoleElement);

	describe("Load an image ===", function() {

		it("should create base64 data", function (done) { 
			var expected = new Image(); 
			
			expected.onload = function () {
				console.log(expected, expected.width, expected.height, expected.naturalHeight);
				
				var canvas = document.createElement('canvas');
				var context = canvas.getContext('2d');
				context.drawImage(expected, 0, 0);
				
				var scaling = 0.1;
				context.scale(scaling, scaling);
				
				var data = context.getImageData(0,0, expected.width * scaling, expected.height * scaling);
				
				var dataURL = canvas.toDataURL("image/jpeg");
			    
			    //remove header e.g 'data:image/png;base64,'
			    var start = dataURL.indexOf(",");
			    var trimmedData = dataURL.substring(start+1);
			    
			    var header = dataURL.substring(0, start);
			    
			    console.log(header);
			    
				console.log(trimmedData.substring(0, 10));
				
				var time = window.performance.now();
				
				var reply = new wrapResult(done);
				
//				sendImageData(time, trimmedData, 1, "box", reply);
			    
			    expect(header).toEqual("data:image/jpeg;base64");
			    
				done();
				
			};

			expected.onerror = function(e) {
				console.log(e);
			}

			expected.src = testPath;

		}, 10000);
	});
	
	
	
});