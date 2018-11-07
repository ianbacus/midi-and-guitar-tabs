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

        this.MaximumPitch = 72;
        this.MinimumPitch = 54;

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
        var pitchRange = v_this.MaximumPitch - v_this.MinimumPitch;
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

    ScrollHorizontal(xOffset)
    {
        var mainDiv = $("#gridboxContainer")
        var newOffset = mainDiv.scrollLeft()+xOffset
        mainDiv.scrollLeft(newOffset);
    }

    CancelScroll()
    {
        var mainDiv = $("#gridboxContainer")
        mainDiv.stop();
    }

    SmoothScroll(xCoordinate, milliseconds)
    {
        var mainDiv = $("#gridboxContainer");
        var gridWidth = mainDiv.width();
        var halfGridWidth = gridWidth/2;
        var xAdjustedCoordinate = xCoordinate - halfGridWidth;
        //var newOffset = mainDiv.scrollLeft()+xOffset

        mainDiv.animate({scrollLeft:xAdjustedCoordinate},milliseconds);
        console.log(milliseconds,"TICKS");

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


    RenderNotes(noteArray, color)
    {
        var gridNoteClass = "gridNote";
        var mainGrid = $(v_this.Maingrid);
        var borderCssString = 'solid '+color+' 1px'

        $("#gridboxContainer").css('border',borderCssString);

        $(".gridNote").remove();


        if(noteArray.length > 0)
        {
            var initialNoteStartTimeTicks = 0;
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
}
