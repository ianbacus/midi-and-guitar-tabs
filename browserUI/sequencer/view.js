let v_this = undefined;

class View
{
    constructor()
    {
        v_this = this;
        this.Maingrid = "#gridbox";
        this.GridArray = "#GridboxArray";
        this.PlayButton = "#PlayButton";
        this.previewObjs = ['cell', 'wire'];
        this.console = null;

        this.MaximumPitch = 77;

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
    	$(this.Maingrid)
            .mousemove(this.OnMouseMove)
            .mousedown(onMouseClickDown)
            .mouseup(onMouseClickUp)
            .mouseenter(onHoverBegin)
            .mouseleave(onHoverEnd);

        $(this.PlayButton).click(onButtonPress);
        $('input[type=radio]').change(this.OnRadioButton);

        $(document).keydown(onKeyUp);
        this.GridMouseHandler = onMouseMove;
        this.RadioButtonHandler = radioButtonHandler;

        $(this.Maingrid).bind('mousewheel DOMMouseScroll', onMouseScroll);

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
        var offset = $(v_this.Maingrid).offset();
        var gridSnap = v_this.gridSnap;

        cursorPosition.x = (Math.ceil((event.pageX - offset.left) / gridSnap)*gridSnap)-gridSnap;
        cursorPosition.y = (Math.ceil(((event.pageY - offset.top)) / gridSnap)*gridSnap)-gridSnap;

        v_this.GridMouseHandler(cursorPosition);
    }

    ConvertPitchToYIndex(pitch)
    {
        var pitchOffset = v_this.MaximumPitch - pitch;

        return v_this.gridSnap*pitchOffset;
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
        var mainDiv = $("#gridboxContainer")
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
        var mainDiv = $("#gridboxContainer")
		var currentScroll = mainDiv.scrollLeft();
        var newOffset = currentScroll+xOffset;
		var gridSnap = v_this.gridSnap;
        mainDiv.scrollLeft(newOffset);
		
		var newScrollPosition = mainDiv.scrollLeft();
		var actualOffset = newScrollPosition - currentScroll;
		if(actualOffset > 0)
			actualOffset = Math.ceil(actualOffset/gridSnap) * gridSnap;
		else if( actualOffset < 0)
			actualOffset = Math.floor(actualOffset/gridSnap) * gridSnap;
		
		return actualOffset;
    }

    CancelScroll()
    {
        var mainDiv = $("#gridboxContainer")
        mainDiv.stop();
    }

    SmoothScroll(xCoordinate, yCoordinate, milliseconds)
    {
        var mainDiv = $("#gridboxContainer");
        var gridWidth = mainDiv.width();
		
        var halfGridWidth = gridWidth/2;
		
        var xAdjustedCoordinate = xCoordinate - halfGridWidth;
		if(yCoordinate === undefined)
		{
			mainDiv.animate({scrollLeft:xAdjustedCoordinate},milliseconds);
		}
		else
		{
			var gridHeight = mainDiv.height();
			var halfGridheight = gridHeight/2;
			var yAdjustedCoordinate = yCoordinate - halfGridheight;
			
			console.log(yAdjustedCoordinate, yCoordinate, halfGridheight)
			mainDiv.animate({scrollTop:yAdjustedCoordinate, scrollLeft:xAdjustedCoordinate},milliseconds);
		}
    }

    GetGridboxThumbnail(instance, imageCallback, index)
    {
        var x = $("#gridboxContainer")[0]

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
                        var cWidth = canvasNode.width();
                        var cHeight = canvasNode.height();
                        context.drawImage(image, 0, 0, cWidth,cHeight);
                    }
                } catch (e) {
                    v_this.console.log(e);
                } finally {

                    nodeIndex++;
                }
            }
    }

    RenderSelectRectangle(selectPosition, cursorPosition)
    {
        var node = document.createElement('div');
        $(node).addClass("selectionRectangle");

        v_this.DeleteSelectRectangle();

        $(v_this.Maingrid).append(node);

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

        $(node).css({'top':top, 'left':left,
                 'border':'solid black 1px', 'position':'absolute',
                 'width':rect_width,'height':rect_height});
    }

	RenderKeys(modeArray)
	{
        var keyNoteClass = "keynote";
        var mainGrid = $(v_this.Maingrid);
		var mainGridWidth = mainGrid.width();
		var mainGridHeight = mainGrid.height();
        $(".keynote").remove();
		
		//Give the tonic more opacity than other notes
		modeArray.some(function(modeSlot)
		{
			var offsetY = 0;
			var repeatOffset = 0;
			console.log("Handling p",pitch);
			var pitch = modeSlot.Pitch;
			var noteOpacity = modeSlot.Opacity;
			
			while(offsetY < mainGridHeight)
			{
				var colorIndex = v_this.GetColorKey(pitch);
				offsetY = repeatOffset + v_this.ConvertPitchToYIndex(pitch);
				console.log("\t",repeatOffset, offsetY);
				if(offsetY < mainGridHeight)
				{
					var node = document.createElement('div');
					$(node).addClass(keyNoteClass);
					mainGrid.append(node);
					$(node).css({'background':colorIndex });
					$(node).css({'top':offsetY, 'left':0});
					$(node).css({"opacity":noteOpacity, "height":v_this.gridSnap,"width":mainGridWidth,"position":"absolute"});
				}
				repeatOffset += (12)*v_this.gridSnap;
			}
		});
	}

    RenderNotes(noteArray, color)
    {
        var gridNoteClass = "gridNote";
        var mainGrid = $(v_this.Maingrid);
        var borderCssString = 'solid '+color+' 1px'
		var initialNoteStartTimeTicks = 0;

        $("#gridboxContainer").css('border',borderCssString);
        $(".gridNote").remove();
		
		noteArray.forEach(function(note)
		{
			var noteWidth = note.Duration*v_this.gridSnap;
			var pitch = note.Pitch;
			var node = document.createElement('div');
			var noteOpacity = 1.0;
			var noteGridStartTimeTicks = note.StartTimeTicks - initialNoteStartTimeTicks;

			var offsetY = v_this.ConvertPitchToYIndex(pitch);
			var offsetX = v_this.ConvertTicksToXIndex(noteGridStartTimeTicks);
			var colorIndex = v_this.GetColorKey(pitch);

			$(node).addClass(gridNoteClass);
			mainGrid.append(node);
			$(node).css({'background':colorIndex, 'border': 'solid gray 1px'});

			if(note.IsSelected)
			{
				$(node).addClass("selected");
				noteOpacity = 0.5;
			}

			if(note.IsHighlighted)
			{
				$(node).css({'background':'white', 'border': 'solid gray 1px'});
			}
			$(node).css({'top':offsetY, 'left':offsetX});
			$(node).css({"opacity":noteOpacity, "height":v_this.gridSnap,"width":noteWidth,"position":"absolute"});

		});
	}
}
