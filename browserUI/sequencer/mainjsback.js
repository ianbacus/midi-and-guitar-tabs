$( function() {

	var synth = T("OscGen", {
    wave: "cos",
    //fami, saw, tri, pulse, konami, cos, sin
    mul: 0.5
	}).play();

	/* variable init */
	{
    //Associate a color with each of the 12 pitches
    var colorKey = ['red','#CC0099','yellow','#669999','#003399','#990000',
        '#000099','#ff6600','#660066','#006600','#669999','#003399'];
    var pitchKey = [261.626,277.183,293.665,311.127,
            329.628,349.228,369.994,391.995,
            415.305,440.000,466.164,493.883,

            523.251,554.365,587.330,622.254,
            659.255,698.456,739.989,783.991,
            830.609,880.000,932.328,987.767,

            1046.50,1108.73,1174.66,1244.51,
            1318.51,1396.61,1479.98,1567.98];

    var selectEnum = {
    	BEGIN: 1,
    	SELECTED: 2,
    	DRAG: 3
    };

    var editEnum = {
    	PREVIEW: 1,
    	DRAG: 2,
    	OFF: 3
    }

    var em = {
    	EDIT: 0,
    	SELECT: 1,
    	DELETE: 2
    }
    var maingrid = "#gridbox";
    var playButton = "#PlayMusic";
    var emColors = ['orange','blue','green'];
    var previewObjs = ['cell', 'wire'];
    var currentObjs = [];
    var mainMode = em.EDIT;
    var editState = editEnum.PREVIEW;
    var selectState = selectEnum.BEGIN;
    var cursorP = { x: -1, y: -1 };
    var selectP = { x: 0, y: 0};
    var currentObj = null;
    var snapX = 20,snapY=20;
    var widthMode = 40;
    var claimedSpots = [];
	}

	/* analysis suite */
	{
    function DrawBeats(meter)
    {
        //Draw black lines
    	var gridWidth = parseInt($(maingrid).css('width'),10)/snapX;
    	var divisions = gridWidth/meter;
    	var currentLeftOffset = 0;
    	for(var i=0;i<divisions;i++)
    	{
            currentLeftOffset += meter*snapX;
            var node = document.createElement('div');
            $(maingrid).append(node);

            $(node).css({'position':'absolute','top':0,'left':currentLeftOffset, 'height':'100%', 'width':1,'background':'black'});
    	}
    }

    function getNoteArray()
    {
    	var noteArray = [];

    	$(".node").each(function()
    	{
            var pitch = 72-(parseInt($(this).css('top'),10)/snapY);
            var delta = parseInt($(this).css('left'),10)/snapX;
            var duration = parseInt($(this).css('width'),10)/snapX;
            var entry = {'pitch':pitch,'delta':delta,'duration':duration};
            noteArray.push(entry);
    	});

        noteArray.sort(function(note1,note2)
        {
            var n1d = note1['delta'];
            var n2d = note2['delta'];

            if(n1d < n2d) return -1;
            else if(n1d > n2d) return 1;
            else return 0;

        });

    	return noteArray;
    }

    function getIntervalArray()
    {
    	var noteArray = getNoteArray();
    	var temporalDict = {};
    	for(var i =0; i<noteArray.length;i++)
    	{
            noteEntry = noteArray[i];
            delta = noteEntry.delta;
            if(!temporalDict[delta])
            	temporalDict[delta] = [];
            temporalDict[delta].push(noteEntry);
    	}
    	temporaryDict = {};
    	for(var delta in temporalDict)
    	{
        chunk = temporalDict[delta];
        delta = parseInt(delta,10);
        for(var j=0; j<chunk.length;j++)
        {
        	//for each note in the chunk at this moment
        	note = chunk[j];
        	duration = note.duration;

        	while(duration-- > 0)
        	{

            tempDelta = duration+delta;
            if(temporalDict[tempDelta])
            {
            	if(!temporaryDict[tempDelta])
                temporaryDict[tempDelta] = [];
            	temporaryDict[tempDelta].push(note);
            }
        	}


        }
    	}
    	//merged dict of temporal events
    	//console.log('temporary: '+JSON.stringify(temporaryDict));
    	//console.log('main: '+JSON.stringify(temporalDict));

    	for(var delta in temporaryDict)
    	{
        temporaryNoteChunk = temporaryDict[delta];
        temporalNoteChunk = temporalDict[delta];

        if(!temporalDict[delta])
        	temporalDict[delta] = temporaryNoteChunk;
        else
        {
        	for(var i=0;i<temporaryNoteChunk.length;i++)
        	{
            var noteToAdd = temporaryNoteChunk[i]
            var pitchAlreadyPresent = false;
            for(var j=0;j<temporalNoteChunk.length;j++)
            {

            	if(temporalNoteChunk[j].pitch == noteToAdd.pitch)
                pitchAlreadyPresent = true;
            }
            if(!pitchAlreadyPresent)
            {
            	temporalDict[delta].push(noteToAdd);

            }
        	}
        }
    	}
    	var intervals = []
    	console.log(JSON.stringify(temporalDict));
    	for(var delta in temporalDict)
    	{
        //If more than 2 entries, just ignore it.. use outer eventually
        if(temporalDict[delta].length == 2)
        	intervals.push(Math.abs(temporalDict[delta][0].pitch - temporalDict[delta][1].pitch));

    	}
    	console.log(intervals);
    	return intervals;

    }
    function CheckMelody(track)
    {

    }

    function CheckCounterpoint()
    {
    	//Fux's fundamental rules:
    	//Any interval -> Perfect: contrary/oblique

    	//Species:
    	//Strong beat: consonance
    	//1st: 1:1 notes
    	//2nd
    	//3rd
    	//4th

    	//General:
    	//Balance leaps of a fifth or more by going down afterwards
    	//Don't move stepwise into a leap in the same direction
    	//

    	//getNoteArray();
    	var perfectIntervals = new Set([0, 7, 12]); //unison, fifth, octave
    	var imperfectIntervals = new Set([3,4,8,9]); //m3,M3,m6,M6
    	var dissonantIntervals = new Set([1,2,5,6,10,11]);
    	var intervals = getIntervalArray();
    	var directions = getDirectionsArray();
    	var beats = getBeatsArray();
    	var previousStrongBeatInterval = null;
    	for(var i=0; i<intervals.length-1;i++)
    	{
        var oppositeMotion = (directions[i] == 'contrary') || (directions[i] == 'oblique');
        var directMotionToPerfectInterval = (perfectIntervals.has(intervals[i+1]) && !oppositeMotion);
        var parallelPerfectIntervals = perfectIntervals.has(intervals[i+1]) && perfectIntervals.has(intervals[i])

        if(beats[i] == 'strong')
        {
        	var directConsonantBeats = perfectIntervals.has(previousStrongBeatInterval) && perfectIntervals.has(intervals[i]);
        	var strongbeatDissonance = dissonantIntervals.has(intervals[i]);
        	previousStrongBeatInterval = intervals[i];
        }

        if(directMotionToPerfectInterval || strongbeatDissonance || parallelPerfectIntervals)
        {
        	//bad
        }

    	}

    }
    DrawBeats(4);
	}

	/* general utilities */
	{
    function checkEncapsulate(rect1,rect2)
    {
    	//See if Rect1 bounds rect2
    	var R1L=parseInt(rect1.css('left'),10),R1T=parseInt(rect1.css('top'),10);
    	R1R=(R1L+parseInt(rect1.css('width'),10)),R1B=(R1T+parseInt(rect1.css('height'),10));

    	var R2L=parseInt(rect2.css('left'),10),R2T=parseInt(rect2.css('top'),10);
    	R2R=(R2L+parseInt(rect2.css('width'),10)),R2B=(R2T+parseInt(rect2.css('height'),10));

    	var xcond = (R1L <= R2L) && (R1R >= R2R);
    	var ycond = (R1T <= R2T) && (R1B >= R2B);

    	return (xcond && ycond);
    }

    function deleteCurrentObj()
    {
    	if(currentObj)
    	{
        $(currentObj).remove();
        currentObj=null;
    	}
    	else console.log("No obj to delete");
    }

    function wasClicked(selector)
    {
    	var cursorRect = $('<div>');
    	$(cursorRect).css({'top':cursorP.y, 'left':cursorP.x, 'height':1, 'width':1});
    	var overlapping = false;
    	$(selector).each(function(){
        if(checkEncapsulate($(this),$(cursorRect)))
        {
        	overlapping = true;
        }
    	});
    	cursorRect = null;
    	return overlapping;
    }

    function getNoteIndex(coordPoint,mod)
    {
    	var ix = coordPoint/20;
    	ix = ix % mod;
    	return (mod-1) - ix;
    }

	}

	/* extra stuff like copy and delete */
	{

    function setDeleteMode()
    {
    	deleteCurrentObj();
    	bindDeletion($(".node"));
    	$(maingrid).css({'border':'solid'+emColors[mainMode]+' 1px'});
    }

    function exitDeleteMode()
    {
    	unbindDeletion($(".node"));
    }

    function setCopyMode()
    {


    }

    function exitCopyMode()
    {

    }
	}

	/*  Select functions */
	{
        function makeSelected(selector)
        {
            //Mark each of the objects defined by the selector as selected
        	$(selector).each(function(){
                if(!($(this).hasClass('selected')))
                	$(this).addClass('selected');
                $(this).data( "init-position", { top: $(this).css('top'),
                                	left: $(this).css('left') } );
                $(this).css({'opacity':0.7});
        	});
        }

        function unSelect(selector)
        {
        	$(selector).removeData("init-position");
        	$(selector).css({'opacity':1});
        	$(selector).removeClass('selected');
        }


        function createSelectRectangle()
        {
        	if(!currentObj)
        	{
                var node = document.createElement('div');
                $(maingrid).append(node);

                selectP.x = cursorP.x;
                selectP.y = cursorP.y;
                    	console.log(cursorP.y,cursorP.x,selectP.x,selectP.y);
                $(node).css({'top':cursorP.y, 'left':cursorP.x,
                    	 'border':'solid black 1px', 'position':'absolute',
                    	 'width':'0px','height':'0px'});
                currentObj = $(node);
        	}
        }

        function bindDeletion(selector)
        {
        	$(selector).mouseup(function()
        	{
            $(this).remove();
        	});
        }

        function unbindDeletion(selector)
        {
        	$(selector).unbind("mouseup");
        }

        function unbindSelection(selector)
        {

        }

        function selectClickDownCallback()
        {
        	switch(selectState)
        	{
            case selectEnum.SELECTED:
            	console.log("cd selected");

            	if(!wasClicked($(".selected")) )
            	{
                    selectState = selectEnum.BEGIN;
            	}
            	else
            	{
                    selectState = selectEnum.DRAG;
                    makeSelected($('.selected'));
                    selectP.x = cursorP.x;
                    selectP.y = cursorP.y;
                    break;
            	}
            case selectEnum.BEGIN:
            	console.log("cd begin");
            	unSelect($('.node'))
            	createSelectRectangle();
            	break;
            case selectEnum.DRAG:
            	console.log("cd drag");
            	selectState = selectEnum.SELECTED;
            	break;
        	}

        }

        function selectClickUpCallback()
        {
        	console.log('clickup',selectState);
            switch(selectState)
            {
            	case selectEnum.BEGIN:
                console.log("cu begin");
                selectState = selectEnum.BEGIN;
                $(".node").each(function()
                {
                	if(checkEncapsulate($(currentObj),$(this)))
                	{
                    console.log("detected selection");
                    makeSelected($(this));
                    selectState = selectEnum.SELECTED;
                	}
                });
                deleteCurrentObj(); //delete selection rectangle

                break;
            	case selectEnum.SELECTED:
                console.log("cu selected");
                break;
            	case selectEnum.DRAG:
                console.log("cu drag");
                break;
            }

        }

        function selectCursorCallback(event)
        {
        	if (currentObj) {
            var y = (cursorP.y)-selectP.y;
            var x = (cursorP.x)-selectP.x;
            //currentObj.css({'height':cursorP.y-topOffset, 'width':cursorP.x-leftOffset});
            //console.log(cursorP.y,cursorP.x,selectP.x,selectP.y);

            if(x < 0) currentObj.css({'width':-1-x,'left':cursorP.x});
            else if(x==0) currentObj.css({'width':0,'left':selectP.x});
            else currentObj.css({'width':x-1});

            if(y < 0) currentObj.css({'height':-1-y,'top':cursorP.y});
            else if(y==0) currentObj.css({'height':0,'top':selectP.y});
            else currentObj.css({'height':y-1});

        	}
        	else if(selectState == selectEnum.DRAG)
        	{
            var y = (cursorP.y)-selectP.y;
            var x = (cursorP.x)-selectP.x;
            $(".selected").each(function()
            {
            	var oldY = parseInt($(this).css('top'),10)
            	var thisY = (parseInt($(this).data( "init-position").top,10)+y);
            	var thisX = (parseInt($(this).data( "init-position").left,10)+x);
            	console.log(oldY,' and '+thisY);

            	if(oldY != thisY)
            	{

                var pitchIndex = getNoteIndex(thisY,24);
                synth.noteOnWithFreq(pitchKey[pitchIndex], 200);
            	}
            	var colorIndex = getNoteIndex(thisY,12);
            	$(this).css({'top':thisY, 'left':thisX ,'background':colorKey[colorIndex]});

            });
        	}
        }


        function setSelectMode()
        {
        	console.log("Set SM"+mainMode);
        	deleteCurrentObj();

        	selectState = selectEnum.BEGIN;
        	$(maingrid).mousedown(selectClickDownCallback)
                 .mouseup(selectClickUpCallback)
                 .mousemove(selectCursorCallback)
                 .css({'border':'solid'+emColors[mainMode]+' 1px'});
                 //.hover(selectHoverOnCallback,selectHoverOffCallback)
        }

        function exitSelectMode()
        {
        	unSelect($(".node"));
        	$(maingrid).off("mousedown",selectClickDownCallback)
                 .off("mouseup",selectClickUpCallback)
                 .off("mousemove",selectCursorCallback);
                 //.off("hover",selectHoverOnCallback,selectHoverOffCallback)
        }

    }

	/* Edit functions */
	{

    function createPreview()
    {
    	if(!currentObj)
    	{
        var node = document.createElement('div');
        $(maingrid).append(node);
        $(node).addClass("selected");
        $(node).css({'top':cursorP.y, 'left':cursorP.x});
        $(node).css({"opacity":0.5, "height":snapY,"width":widthMode,"position":"absolute"});
        var colorIndex = getNoteIndex(cursorP.y,12);///snapY)%12);
        $(node).css({'background':colorKey[colorIndex]});
        currentObj = $(node);
    	}
    }

    function bindDragging(selector)
    {
    	$(selector).mousedown(function()
    	{
        if(!currentObj)
        {
        	currentObj = $(this);
        }
    	}).mouseup(function()
    	{
            currentObj = null;
            //claimedSpots.push({'position':[cursorP.x,cursorP.y],'width':widthMode});
    	});
    }

    function unbindDragging(selector)
    {
    	$(selector).unbind("mousedown");
    	$(selector).unbind("mouseup");
    }

    function editClickDownCallback()
    {
    	//Instantiate object, create node
    	var clickednode = wasClicked($(".node"));

    	if(currentObj)
    	{
            $(currentObj).css({'opacity':'1'});
            $(currentObj).addClass('node');
            $(currentObj).removeClass('selected');
            bindDragging(currentObj);
    	}
    }

    function editClickUpCallback()
    {
    	createPreview();
    	var pitchIndex = getNoteIndex(cursorP.y,24);
    	synth.noteOnWithFreq(pitchKey[pitchIndex], 100);
    	getIntervalArray();
    }

    function editHoverOnCallback() {
    	//When cursor enters the gridbox

    	if(editState == editEnum.PREVIEW)
    	{
            createPreview();
    	}
    }


    function editCursorCallback(event)
    {
    	//console.log(currentObj);

    	//When dragging, change color of preview
    	if(currentObj)
    	{
        var colorIndex = getNoteIndex(cursorP.y,12);
        currentObj.css({'top':cursorP.y, 'left':cursorP.x,'background':colorKey[colorIndex]});

    	}
    }

    function setEditMode()
    {
    	console.log("Set EM"+emColors[mainMode]);
    	editState = editEnum.PREVIEW;
    	deleteCurrentObj();
    	bindDragging($(".node"));

    	$(maingrid).mousedown(editClickDownCallback)
             .mouseup(editClickUpCallback)
             .hover(editHoverOnCallback,function(){})
             .mousemove(editCursorCallback)
             .css({'border':'solid'+emColors[mainMode]+' 1px'});
    	createPreview();
    }

    function exitEditMode()
    {
    	editState = editEnum.OFF;
    	unbindDragging($(".node"));
    	$(maingrid).off("mousedown",editClickDownCallback)
             .off("mouseup",editClickUpCallback)
             .off("hover",editHoverOnCallback)
             .off("mousemove",editCursorCallback);
    	console.log("exit edit mode");

    }
    }

    /* Main functions */
    {
        var noteIndex = 0;
        var noteArray = []
        var pendingTimeout;

        function playChord()
        {
            var lastDelta = 0;

            if(noteIndex < noteArray.length)
            {
                var pitchIndex = noteArray[noteIndex]['pitch'] - 54;
                var duration = noteArray[noteIndex]['duration'];
                lastDelta = noteArray[noteIndex]['delta'];

                synth.noteOnWithFreq(pitchKey[pitchIndex], 200);
                noteIndex++;
            }

            if(noteIndex < noteArray.length)
            {
                var millisecondsPerTick = 100;
                var relativeDelta = noteArray[noteIndex]['delta'] - lastDelta;
                var delta = relativeDelta*millisecondsPerTick;
                console.log(delta);
                pendingTimeout = setTimeout(playChord, delta)
            }
        }

        function playCallback(event)
        {
            noteArray = getNoteArray();
            noteIndex = 0;
            clearTimeout(pendingTimeout);
            playChord();
        }

        function mainCursorCallback(event)
        {
        	//Move preview object to follow cursor, make it snappable
        	var offset = $(maingrid).offset();
        	cursorP.x = (Math.ceil((event.pageX-offset.left) / snapX)*snapX)-snapX;
        	cursorP.y = (Math.ceil(((event.pageY-offset.top)) / snapY)*snapY)-snapY;
        }
        function mainHoverOnCallback(event)
        {
        	$(this).css({'border':'solid '+emColors[mainMode]+' 1px'})
        }

        function mainHoverOffCallback(event)
        {
        	$(this).css({'border':'solid black 1px'});
        	deleteCurrentObj();
        }

        function changemainMode(nextMode)
        	{
        	if(mainMode == nextMode) return;
        	switch(mainMode)
        	{
            case em.EDIT:
            	exitEditMode();
            	break;
            case em.SELECT:
            	exitSelectMode();
            	break;
            case em.DELETE:
            	exitDeleteMode();
            	break;
        	}
        	mainMode = nextMode;
        	switch(nextMode)
        	{
            case em.EDIT:
            	console.log("edit mode");
            	setEditMode();
            	break;
            case em.SELECT:
            	console.log("select mode");
            	setSelectMode();
            	break;
            case em.DELETE:
            	console.log("delete mode");
            	setDeleteMode();
            	break;

        	}

        }

        $(document).keyup(function(e)
        {
        	var nextMode = mainMode;
        	switch(e.keyCode)
        	{
                case 32: //spacebar
                playCallback(null);
                e.preventDefault();
                break;

            case 67: //"c" key"
            	console.log(getNoteArray());

            	break;
            case 68: //"d" key
            	if((selectState == selectEnum.SELECTED) || (selectState == selectEnum.DRAG))
            	{
                selectState = selectEnum.BEGIN;
                $(".selected").remove();
            	}
            	else
            	{
                nextMode = em.DELETE;
            	}
            	break;

            case 81: //"q" key
            	console.log("cancel");
            	unSelect($(".node"));
            	if(mainMode == em.EDIT)
                editState = editEnum.DRAG;
            	deleteCurrentObj();
            	break;
            case 87: //"w" key
            	if(widthMode > snapX)
                widthMode = widthMode/2;
            	//if(currentObj)
            	$('.selected').each(function(){
                $(this).css({'width':widthMode});
            	});
            	break;
            case 69: //"e" key
            	if(widthMode < 32*snapX)
                widthMode = widthMode*2;
            	//if(currentObj)
            	$('.selected').each(function(){
                $(this).css({'width':widthMode});
            	});
            	break;
            case 88: //"x" key"
            	nextMode = em.SELECT;
            	break;
            case 90: //"z" key"
            	nextMode = em.EDIT;
            	break;
        	}
        	changemainMode(nextMode);
        });
	}


	$(maingrid).mousemove(mainCursorCallback);
	$(maingrid).hover(mainHoverOnCallback,mainHoverOffCallback);
    $(playButton).click(playCallback)
    //$(playButton).
	setEditMode();
} );
