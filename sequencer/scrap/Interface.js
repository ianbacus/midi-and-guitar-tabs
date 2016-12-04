/* globals Tone, StartAudioContext */


var Interface = {
    isMobile : false
};

/**
 *
 *
 *  INIT
 *  
 */
$(function(){
	var topbar = $("<div>").attr("id", "TopBar");
	$("body").prepend(topbar);
	
	if (typeof Logo !== "undefined"){
	    Logo({
		    "container" : topbar.get(0),
			"height" : topbar.height() - 6,
			"width" : 140
			});

	}
	$("<div>")
	    .attr("id", "Examples")
	    .attr("title", "examples")
	    .html("<a href='./index.html'>examples</a>")
	    .appendTo(topbar);
	//mobile start
	if( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ) {
	    Interface.isMobile = true;
	    $("body").addClass("Mobile");
	    var element = $("<div>", {"id" : "MobileStart"}).appendTo("body");
	    var button = $("<div>").attr("id", "Button").text("Enter").appendTo(element);
	    StartAudioContext.setContext(Tone.context);
	    StartAudioContext.on(button);
	    StartAudioContext.onStarted(function(){
		    element.remove();
		});
	}
    });

/**
 *
 *LOADING INDICATOR
 *  
 */
Interface.Loader = function(){
    this.element = $("<div>", {
	    "id" : "Loading",
	}).appendTo("body");

    this.text = $("<div>", {
	    "id" : "Text",
	    "text" : "Loading"
	}).appendTo(this.element);

    Tone.Buffer.on("load", function(){
	    this.element.addClass("Loaded");
	}.bind(this));
};

/**
 *
 *  
 *  DRAGGER
 *
 */
Interface.Dragger = function(params){

    if ($("#DragContainer").length === 0){
	$("<div>", {
		"id" : "DragContainer"
	    }).appendTo(params.parent || "#Content");
    }

    this.container = $("#DragContainer");

    /**
     *  the tone object
     */
    this.tone = params.tone;

    /**
     *  callbacks
     */
    this.start = params.start;

    this.end = params.end;

    this.drag = params.drag;

    /**
     *  the name
     */
    var name = params.name ? params.name : this.tone ? this.tone.toString() : "";

    /**
     *  elements
     */
    this.element = $("<div>", {
	    "class" : "Dragger",
	    "id" : name
	}).appendTo(this.container)
    .on("dragMove", this._ondrag.bind(this))
    .on("touchstart mousedown", this._onstart.bind(this))
    .on("dragEnd touchend mouseup", this._onend.bind(this));

    this.name = $("<div>", {
	    "id" : "Name",
	    "text" : name
	}).appendTo(this.element);

    this.element.draggabilly({
	    "axis" : this.axis,
		"containment": this.container
		});

    /**
     *  x slider
     */
    var xParams = params.x;
    xParams.axis = "x";
    xParams.element = this.element;
    xParams.tone = this.tone;
    xParams.container = this.container;
    this.xAxis = new Interface.Slider(xParams);

    /**
     *  y slider
     */
    var yParams = params.y;
    yParams.axis = "y";
    yParams.element = this.element;
    yParams.tone = this.tone;
    yParams.container = this.container;
    this.yAxis = new Interface.Slider(yParams);

    //set the axis indicator
    var position = this.element.position();
    this.halfSize = this.xAxis.halfSize;
    this.xAxis.axisIndicator.css("top", position.top + this.halfSize);
    this.yAxis.axisIndicator.css("left", position.left + this.halfSize);
};

Interface.Dragger.prototype._ondrag = function(e, pointer){
    if (this.drag){
	this.drag();
    }
    this.xAxis._ondrag(e, pointer);
    this.yAxis._ondrag(e, pointer);
    var position = this.element.position();
    this.xAxis.axisIndicator.css("top", position.top + this.halfSize);
    this.yAxis.axisIndicator.css("left", position.left + this.halfSize);
};

Interface.Dragger.prototype._onstart = function(e){
    if (this.start){
	this.start();
    }
    this.xAxis._onstart(e);
    this.yAxis._onstart(e);
};

Interface.Dragger.prototype._onend = function(e){
    if (this.end){
	this.end();
    }
    this.xAxis._onend(e);
    this.yAxis._onend(e);
    var position = this.element.position();
    this.xAxis.axisIndicator.css("top", position.top + this.halfSize);
    this.yAxis.axisIndicator.css("left", position.left + this.halfSize);
};





/**
 *
 * BUTTON
 *  
 */
Interface.Button = function(params){

    this.activeText = params.activeText || false;

    this.text = params.text || "Button";

    this.type = params.type || "moment";

    this.element = $("<div>", {
	    "class" : "Button",
	    "text" : this.text
	}).appendTo(params.parent || "#Content")
    .on("mousedown touchstart", this._start.bind(this));

    if (this.type === "moment"){
	this.element.on("mouseup touchend", this._end.bind(this));
    } else {
	this.element.addClass("Toggle");
    }

    /**
     *  the button state
     */
    this.active = false;

    /**
     *  callbacks
     */
    this.start = params.start;
    this.end = params.end;

    /**
     *  key presses
     */
    if (params.key){
	this.key = params.key;
	$(window).on("keydown", this._keydown.bind(this));
	if (this.type === "moment"){
	    $(window).on("keyup", this._keyup.bind(this));
	}
    }
};

Interface.Button.prototype._start = function(e){
    if (e){
	e.preventDefault();
    }
    if (!this.active){
	this.active = true;
	this.element.addClass("Active");
	if (this.activeText){
	    this.element.text(this.activeText);
	}
	if (this.start){
	    this.start();
	}
    } else if (this.type === "toggle" && this.active){
	this._end();
    }
};

Interface.Button.prototype._end = function(e){
    if (e){
	e.preventDefault();
    }
    ifport = function(){

	this.element = $("<div>", {
		"class" : "Transport",
	    }).appendTo("#Content");

	this.position = $("<div>", {
		"id" : "Position"
	    }).appendTo(this.element);

	this._boundLoop = this._loop.bind(this);
	this._loop();
    };

    Interface.Transport.prototype._loop = function(){
	setTimeout(this._boundLoop, 50);
	this.position.text(Tone.Transport.position);
    };
    
}
