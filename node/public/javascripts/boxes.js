function toggle(check, display){
	
	var canvas=document.getElementById(display);
	
	if (check.checked){
		canvas.style.display="None";
	}
	else {
		canvas.style.display="Inline";
	}
}

/**
 * Created by tim on 07/03/2017.
 */
function drawBox(containerId, canvasId, json, color, scoreMultiplier, canvasImage){
	
    var container=document.getElementById(containerId);
    var canvas=document.getElementById(canvasId);
    var context=canvas.getContext("2d");
    
    context.save();
    
    var scalingElement;
    if (canvasImage){
    	scalingElement = document.getElementById(canvasImage);
    }
    else {
    	scalingElement = canvas;
    }

    var width = scalingElement.getBoundingClientRect().width;

    canvas.width = width;
    
    var scaling = width / json.size[0];

    var height = json.size[1] * scaling;

    canvas.height = height;
    //container.style.height = "" + height + "px";
    
    context.scale(scaling, scaling);
    context.font = "30px Arial";
    context.fillStyle = "rgba(255,255,0,0.5)";
    
    context.lineWidth = 3 / scaling;

    for (i=0; i<json.locations.length; i++){

        var box = json.locations[i].location;
        var x = box[0];
        var y = box[1];
        var w = box[2] - x;
        var h = box[3] - y;

        context.beginPath();
        context.rect(x, y, w, h);

        var alpha = Math.min(1, json.locations[i].score * scoreMultiplier);

        context.strokeStyle = "rgba(0,0,0," + alpha + ")";
        context.stroke();

        context.beginPath();
        context.rect(x+3/scaling, y+3/scaling, w-6/scaling, h-6/scaling);

        context.strokeStyle = "rgba(" + color + "," + alpha + ")";
        context.stroke();
        
        var classification = json.locations[i].classification;
        
        if (classification){
        	context.fillStyle = "rgba(0,0,0,0.7)";
        	context.fillText(classification, x, y+20);
        	context.strokeStyle = "rgba(255,255,255,0.7)";
        	context.lineWidth = 1;
        	context.strokeText(classification, x, y+20);
        }
    }
    
    context.restore();
}

/**
 * Created by tim on 07/03/2017.
 */
function drawFace(containerId, canvasId, json, color, scoreMultiplier, canvasImage){
    var container=document.getElementById(containerId);
    var canvas=document.getElementById(canvasId);
    var context=canvas.getContext("2d");
    
    context.save();
    
    var scalingElement;
    if (canvasImage){
    	scalingElement = document.getElementById(canvasImage)
    }
    else {
    	scalingElement = canvas;
    }

    var width = scalingElement.getBoundingClientRect().width;

    canvas.width = width;
    
    var scaling = width / json.size[0];

    var height = json.size[1] * scaling;

    canvas.height = height;
    //container.style.height = "" + height + "px";
    
    context.scale(scaling, scaling);
    
    context.lineWidth = 3 / scaling;

    for (i=0; i<json.locations.length; i++){

        var box = json.locations[i].location;
        var x = box[0];
        var y = box[1];
        var w = box[2] - x;
        var h = box[3] - y;
        
        var xscale = 0;
        var yscale = 0;
        
        var xshift = 0;
        var yshift = 0;
        
        var points = [];
        
        if(json.alignments){
	        var adjust = json.alignments[i].adjust;
	        var ax = adjust[0];
	        var ay = adjust[1];
	        
	        var aw = adjust[2] - ax;
	        var ah = adjust[3] - ay;
	        
	        xscale = w / aw;
	        yscale = h / ah;
	        
	        xshift = x - ax;
	        yshift = y - ay;
	        
	        points = json.alignments[i].alignment;
        }
        
        context.beginPath();
        context.rect(x, y, w, h);

        var alpha = Math.min(1, json.locations[i].score * scoreMultiplier);

        context.strokeStyle = "rgba(0,0,0," + alpha + ")";
        context.stroke();

        context.beginPath();
        context.rect(x+3/scaling, y+3/scaling, w-6/scaling, h-6/scaling);

        context.strokeStyle = "rgba(" + color + "," + alpha + ")";
        context.stroke();
        
        context.beginPath();
        
        for (p=0; p<points.length; p++){
        	var pxa = x + points[p][0] * w;
        	var pya = y + points[p][1] * h;
            
            var px = (x - xshift + points[p][0] * w / xscale) ;
            var py = (y - yshift + points[p][1] * h / yscale) ;
            
        	context.moveTo(px, py);
        	context.arc(px, py, 1, 0,2*Math.PI);
        }
        context.strokeStyle = "rgba(0,255,0,0.4)";
        context.stroke();
    }
    
    context.restore();
}

function classify(containerId, json) {

    var container = document.getElementById(containerId);
    for (i=0; i<json.scores.length; i++){
        var li = document.createElement("li");
        li.innerHTML = json.scores[i].label + ": " + json.scores[i].score;
        container.appendChild(li)
    }

}

function wrapClassify(){
	this.apply = function(json){
		classify("scores", json);
	}
}

function wrapBox(options){
	
	if (options){
		this.color = options.color;
		this.canvas = options.canvas;
	}
	else {
		this.color = "255,0,255";
		this.canvas = "detect_canvas";
	}
	
	this.apply = function(json){
		drawBox("drawing", this.canvas, json, this.color, 3.0, "raw_image");
	}
	
}

function wrapFace(){
	
	this.apply = function(json){
		drawFace("drawing", 'face_canvas', json, "255,0,0", 0.6, "raw_image");
	}
	
}
