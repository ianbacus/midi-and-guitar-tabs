let v_this = undefined;

class View
{
    constructor()
    {
        v_this = this;
        v_this.Maingrid = "#gridbox";
        v_this.GridArray = "#GridboxArray";
        v_this.PlayButton = "#PlayButton";
        v_this.previewObjs = ['cell', 'wire'];

        v_this.MaximumPitch = 72;
        v_this.MinimumPitch = 54;

        v_this.selectP = { x: 0, y: 0};

        v_this.gridSnap = 20;

        v_this.colorKey = [
            'red',    '#CC0099','yellow', '#669999',
            '#003399','#990000','#000099','#ff6600',
            '#660066','#006600','#669999','#003399'];

        v_this.pitchKey = [
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
        onMouseMove, onMouseClickUp, onMouseClickDown,
        onHoverBegin, onHoverEnd,
        onButtonPress)
    {
    	$(v_this.Maingrid)
            .mousemove(v_this.OnMouseMove)
            .mousedown(onMouseClickDown)
            .mouseup(onMouseClickUp)
            .hover(onHoverBegin,onHoverEnd);

        $(v_this.PlayButton).click(onButtonPress);

        $(document).keydown(onKeyUp);
        v_this.GridMouseHandler = onMouseMove;
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

    RenderGridArray(numberOfEntries, index)
    {

        console.log("Rendering preview", numberOfEntries, index);
        var domGridArray = $(v_this.GridArray);
        domGridArray.empty();
        var nodeIndex = 0;

        while(nodeIndex < numberOfEntries)
        {
            var entryNode = "<li> slice " + nodeIndex + "</li>";
            //TODO: use image of entry
            if(nodeIndex === index)
            {
                entryNode = "<li style=\"border: 1px solid black\"> slice " + nodeIndex + "</li>";
            }
            domGridArray.append(entryNode);
            nodeIndex++;
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

        mainGrid.css({'border':'solid'+color+' 1px'});
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

                if(note.IsSelected)
                {
                    $(node).addClass("selected");
                    noteOpacity = 0.5;
                }

                $(node).addClass(gridNoteClass);
                mainGrid.append(node);
                $(node).css({'top':offsetY, 'left':offsetX});
                $(node).css({"opacity":noteOpacity, "height":v_this.gridSnap,"width":noteWidth,"position":"absolute"});
                $(node).css({'background':colorIndex});
            });
        }
    }
}
