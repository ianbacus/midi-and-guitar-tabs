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
        c_this.EditorMode = editModeEnumeration.EDIT;
        c_this.View = view;
        c_this.Model = model;
        c_this.CursorPosition = { x: -1, y: -1 };
        c_this.SelectorPosition = { x: -1, y: -1 };
        c_this.Playing = false;
        c_this.Hovering = false;
        c_this.SelectingGroup = false;
        c_this.EditModeColors = ['orange','blue','green'];
        c_this.DefaultNoteDuration = 4;
        c_this.MillisecondsPerTick = 100;
        c_this.NoteIndex = 0;
        c_this.PendingTimeout = null;
        c_this.SequenceNumber = 0;


    }

    Initialize()
    {
        c_this.RenderGridArray();
    }

    RenderGridArray()
    {
        var [gridPreviewList, gridPreviewIndex] = [c_this.Model.GridPreviewList.length, c_this.Model.GridPreviewIndex];

        c_this.RenderMainGridBox()
        c_this.View.RenderGridArray(gridPreviewList, gridPreviewIndex);

    }

    RenderMainGridBox()
    {
        var editModeColor = c_this.EditModeColors[c_this.EditorMode];
        c_this.View.RenderNotes(c_this.Model.Score, editModeColor);
    }

    DeleteSelectedNotes(pushAction, sequenceNumber)
    {
        var i = 0;
        var sequenceNumber = 0;
        var score = c_this.Model.Score;

        for(var index = 0; index < score.length; index++)
        {
            var note = score[index]
            if(note.IsSelected)
            {
                c_this.Model.DeleteNoteWithIndex(index,sequenceNumber, score, pushAction)
                index--;
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

    CountSelectedNotes()
    {
        var selectCount = 0;
        c_this.ModifySelectedNotes(function(note)
        {
            selectCount++;}
        );

        return selectCount;
    }

    GetNextSequenceNumber()
    {
        c_this.SequenceNumber = (c_this.SequenceNumber+1)%2;
        return c_this.SequenceNumber;
    }

    OnKeyUp(event)
    {
        switch(event.keyCode)
        {
        //Mode control: select, edit, delete
        case 88: //"x" key": Select mode
            if(c_this.EditorMode != editModeEnumeration.SELECT)
            {
                c_this.EditorMode = editModeEnumeration.SELECT;
                c_this.HandleSelectionReset();
                c_this.RenderGridArray()
            }

            break;
        case 90: //"z" key" undo/redo, Edit mode
            var renderGrid = true;
            //Undo
            if(event.ctrlKey)
            {
                //If a group is being selected, unselect it
                var selectCount = c_this.CountSelectedNotes();
                if(selectCount > 1)
                {
                    console.log("Resetting selected notes");
                    c_this.HandleSelectionReset()
                }
                else
                {
                    c_this.Model.Undo();
                    c_this.StopPlayingNotes();

                }
            }

            //Redo
            else if(event.shiftKey)
            {
                c_this.Model.Redo();
                c_this.StopPlayingNotes();
            }

            //Edit mode
            else if(c_this.EditorMode != editModeEnumeration.EDIT)
            {
                c_this.EditorMode = editModeEnumeration.EDIT;
                c_this.HandleSelectionReset();
                var previewNote = c_this.CreatePreviewNote();
                c_this.Model.AddNote(previewNote, 0, c_this.Model.Score, false);
            }
            else {
                renderGrid = false;
            }
            if(renderGrid)
            {
                c_this.RenderGridArray();
            }

            break;
        case 68: //"d" key
            //Delete any selected notes, and enter delete mode
            c_this.DeleteSelectedNotes(true);
            c_this.RenderGridArray()
            break;

        case 32: //spacebar
            if(!c_this.Playing)
            {
                var playbackBuffer = []

                //Add all unselected notes to the playback buffer
                c_this.Model.GridPreviewList.forEach(function(arrayBuffer)
                {
                    arrayBuffer.forEach(function(note)
                    {
                        if(!note.IsSelected)
                        {
                            playbackBuffer.push(note);
                        }
                    });
                });

                c_this.PlayNotes(playbackBuffer);
            }
            else
            {
                c_this.StopPlayingNotes();
            }
            event.preventDefault();
            break;

        case 67: //"c" key"
            var copyBuffer = []

            //Copy all selected notes into a buffer
            c_this.ModifySelectedNotes(function(note)
            {
                var copiedNote = new Note(note.StartTimeTicks, note.Pitch, note.Duration, true);
                copyBuffer.push(copiedNote);
            });

            //Reset the position of the selected notes in case they were dragged away from their start point
            c_this.HandleSelectionReset();

            //Instantiate the copied notes
            copyBuffer.forEach(function(note)
            {
                c_this.Model.AddNote(note, 0, c_this.Model.Score, false);
            });

            c_this.RenderGridArray();
            break;

        case 81: //"q" key
            break;

        case 87: //"w" key: Halve durations
            break;

        case 69: //"e" key: Add new grid
            c_this.Model.CreateGridPreview();
            c_this.RenderGridArray();

            break;
        case 38: //up arrow: select grid
            event.preventDefault();
            c_this.HandleGridMove(true);
            c_this.RenderGridArray();
            break;

        case 40: //down arrow: select grid
            event.preventDefault();
            c_this.HandleGridMove(false);
            c_this.RenderGridArray();
            break;
        }
    }

    HandleGridMove(upwardsDirection)
    {
        var moveFunction;
        var copyBuffer = [];
        var sequenceNumber = 0;//c_this.GetNextSequenceNumber();
        var newGridIndex;

        if(upwardsDirection)
        {
            moveFunction = c_this.Model.GotoPreviousGrid;
            newGridIndex = Math.max(c_this.Model.GridPreviewIndex-1, 0);
        }

        else
        {
            moveFunction = c_this.Model.GotoNextGrid;
            newGridIndex = Math.min(c_this.Model.GridPreviewIndex+1, c_this.Model.GridPreviewList.length-1);
        }

        c_this.Model.SetCurrentGridPreview(c_this.Model.Score);

        console.log("Transport begin");
		
        //Capture any selected notes and delete them before changing grids
        c_this.ModifySelectedNotes(function(note)
        {
			var copiedNote = note;
            console.log("Packing note: ", note);
            copyBuffer.push(copiedNote);
        });

        c_this.DeleteSelectedNotes(false, sequenceNumber);

        //Change to the next grid
        moveFunction();

        //Instantiate the copied notes in the next buffer
        copyBuffer.forEach(function(note)
        {
            console.log("Transporting note: ", note, newGridIndex);
			note.CurrentGridIndex = newGridIndex;
            c_this.Model.AddNote(note, sequenceNumber, c_this.Model.Score, false);
        });

        console.log("Transport end");

    }

    GetNextUnselectedNote()
    {
        var note = null;

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

    DoNotesOverlap(note1, note2)
    {
        var note1SearchStartTicks = note1.StartTimeTicks;
        var note1SearchEndTicks = note1SearchStartTicks + note1.Duration;

        var note2SearchStartTicks = note2.StartTimeTicks;
        var note2SearchEndTicks = note2SearchStartTicks + note2.Duration;

        var note2StartsDuringNote1 =
            (note1SearchStartTicks <= note2SearchStartTicks) && (note2SearchStartTicks < note1SearchEndTicks);

        var note1StartsDuringNote2 =
            (note2SearchStartTicks <= note1SearchStartTicks) && (note1SearchStartTicks < note2SearchEndTicks);

        var results = {
            Note1Subordinate:note1StartsDuringNote2,
            Note2Subordinate:note2StartsDuringNote1,
        };

        return results;
    }

    GetChordNotes(noteArray, noteIndex, includeSuspensions)
    {
        var leftSearchIndex = noteIndex - 1;
        var rightSearchIndex = noteIndex + 1;
        var currentNote = noteArray[noteIndex];
        var chordNotes = [currentNote];
        var returnIndex = noteArray.length;

        function includeNote(targetNote,searchNote,includeSuspensions)
        {
            var result = false;
            if(!searchNote.IsSelected)
            {
                var overlapResult = c_this.DoNotesOverlap(targetNote, searchNote)
                var suspensionOrChordNote = overlapResult.Note1Subordinate;
                var chordNote = overlapResult.Note1Subordinate && overlapResult.Note2Subordinate;
                result = (includeSuspensions && suspensionOrChordNote) || (!includeSuspensions && chordNote);
            }

            return result;
        }

        //search left
        while(leftSearchIndex >= 0)
        {
            var leftSearchNote = noteArray[leftSearchIndex];
            var noteInChord = includeNote(currentNote,leftSearchNote, includeSuspensions);
            if(noteInChord)
            {
                chordNotes.push(leftSearchNote);
            }

            leftSearchIndex--;
        }

        //search right
        while(rightSearchIndex < noteArray.length)
        {
            var rightSearchNote = noteArray[rightSearchIndex];
            var noteInChord = includeNote(currentNote,rightSearchNote, includeSuspensions);
            if(noteInChord)
            {
                chordNotes.push(rightSearchNote);
            }

            //The first invalid unselected note on the right side will be the longest note of the next chord
            //as a result of the note sorting order
            else if(!rightSearchNote.IsSelected)
            {
                returnIndex = rightSearchIndex;
                break;
            }

            rightSearchIndex++;
        }

        return [chordNotes,returnIndex];
    }

    PlayChord(noteArray, noteIndex, includeSuspensions)
    {
        //Get all notes that play during this note, return the index of the first note that won't be played in this chord

        var [chordNotes,returnIndex] = c_this.GetChordNotes(noteArray, noteIndex, includeSuspensions)

        chordNotes.forEach(function(note)
        {
            note.Play(c_this.MillisecondsPerTick);
        });

        return returnIndex;
    }

    OnPlayAllNotes()
    {
        var playbackNoteArray = c_this.PlaybackNoteArray;
        var noteIndex = c_this.NoteIndex;
        var currentNote = playbackNoteArray[noteIndex];
        var nextNoteIndex = c_this.PlayChord(playbackNoteArray, noteIndex,false);
        console.log("OnPlay");
        if(nextNoteIndex < playbackNoteArray.length)
        {
            var nextNote = playbackNoteArray[nextNoteIndex];
            var relativeDelta = nextNote.StartTimeTicks - currentNote.StartTimeTicks;
            var delta = relativeDelta*c_this.MillisecondsPerTick;

            c_this.NoteIndex = nextNoteIndex;
            c_this.PendingTimeout = setTimeout(c_this.OnPlayAllNotes, delta)
        }
        else
        {
            c_this.StopPlayingNotes();
        }
    }

    PlayNotes(noteArray)
    {
        c_this.NoteIndex = 0;
        c_this.StopPlayingNotes();
        if(noteArray.length > 0)
        {
            c_this.PlaybackNoteArray = noteArray

            c_this.Playing = true;

            c_this.OnPlayAllNotes();
        }
    }

    StopPlayingNotes()
    {
        c_this.Playing = false;
        clearTimeout(c_this.PendingTimeout);

    }

    CreatePreviewNote()
    {
        var startTicks = c_this.View.ConvertXIndexToTicks(c_this.CursorPosition.x);
        var pitch = c_this.View.ConvertYIndexToPitch(c_this.CursorPosition.y);

        var previewNote = new Note(startTicks, pitch, c_this.DefaultNoteDuration, true);

        return previewNote;
    }

    OnHoverBegin(event)
    {
        if(!c_this.Hovering)
        {
            c_this.Hovering = true;
            if(c_this.EditorMode === editModeEnumeration.EDIT)
            {
                var previewNote = c_this.CreatePreviewNote();
                c_this.Model.AddNote(previewNote, 0, c_this.Model.Score, false);
            }
        }

        c_this.RenderMainGridBox()
    }

    OnHoverEnd(event)
    {
        if(c_this.Hovering)
        {
            c_this.Hovering = false;
            c_this.HandleSelectionReset();
        }

        c_this.RenderMainGridBox()
    }

    OnButtonPress(event)
    {
        //Play

    }

    HandleSelectionReset()
    {
		var score = c_this.Model.Score;
		
		
		var resetIndex = score.length;
		while(resetIndex --> 0)
        //c_this.ModifySelectedNotes(function(note)
        {
			var note = c_this.Model.Score[resetIndex];
			var deletion = true;
			
			if(note.IsSelected)
			{
				//Unselect and reset the position of notes that existed before selection
				if(note.SelectedPitchAndTicks != null)
				{
					console.log("HOVER reset ");
					note.IsSelected = false;
					deletion = note.ResetPosition();
				}

				//Delete preview notes that were not initially selected
				else
				{
					console.log("HOVER deleted ");
					c_this.Model.DeleteNote(note, 0, c_this.Model.Score, false);
				}
			}
			
        }
		//);
		
    }

    ///Update the cursor position, move all selected notes
    OnMouseMove(cursorPosition)
    {
        if(c_this.LastCursorPosition != c_this.CursorPosition)
        {
            c_this.LastCursorPosition = c_this.CursorPosition;
        }

        c_this.CursorPosition = cursorPosition;

        //If there are selected notes, move them
        var selectCount = c_this.CountSelectedNotes();

        //If a selection rectangle is being drawn, begin selecting notes caught in the rectangle
        if(c_this.SelectingGroup)
        {
            c_this.View.RenderSelectRectangle(c_this.SelectorPosition, c_this.CursorPosition);
            var selectRectangle =
            {
                x1: Math.min(c_this.SelectorPosition.x, c_this.CursorPosition.x),
                y1: Math.min(c_this.SelectorPosition.y, c_this.CursorPosition.y),
                x2: Math.max(c_this.SelectorPosition.x, c_this.CursorPosition.x),
                y2: Math.max(c_this.SelectorPosition.y, c_this.CursorPosition.y)
            };

            c_this.DoActionOnAllNotes(function(note)
            {
                var noteRectangle = c_this.GetNoteRectangle(note);
                var noteIsCaptured = c_this.DoesRectangle1CoverRectangle2(selectRectangle, noteRectangle);

                if(noteIsCaptured)
                {
                    note.IsSelected = true;
                }

                else
                {
                    note.IsSelected = false;
                }
            });

            c_this.RenderMainGridBox()
        }

        //If no selection rectangle is being drawn,
        else if(selectCount > 0)
        {
            var x_offset = c_this.View.ConvertXIndexToTicks(c_this.CursorPosition.x) - c_this.View.ConvertXIndexToTicks(c_this.LastCursorPosition.x);
            var y_offset = c_this.View.ConvertYIndexToPitch(c_this.CursorPosition.y) - c_this.View.ConvertYIndexToPitch(c_this.LastCursorPosition.y);

            //Todo: move notes relative to start position
            c_this.ModifySelectedNotes(function(note){
                note.Move(x_offset, y_offset);
            });
            c_this.Model.SortScoreByTicks();
            c_this.RenderMainGridBox()
        }

        else
        {

        }
    }

    HandleIndividualNotePlayback(noteIndex)
    {
        var playbackMode = 2;
        var score = c_this.Model.Score;
        var note = score[noteIndex];

        var eventData = c_this.View.GetFormData();
        eventData.forEach(function(formData)
        {
            if(formData.id == 'Playback')
            {
                playbackMode = formData.value;
            }
        });

        if(playbackMode == 0)
        {
            note.Play(c_this.MillisecondsPerTick);
        }

        else if(playbackMode == 1)
        {
            c_this.PlayChord(score, noteIndex, false)
        }

        else
        {
            c_this.PlayChord(score, noteIndex, true)
        }
    }

    OnMouseClickDown(event)
    {
        if(c_this.EditorMode == editModeEnumeration.SELECT)
        {
            var selectCount = c_this.CountSelectedNotes();
            var clickedNoteIndex = c_this.GetNoteIfClicked();
            var score = c_this.Model.Score;

            //If a note is clicked, play it
            if((0 <= clickedNoteIndex) && (clickedNoteIndex < score.length))
            {
                c_this.HandleIndividualNotePlayback(clickedNoteIndex);
            }

            //If there are no selected notes, create a select rectangle
            else if(selectCount == 0)
            {
                c_this.SelectingGroup = true;

                c_this.SelectorPosition = c_this.CursorPosition;

                c_this.View.RenderSelectRectangle(c_this.SelectorPosition, c_this.CursorPosition);
            }
        }
    }
	
	GetPlaybackMode()
	{
		var eventData = c_this.View.GetFormData();
		var playbackMode = 0;
		
		eventData.forEach(function(formData)
		{
			if(formData.id == 'Playback')
			{
				playbackMode = formData.value;
			}
		});
		
		return playbackMode;
	}

    ///Unselect all selected notes to anchor them
    OnMouseClickUp(event)
    {
        c_this.StopPlayingNotes();
        if(c_this.SelectingGroup === true)
        {
            c_this.View.DeleteSelectRectangle();
            c_this.SelectingGroup = false;
        }

        else
        {
			var playbackBuffer = [];
            var playbackMode = c_this.GetPlaybackMode();
            var sequenceNumber = sequenceNumber = c_this.GetNextSequenceNumber();

			//Play notes and handle move completion
            if(playbackMode == 0)
            {
                note.Play(c_this.MillisecondsPerTick);
                c_this.ModifySelectedNotes(function(note)
                {
                    playbackBuffer.push(note);
                    note.IsSelected = false;
                    note.OnMoveComplete(sequenceNumber);
                });
            }

			//Play all intersecting chords and handle move completion
            else
            {
                c_this.ModifySelectedNotes(function(note)
                {
                    playbackBuffer.push(note);
                });

                //play chords of each note by merging the playback buffer with the overlapping parts of the score
                c_this.DoActionOnAllNotes(function(note)
                {
                    var firstTick = playbackBuffer[0].StartTimeTicks;
                    var lastTick = playbackBuffer[playbackBuffer.length-1].StartTimeTicks + playbackBuffer[playbackBuffer.length-1].Duration;
                    var bounded = (firstTick <= note.StartTimeTicks) && (note.StartTimeTicks < lastTick)
                    if(!note.IsSelected && bounded)
                    {
                        playbackBuffer.push(note);
                    }
                });

                c_this.ModifySelectedNotes(function(note)
                {
                    note.IsSelected = false;
                    note.OnMoveComplete(sequenceNumber);
                });

                playbackBuffer.sort(m_this.CompareNotes);
            }

            c_this.PlayNotes(playbackBuffer);
        }

		//Create a new preview note if edit mode is active
        if(c_this.EditorMode == editModeEnumeration.EDIT)
        {
            var previewNote = c_this.CreatePreviewNote();
            c_this.Model.AddNote(previewNote, 0, c_this.Model.Score, false);
        }

        c_this.RenderGridArray();
    }

    HandleControlScroll(scrollUp)
    {
        var shouldScroll = true;
        var selectCount = c_this.CountSelectedNotes();

        function shouldResizeNote(note, selectCount)
        {
            return (!note.IsSelected && (selectCount == 0)) || (note.IsSelected  && (selectCount > 0))
        }

        var firstNotePosition = undefined;
        c_this.DoActionOnAllNotes(function(note)
        {
            //Determine if the resize request is valid
            if(shouldResizeNote(note,selectCount))
            {
                if(firstNotePosition == undefined)
                {
                    firstNotePosition = note.StartTimeTicks;
                }

                var noteOffset =  (note.StartTimeTicks - firstNotePosition);

                if(!shouldScroll)
                {
                    return;
                }
                else if(scrollUp)
                {
                    shouldScroll = note.Duration <= 8;
                }
                else
                {
                    shouldScroll = (note.Duration > 1)  && ((noteOffset % 2) == 0);;
                }
            }
        });

        firstNotePosition = undefined;
        if(shouldScroll)
        {
            //Resize all notes as requested. If only one note is selected, treat it like a preview note and change the default preview note size
            c_this.DoActionOnAllNotes(function(note)
            {
                if(shouldResizeNote(note,selectCount))
                {
                    if(firstNotePosition == undefined)
                    {
                        firstNotePosition = note.StartTimeTicks;
                    }

                    var newDuration;
                    var newPosition;

                    var noteOffset =  (note.StartTimeTicks - firstNotePosition);

                    if(scrollUp)
                    {
                        shouldScroll = note.Duration <= 16;
                        newDuration = note.Duration*2;
                        newPosition = firstNotePosition + noteOffset*2;
                    }
                    else
                    {
                        shouldScroll = note.Duration > 1;
                        newDuration = note.Duration/2;
                        newPosition = firstNotePosition + noteOffset/2;
                    }

                    if(selectCount == 1)
                    {
                        c_this.DefaultNoteDuration = newDuration;
                    }

                    note.Duration = newDuration;
                    note.StartTimeTicks = newPosition;

                }
            });
            c_this.RenderMainGridBox();
        }
    }
    OnMouseScroll(event)
    {
        var ctrl = event.ctrlKey;
        var alt = event.altKey;
        var shift = event.shiftKey;
        var scrollUp = (event.originalEvent.wheelDelta > 0 || event.originalEvent.detail < 0);

        //Change horizontal scale
        if(ctrl)
        {
            c_this.HandleControlScroll(scrollUp)
            event.preventDefault();
        }
        else if(shift)
        {
            event.preventDefault();
            var xOffset = 100;
            if(scrollUp)
            {
                xOffset *= -1;
            }

            c_this.View.ScrollHorizontal(xOffset)
        }

        else
        {
            //Regular scroll: do nothing
        }
    }

    OnRadioButtonPress(eventData)
    {
        console.log(eventData);
        /*
        var html2obj = html2canvas($('body')[0]);
        var queue  = html2obj.parse();
        var canvas = html2obj.render(queue);
        var img = canvas.toDataURL();

        window.open(img);*/

    }

    GetNoteRectangle(note)
    {
        var x1Value = c_this.View.ConvertTicksToXIndex(note.StartTimeTicks);
        var y1Value = c_this.View.ConvertPitchToYIndex(note.Pitch);
        var gridSnap = c_this.View.gridSnap;

        var noteRectangle = {
            x1: x1Value,
            y1: y1Value,
            x2: x1Value+note.Duration*gridSnap,
            y2: y1Value+1*gridSnap
        };

        return noteRectangle;
    }

    GetNoteIfClicked()
    {
        //Get the index of a clicked note. Return -1 if no notes were clicked
        var clickedNoteIndex = -1;

        var cursorPosition = c_this.CursorPosition;
        var cursorRectangle =
        {
            x1: cursorPosition.x,
            y1: cursorPosition.y,
            x2: cursorPosition.x,
            y2: cursorPosition.y,
        }

        var noteIndex = 0;
        c_this.DoActionOnAllNotes( function(note)
        {
            var noteRectangle = c_this.GetNoteRectangle(note);
            var noteWasClicked = c_this.DoesRectangle1CoverRectangle2(noteRectangle, cursorRectangle);

            if(noteWasClicked)
            {
                clickedNoteIndex = noteIndex;
            }
            noteIndex++;
        });

        return clickedNoteIndex;
    }


    DoesRectangle1CoverRectangle2(rectangle1, rectangle2)
    {
        var xCovered = (rectangle1.x1 <= rectangle2.x1) && (rectangle1.x2 >= rectangle2.x2) ;
        var yCovered = (rectangle1.y1 <= rectangle2.y1) && (rectangle1.y2 >= rectangle2.y2) ;
        var result = xCovered && yCovered;
        return result;
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
