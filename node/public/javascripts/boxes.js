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
    container.style.height = "" + height + "px";
    
    context.scale(scaling, scaling);
    
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