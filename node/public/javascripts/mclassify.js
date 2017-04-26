function sendImageData(blob, mime, dest, wrapper) {
    
	var docConsole=document.getElementById("console");
	
	var start = window.performance.now();

	var content = document.createTextNode("checking: " + dest);
	docConsole.appendChild(content);
	var br = document.createElement("br");
	docConsole.appendChild(br);
	
	var formData = new FormData();
	
    formData.append("image_blob", blob);
    formData.append("image_type", mime);
    
    var xhr = new XMLHttpRequest;
    xhr.open( "POST", dest, true);
    xhr.onload = function (e) {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          console.log(xhr.responseText);
          var end = window.performance.now();
          var secs = end - start;
          var content = document.createTextNode("got: " + dest + ", in: " + secs);
      	  docConsole.appendChild(content);
      	  var br = document.createElement("br");
    	  docConsole.appendChild(br);
          
          var json = JSON.parse(xhr.responseText);
          wrapper.apply(json);
        } else {
          console.error(xhr.statusText);
        }
      }
    };
    xhr.onerror = function (e) {
      console.error(xhr.statusText);
    };
    xhr.send(formData);

}