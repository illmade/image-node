function copyToCanvas(img, canvasId, imageType) {
	
    var img = document.getElementById(img);
    var width = img.getBoundingClientRect().width;
    var height = img.getBoundingClientRect().height;
    var nwidth = img.naturalWidth;
    var nheight = img.naturalHeight;
    
    var scaling = width / nwidth;
    
    var canvas = document.getElementById(canvasId);
    canvas.width = width;
    canvas.height = height;
    var ctx = canvas.getContext("2d");
    ctx.scale(scaling, scaling);
    ctx.drawImage(img,0,0);
    
    var output = "image/png";
    if (imageType = 1){
    	output = "image/jpeg";
    }
    else if (imageType = 2){
    	output = "image/gif";
    }
    
    var dataURL = canvas.toDataURL(output);
    
    //remove header e.g 'data:image/png;base64,'
    var start = dataURL.indexOf(",");
    var trimmedData = dataURL.substring(start+1);
    
    return trimmedData;
}

function sendImageData(time, blob, mime, dest, wrapper) {
    
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
          var total = end - time;
          var content = document.createTextNode("got: " + dest + ", in: " + secs + ", total: " + total);
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