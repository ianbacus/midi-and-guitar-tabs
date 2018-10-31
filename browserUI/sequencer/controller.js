var sm = {
    BEGIN: 1,
    SELECTED: 2,
    DRAG: 3
};


var editModeEnumeration = {
	EDIT: 0,
	SELECT: 1,
	DELETE: 2
}

let c_this = undefined;


class SelectModeController
{
    constructor()
    {
        //SelectMode = sm.BEGIN;
    }

    OnMouseMove(event)
    {
        //case 1: render preview
        //case 2: move selected items
        //case 3:
    }
};

class EditModeController
{
    constructor()
    {
        //EditMode = em.BEGIN;
    }

    OnMouseMove(event)
    {
        //case 1: render preview
        //case 2: move selected items
        //case 3:
    }
};

class Controller
{
    constructor(view, model)
    {
        c_this = this;
        c_this.View = view;
        c_this.Model = model;
        c_this.CursorPosition = { x: -1, y: -1 };
        c_this.SelectorPosition = { x: -1, y: -1 };
        c_this.Hovering = false;
        c_this.SelectingGroup = false;

        c_this.NoteIndex = 0;
        c_this.PendingTimeout = null;
    }

    DeleteSelectedNotes()
    {
        var i = 0;
        for(var index = 0; index < c_this.Model.Score.length; index++)
        {
            var note = c_this.Model.Score[index]
            if(note.IsSelected)
            {
                c_this.Model.DeleteNoteWithIndex(index)
            }
        }
    }

    ModifySelectedNotes(transform)
    {
        c_this.Model.Score.forEach( function(note)
        {
            if(note.IsSelected)
            {
                transform(note);
            }
        });
    }

    DoActionOnAllNotes(transform)
    {
        c_this.Model.Score.forEach( function(note)
        {
            transform(note)
        });
    }

    OnKeyUp(event)
    {
        switch(event.keyCode)
        {
        //Mode control: select, edit, delete
        case 88: //"x" key"
            console.log("Select mode");
            c_this.EditorMode = editModeEnumeration.SELECT;
            c_this.DeleteSelectedNotes(); //todo don't delete selected groups
            c_this.View.RenderNotes(c_this.Model.Score);

            break;
        case 90: //"z" key"
            console.log("Edit mode");
            c_this.EditorMode = editModeEnumeration.EDIT;
            c_this.DeleteSelectedNotes(); //todo don't delete selected groups
            var previewNote = c_this.CreatePreviewNote();
            c_this.Model.AddNote(previewNote);
            c_this.View.RenderNotes(c_this.Model.Score);
            zx
            break;
        case 68: //"d" key
            //Delete any selected notes, and enter delete mode
            c_this.DeleteSelectedNotes();
            c_this.EditorMode = editModeEnumeration.DELETE;
            break;

        case 32: //spacebar
            c_this.DoActionOnAllNotes(function(note){console.log(note);});
            c_this.PlayNotes();
            event.preventDefault();
            break;
        case 67: //"c" key"
            break;
        case 81: //"q" key
            if(editorMode == em.EDIT) { editState = editEnum.DRAG; }
            c_this.ModifySelectedNotes(function(note) {note.IsSelected = false;});
            deleteCurrentObj();
            break;
        case 87: //"w" key: Halve durations
            c_this.ModifySelectedNotes(function(note){note.Duration = ceil(note.Duration /= 2);});
            break;
        case 69: //"e" key: Double durations
            c_this.ModifySelectedNotes(function(note){ note.Duration = min(32, note.Duration *= 2); });
            break;

        }
    }

    GetNextUnselectedNote()
    {
        var note = null;

        console.log("note index "+c_this.NoteIndex);
        //Get the next note that isn't selected
        while(c_this.NoteIndex < c_this.Model.Score.length)
        {
            var note = c_this.Model.Score[c_this.NoteIndex ];

            c_this.NoteIndex++;
            if(!note.IsSelected)
            {
                break;
            }
            else
            {
                note = null;
            }

        }

        return note;
    }

    PlayChord()
    {
        var lastDelta = 0;
        var note = c_this.GetNextUnselectedNote();

        if(note != null)
        {
            lastDelta = note.StartTimeTicks;
            console.log("a",note.Pitch);
            note.Play();

            note = c_this.GetNextUnselectedNote();
        }

        if(note != null)
        {
            var millisecondsPerTick = 100;
            var relativeDelta = note.StartTimeTicks - lastDelta;
            var delta = relativeDelta*millisecondsPerTick;
            c_this.NoteIndex -= 1;

            c_this.PendingTimeout = setTimeout(c_this.PlayChord, delta)
        }
    }

    PlayNotes()
    {
        c_this.NoteIndex = 0;
        clearTimeout(c_this.PendingTimeout);
        c_this.PlayChord();
    }

    CreatePreviewNote()
    {
        var startTicks = c_this.View.ConvertXIndexToTicks(c_this.CursorPosition.x);
        var pitch = c_this.View.ConvertYIndexToPitch(c_this.CursorPosition.y);

        var previewNote = new Note(startTicks, pitch, 4);
        previewNote.IsSelected = true;

        return previewNote;
    }

    OnHoverBegin(event)
    {
        if(!c_this.Hovering)
        {
            var previewNote = c_this.CreatePreviewNote();
            c_this.Model.AddNote(previewNote);
            c_this.Hovering = true;
        }

        c_this.View.RenderNotes(c_this.Model.Score);
    }

    OnHoverEnd(event)
    {
        if(c_this.Hovering)
        {
            c_this.Hovering = false;
            c_this.DeleteSelectedNotes();
        }

        c_this.View.RenderNotes(c_this.Model.Score);
    }

    OnButtonPress(event)
    {
        //Play

    }

    ///Update the cursor position, move all selected notes
    OnMouseMove(cursorPosition)
    {
        c_this.CursorPosition = cursorPosition;

        //If there are selected notes, move them
        var selectCount = 0;
        c_this.ModifySelectedNotes(function(note){selectCount++;});

        //If a selection rectangle is being drawn, begin selecting notes caught in the rectangle
        if(c_this.SelectingGroup)
        {
            console.log("Resize");
            c_this.View.RenderSelectRectangle(c_this.SelectorPosition, c_this.CursorPosition);
        }

        //If no selection rectangle is being drawn,
        else if(selectCount > 0)
        {
            var startTicks = c_this.View.ConvertXIndexToTicks(cursorPosition.x);
            var pitch = c_this.View.ConvertYIndexToPitch(cursorPosition.y);

            c_this.ModifySelectedNotes(function(note){note.Move(startTicks, pitch);});
            c_this.View.RenderNotes(c_this.Model.Score);
        }

        else
        {

        }
    }

    ///Unselect all selected notes to anchor them
    OnMouseClickUp(event)
    {
        c_this.ModifySelectedNotes(function(note)
        {
            note.IsSelected = false;
            note.Play();
        });

        if(c_this.EditorMode == editModeEnumeration.SELECT)
        {
            //Delete the selection rectangle (TODO)
            c_this.View.DeleteSelectRectangle();
            c_this.SelectingGroup = false;
        }

        else
        {
            var previewNote = c_this.CreatePreviewNote();

            c_this.Model.AddNote(previewNote);
        }
    }

    OnMouseClickDown(event)
    {
        if(c_this.EditorMode == editModeEnumeration.SELECT)
        {
            var selectCount = 0;
            c_this.ModifySelectedNotes(function(note){selectCount++;});
            var cursorPosition = c_this.CursorPosition;
            var clickedNote = null;
            var cursorRectangle =
            {
                x1: cursorPosition.x,
                y1: cursorPosition.y,
                x2: cursorPosition.x,
                y2: cursorPosition.y,
            }

            c_this.Model.Score.forEach( function(note)
            {
                var x1Value = c_this.View.ConvertTicksToXIndex(note.StartTimeTicks);
                var y1Value = c_this.View.ConvertPitchToYIndex(note.Pitch);

                var noteRectangle = {
                    x1: x1Value,
                    y1: y1Value,
                    x2: x1Value+note.Duration,
                    y2: y1Value+1
                };

                var noteWasClicked = c_this.DoesRectangle1CoverRectangle2(noteRectangle, cursorRectangle);
                if(noteWasClicked)
                {
                    clickedNote = note;
                }

            });

            //If a note is clicked, play it
            if(clickedNote != null)
            {

                console.log("Clicked note");
                note.Play();
            }

            //If there are no selected notes, create a select rectangle
            else if(selectCount == 0)
            {
                c_this.SelectingGroup = true;

                c_this.SelectorPosition = c_this.CursorPosition;

                console.log("Begin selecting");
                c_this.View.RenderSelectRectangle(c_this.selectorStartPosition, c_this.CursorPosition);
            }

        }

    }


    DoesRectangle1CoverRectangle2(rectangle1, rectangle2)
    {
        return false;
    }



        	/* analysis suite
        function DrawBeats(meter)
        {
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
            //console.log(entry);
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
        DrawBeats(4);*/

}
