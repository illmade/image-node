var request = require("request");

var base_url = "http://localhost:3000/"
var show_url = "http://localhost:3000/show"

describe("Asynchronous specs", function() {
	 var value;
	
	 beforeEach(function(done) {
	    setTimeout(function() {
	      value = 0;
	      done();
	    }, 1);
	  });
	 
	it("should support async execution of test preparation and expectations", function(done) {
	   value++;
	   expect(value).toBeGreaterThan(0);
	   done();
	});
		
	describe("Server", function() {
	  describe("GET /", function() {
	    it("returns status code 200", function(done) {
	      request.get(base_url, function(error, response, body) {
	        expect(response.statusCode).toBe(200);
	        done();
	      });
	    });
	  });
	});
	
	describe("Server", function() {
	  describe("GET /show", function() {
	    it("returns status code 404", function(done) {

	      request.get(show_url, function(error, response, body) {
	        expect(response.statusCode).toBe(404);
	        done();
	      });
	    });
	  });
	});
});