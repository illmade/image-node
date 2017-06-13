browser.waitForAngularEnabled(false);

describe('Protractor App', function() {
	it('should have a title', function() {
		browser.get('http://localhost:3000/');
		expect(browser.getTitle()).toEqual('Upload');
	});

	it('should detect the dog', function() {
		browser.get('http://localhost:3000/');

		var form = element(by.xpath('/html/body/form[2]'));
		var boxFileInput = element(by.xpath('/html/body/form[2]/input'));

		boxFileInput.sendKeys('/Users/tim/projects/image-node/node/public/images/dog.jpg');
		form.submit().then(function() {

			var classified = browser.getPageSource().then(function(json) {
				return json.includes('"locations":[{"location":[1280.19,823.977,2395.39,2132.16],"score":0.999562,"classification":"Dog"}]}');
			});

			classified.then(function(bools) {
				expect(bools).toEqual(true);
			});
		});

	});

	it('should classify the image', function() {
		browser.get('http://localhost:3000/');

		var form = element(by.xpath('/html/body/form[3]'));
		var boxFileInput = element(by.xpath('/html/body/form[3]/input'));

		boxFileInput.sendKeys('/Users/tim/projects/image-node/node/public/images/dog.jpg');

		form.submit().then(function() {
			var classified = browser.getPageSource().then(function(json) {
				return json.includes('"label":"beagle","labelIndex":"163","score":"0.907476"');
			});

			classified.then(function(bools) {
				expect(bools).toEqual(true);
			});
		});

	});
	
	it('should find only low probability humans', function() {
		browser.get('http://localhost:3000/');

		var form = element(by.xpath('/html/body/form[4]'));
		var boxFileInput = element(by.xpath('/html/body/form[4]/input'));

		boxFileInput.sendKeys('/Users/tim/projects/image-node/node/public/images/dog.jpg');

		form.submit().then(function() {
			var classified = browser.getPageSource().then(function(json) {
				return json.includes('"location":[1359.31,913.602,2322.7,2158.49],"score":0.0218683');
			});

			classified.then(function(bools) {
				expect(bools).toEqual(true);
			});
		});

	});
});