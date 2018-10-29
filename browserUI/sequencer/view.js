class View
{
    constructor()
    {
        this.Maingrid = "#gridbox";
        this.PlayButton = "#PlayButton";
        this.emColors = ['orange','blue','green'];
        this.previewObjs = ['cell', 'wire'];

        this.MaximumPitch = 72;
        this.MinimumPitch = 54;

        this.cursorP = { x: -1, y: -1 };
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
        onMouseMove,
        onHoverBegin,
        onHoverEnd,
        onKeyPress,
        onButtonPress)
    {
    	$(this.Maingrid).mousemove(onMouseMove);
    	$(this.Maingrid).hover(onHoverBegin,onHoverEnd);
        $(this.PlayButton).click(onButtonPress);
        $(document).keyup(onKeyUp);
    }

    ConvertPitchToYIndex(pitch)
    {
        var pitchRange = this.MaximumPitch - this.MinimumPitch;
        pitchOffset = this.MaximumPitch - pitch;

        return gridSnap*pitchOffset;
    }

    ConvertTicksToXIndex(ticks)
    {
        return gridSnap*ticks;
    }

    GetColorKey(pitch)
    {
        var colorIndex = pitch % 12;

        return colorKey[colorIndex];
    }

    RenderNotes(noteArray)
    {
        if(noteArray.length > 0)
        {
            initialNoteStartTimeTicks = noteArray[0].StartTimeTicks;
            noteArray.forEach(function(note)
            {
                var noteWidth = note.Duration;
                var node = document.createElement('div');
                var noteOpacity = 1.0;
                var noteGridStartTimeTicks = note.StartTimeTicks - initialNoteStartTimeTicks;

                var offsetY = ConvertPitchToYIndex(pitch);
                var offsetX = ConvertTicksToXIndex(noteGridStartTimeTicks);
                var colorIndex = GetColorKey(pitch);

                if(note.IsSelected)
                {
                    $(node).addClass("selected");
                    noteOpacity = 0.5;
                }

                $(this.maingrid).append(node);
                $(node).css({'top':offsetY, 'left':offsetX});
                $(node).css({"opacity":noteOpacity, "height":snapY,"width":noteWidth,"position":"absolute"});
                $(node).css({'background':colorIndex});
            });
        }
    }
}
