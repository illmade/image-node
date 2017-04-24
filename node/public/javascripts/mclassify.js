function sendImageData(blob, mime, dest) {
    
	var formData = new FormData();

    formData.append("image_blob", blob);
    formData.append("image_type", mime);
    
    var xhr = new XMLHttpRequest;
    xhr.open( "POST", dest, true);
    xhr.onload = function (e) {
      if (xhr.readyState === 4) {
        if (xhr.status === 200) {
          console.log(xhr.responseText);
          var json = JSON.parse(xhr.responseText);
          drawBox("drawing", 'detect_canvas', json, "255,0,255", 3.0, "raw_image");
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