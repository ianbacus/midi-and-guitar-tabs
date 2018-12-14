let v_this = undefined;

class View
{
    constructor()
    {
        v_this = this;

		this.Maingrid = null;
        this.GridboxContainer = null;//"#gridboxContainer";
        this.GridArray = "#GridboxArray";

        this.previewObjs = ['cell', 'wire'];
        this.console = null;

        this.MaximumPitch = 83;

        this.selectP = { x: 0, y: 0};

        this.gridSnap = 20;

        this.colorKey = [
            'red',    '#CC0099','yellow', '#669999',
            '#003399','#990000','#000099','#ff6600',
            '#660066','#006600','#669999','#003399'];

        this.pitchKey = [
            261.626,277.183,293.665,311.127,
            329.628,349.228,369.994,391.995,
            415.305,440.000,466.164,493.883,

            523.251,554.365,587.330,622.254,
            659.255,698.456,739.989,783.991,
            830.609,880.000,932.328,987.767,

            1046.50,1108.73,1174.66,1244.51,
            1318.51,1396.61,1479.98,1567.98];


    }

    Initialize(
        onKeyUp,
        onMouseScroll,
        onMouseMove, onMouseClickUp, onMouseClickDown,
        onHoverBegin, onHoverEnd,
        onButtonPress,
        radioButtonHandler)
    {

        this.Maingrid = $("#gridbox");
        this.GridboxContainer = $("#gridboxContainer");
        this.GridArray_ = $("#GridboxArray");

    	this.Maingrid
            .mousemove(this.OnMouseMove)
            .mousedown(onMouseClickDown)
            .mouseup(onMouseClickUp)
            .mouseenter(onHoverBegin)
            .mouseleave(onHoverEnd);

        //$(this.PlayButton).click(onButtonPress);
        $('input[type=radio]').change(this.OnRadioButton);

        $(document).keydown(onKeyUp);
        this.GridMouseHandler = onMouseMove;
        this.RadioButtonHandler = radioButtonHandler;

        this.Maingrid.bind('mousewheel DOMMouseScroll', onMouseScroll);

    }

    OnRadioButton(event)
    {
        var eventData = v_this.GetFormData();

        v_this.RadioButtonHandler(eventData);
    }

    GetFormData()
    {
        var filter = $('input:checked');
        var identifier = filter.parent().parent().parent().attr("id");
        var eventData = []

        filter.each(function()
        {
            var identifier = $(this).parent().parent().parent().attr("id");
            eventData.push({id:identifier, value:this.value});
        });

        return eventData;
    }

    OnMouseMove(event)
    {
        var cursorPosition = { x: -1, y: -1 };
        var offset = v_this.Maingrid.offset();
        var gridSnap = v_this.gridSnap;

        cursorPosition.x = (Math.ceil((event.pageX - offset.left) / gridSnap)*gridSnap)-gridSnap;
        cursorPosition.y = (Math.ceil(((event.pageY - offset.top)) / gridSnap)*gridSnap)-gridSnap;

        v_this.GridMouseHandler(cursorPosition);
    }

    ConvertPitchToYIndex(pitch)
    {
        var pitchOffset = v_this.MaximumPitch - pitch;
		var mainGridHeight = v_this.Maingrid.height();

        var result = (v_this.gridSnap*pitchOffset) % mainGridHeight;
        return result;
    }

    ConvertTicksToXIndex(ticks)
    {
        return v_this.gridSnap*ticks;
    }

    ConvertYIndexToPitch(yIndex)
    {
        return v_this.MaximumPitch - (yIndex/v_this.gridSnap);
    }

    ConvertXIndexToTicks(xIndex)
    {
        return xIndex/v_this.gridSnap;
    }

    GetColorKey(pitch)
    {
        var colorIndex = pitch % 12;
        return v_this.colorKey[colorIndex];
    }

    DeleteSelectRectangle()
    {
        $(".selectionRectangle").remove();
    }

	ScrollVertical(yOffset)
	{
        var mainDiv = this.GridboxContainer;

		var currentScroll = mainDiv.scrollTop();
        var newOffset = currentScroll+yOffset;
		var gridSnap = v_this.gridSnap;
        mainDiv.scrollTop(newOffset);

		var newScrollPosition = mainDiv.scrollTop();
		var actualOffset = newScrollPosition - currentScroll;
		if(actualOffset > 0)
			actualOffset = Math.ceil(actualOffset/gridSnap) * gridSnap;
		else if( actualOffset < 0)
			actualOffset = Math.floor(actualOffset/gridSnap) * gridSnap;

		return actualOffset;
	}

    ScrollHorizontal(xOffset)
    {
        var mainDiv = this.GridboxContainer;

		var currentScroll = mainDiv.scrollLeft();
        var newOffset = currentScroll+xOffset;
		var gridSnap = v_this.gridSnap;
        mainDiv.scrollLeft(newOffset);

		var newScrollPosition = mainDiv.scrollLeft();
		var actualOffset = newScrollPosition - currentScroll;
		if(actualOffset > 0)
		{
			actualOffset = Math.ceil(actualOffset/gridSnap) * gridSnap;
		}
		else if( actualOffset < 0)
		{
			actualOffset = Math.floor(actualOffset/gridSnap) * gridSnap;
		}

		return actualOffset;
    }

    CancelScroll()
    {
        this.GridboxContainer.stop();
        this.TickCount = 0;
    }

    ScrollDelegate()
    {
        if(this.TickCount > 0)
        {
            var x = this.GridboxContainer.scrollLeft();
            var y = this.GridboxContainer.scrollTop();
            this.GridboxContainer.scrollLeft(x+10,y); // horizontal and vertical scroll increments
            this.TickCount--;
            this.PendingTimeout = setTimeout(
                $.proxy(this.ScrollDelegate, this), this.MillisecondsPerTick);
        }
    }

    SmoothScroll(startx, xCoordinate, yCoordinate, millisecondsPerTick)
    {
        var mainDiv = this.GridboxContainer;
        var gridWidth = mainDiv.width();

        var halfGridWidth = gridWidth/2;

        var xAdjustedCoordinate = xCoordinate - halfGridWidth;
        var currentScrollLeft = mainDiv.scrollLeft();

        clearTimeout(this.PendingTimeout);
		if(yCoordinate === undefined)
		{
			//mainDiv.animate({scrollLeft:xAdjustedCoordinate},milliseconds);

            v_this.MillisecondsPerTick = millisecondsPerTick/2;
            v_this.TickCount = (xAdjustedCoordinate - startx)/10;
            v_this.ScrollDelegate();

		}
		else
		{
			var gridHeight = mainDiv.height();
			var halfGridheight = gridHeight/2;
			var yAdjustedCoordinate = yCoordinate - halfGridheight;

			mainDiv.animate({scrollTop:yAdjustedCoordinate, scrollLeft:xAdjustedCoordinate},500);
		}
    }

    GetGridboxThumbnail(instance, imageCallback, index)
    {
        var x = this.GridboxContainer[0]

        html2canvas(x, {logging:false}).then(function(img)
        {
            var eventData = {Image: img, GridIndex: index};
            imageCallback.call(instance, eventData)
        });
    }

    RenderGridArray(gridImages, selectedIndex)
    {
        var numberOfEntries = gridImages.length;
        var domGridArray = $(v_this.GridArray);
        domGridArray.empty();
        var nodeIndex = 0;

        while(nodeIndex < numberOfEntries)
        {
            var image = gridImages[nodeIndex];
            var canvasNode = $('<canvas/>');
            domGridArray.append(canvasNode);

            if(nodeIndex == selectedIndex)
            {
                canvasNode.css({'border':'solid purple 3px'});
            }
            else {
                canvasNode.css({'border':'solid black 1px'});
            }

            try {
                if(image != null)
                {
                    var dataurl = image.toDataURL()
                    var context = canvasNode[0].getContext("2d");
                    var cWidth = canvasNode.width()*2;
                    var cHeight = canvasNode.height()*2;
                    context.drawImage(image, 0, 0, cWidth,cHeight);
                }
            } catch (e) {

            } finally {

                nodeIndex++;
            }
        }
    }

    RenderSelectRectangle(selectPosition, cursorPosition)
    {
        var node = document.createElement('div');
        $(node).addClass("selectionRectangle");

        var x_offset = 0;
        var y_offset = 0;

        var rect_width = (cursorPosition.x - selectPosition.x);
        var rect_height = (cursorPosition.y - selectPosition.y);

        if(rect_width < 0)
        {
            rect_width *= -1;
            x_offset = rect_width;
        }

        if(rect_height < 0)
        {
            rect_height *= -1;
            y_offset = rect_height;
        }

        var top = selectPosition.y-y_offset;
        var left = selectPosition.x-x_offset;

        $(node).css({
			'top':top,
			'left':left,
			'border':'solid black 1px',
			'position':'absolute',
			'width':rect_width,
			'height':rect_height
		});

		v_this.DeleteSelectRectangle();
        v_this.Maingrid.append(node);
    }

	RenderKeys(modeArray)
	{
        var keyNoteClass = "keynote";
		var mainGridWidth = v_this.Maingrid.width();
		var mainGridHeight = v_this.Maingrid.height();
        var isTonicNote = true;
        $(".keynote").remove();

        console.log("H",mainGridHeight);

        function functionRenderKeyRow(offsetY, colorIndex, noteOpacity, isTonicNote)
        {
            var node = document.createElement('div');

            $(node).addClass(keyNoteClass);
            $(node).css({
				'background':colorIndex,
				'top':offsetY,
				'left':0,
				"opacity":noteOpacity,
				"height":v_this.gridSnap,
				"width":mainGridWidth,
				"position":"absolute"});

            if(isTonicNote)
            {
                $(node).css({"border-bottom":'solid black 2px'});
            }

            v_this.Maingrid.append(node);
        }

		//Give the tonic more opacity than other notes
		modeArray.some(function(modeSlot)
		{
			const pitch = modeSlot.Pitch;
			const colorIndex = v_this.GetColorKey(pitch);
            const incrementOffset = 12 * v_this.gridSnap;
			const noteOpacity = modeSlot.Opacity;
            const keyOffsetY = v_this.ConvertPitchToYIndex(pitch);

            var lowerOffset = keyOffsetY-incrementOffset;
            var upperOffset = keyOffsetY+incrementOffset;

            functionRenderKeyRow(keyOffsetY, colorIndex, noteOpacity, isTonicNote);
            while(lowerOffset >= 0)
            {
                functionRenderKeyRow(lowerOffset, colorIndex, noteOpacity, isTonicNote);
                lowerOffset -= incrementOffset;
            }
            while (upperOffset <= mainGridHeight)
            {
                functionRenderKeyRow(upperOffset, colorIndex, noteOpacity, isTonicNote);
                upperOffset += incrementOffset;
            }

            isTonicNote = false;
		});
	}

    RenderNotes(noteArray, color)
    {
        var gridNoteClass = "gridNote";
        var mainGrid = v_this.Maingrid;

        var borderCssString = 'solid '+color+' 1px'
		var initialNoteStartTimeTicks = 0;

        v_this.GridboxContainer.css('border',borderCssString);
        $(".gridNote").remove();

		noteArray.forEach(function(note)
		{
			var noteWidth = note.Duration*v_this.gridSnap;
			var pitch = note.Pitch;
			var noteOpacity = 1.0;

			var noteGridStartTimeTicks = note.StartTimeTicks - initialNoteStartTimeTicks;
			var offsetY = v_this.ConvertPitchToYIndex(pitch);
			var offsetX = v_this.ConvertTicksToXIndex(noteGridStartTimeTicks);
			var colorIndex = v_this.GetColorKey(pitch);

			var node = document.createElement('div');

			if(note.IsHighlighted)
			{
				colorIndex = 'white';
			}

			if(note.IsSelected)
			{
				noteOpacity = 0.5;
			}

			$(node).addClass(gridNoteClass);
			$(node).css({
				'background':colorIndex,
				'border': 'solid gray 1px',
				'top':offsetY,
				'left':offsetX,
				'opacity':noteOpacity,
				'height':v_this.gridSnap,
				'width':noteWidth,
				'position':'absolute'
			});

			mainGrid.append(node);

		});
	}
}
