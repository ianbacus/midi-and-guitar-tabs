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

var Modes =
[
    [2,2,1,2,2,1], //major
    [2,1,2,2,2,1,1,1], //minor
];

let c_this = undefined;

class Controller
{
    constructor(view, model)
    {
        c_this = this;
        this.EditorMode = editModeEnumeration.EDIT;
        this.View = view;
        this.Model = model;
        this.CursorPosition = { x: -1, y: -1 };
        this.SelectorPosition = { x: -1, y: -1 };
        this.Playing = false;
        this.Hovering = false;
        this.SelectingGroup = false;
        this.EditModeColors = ['orange','blue','green'];
        this.DefaultNoteDuration = 4;
        this.MillisecondsPerTick = 100;
        this.NoteIndex = 0;
        this.PendingTimeout = null;
        this.SequenceNumber = 0;
        this.console = null;
        c_this.TonicKey = 0;
        c_this.MusicalModeIndex = 0;

        this.PasteBuffer = []

    }

    SetKeyReference(tonic, modeIndex)
    {
        var tonicOpacity = 0.25;
        var dominantOpacity = 0.20;
		var modeBuffer = [{Pitch:tonic, Opacity: tonicOpacity}];
        var currentTone = tonic;
        var intervals = Modes[modeIndex]

		intervals.some(function(interval)
		{
            var noteOpacity = 0.050;
            currentTone += interval;
            var relativeInterval = Math.abs(currentTone - tonic)

            //Dominant
            if(relativeInterval === 7)
            {
                noteOpacity = dominantOpacity;
            }

            var modeSlot = {Pitch:currentTone, Opacity: noteOpacity};
            modeBuffer.push(modeSlot);
		});

		c_this.View.RenderKeys(modeBuffer);
    }

    Initialize()
    {
        this.RefreshGridPreview();
        this.SetKeyReference(c_this.TonicKey, c_this.MusicalModeIndex);

    }

    OnThumbnailRender(eventData)
    {
        var image = eventData.Image;
        var index = eventData.GridIndex;

        c_this.Model.GridImageList[index] = image;

        c_this.View.RenderGridArray(c_this.Model.GridImageList, index);
    }

    RefreshGridPreview()
    {
        c_this.RefreshEditBoxNotes();
        c_this.View.GetGridboxThumbnail(c_this, c_this.OnThumbnailRender, c_this.Model.GridPreviewIndex);
    }

    RefreshEditBoxNotes()
    {
        var editModeColor = c_this.EditModeColors[c_this.EditorMode];
        c_this.View.RenderNotes(c_this.Model.Score, editModeColor);
    }

    DeleteSelectedNotes(pushAction)
    {
        var i = 0;
        var score = c_this.Model.Score;
        var sequenceNumber = 0;

        if(pushAction)
        {
            sequenceNumber = c_this.GetNextSequenceNumber();
        }

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

    ModifySelectedNotes(transform, forwardIterate=true)
    {
        c_this.Model.SelectedNotes.some(function(note)
        {
            transform(note);
        });
    }

    ModifyNoteArray(noteArray, modifyFunction, forwardIterate=true)
    {
        if(forwardIterate)
        {
            var modifyIndex = -1;
            while(modifyIndex++ < noteArray.length-1)
            {
    			var note = noteArray[modifyIndex];
                modifyFunction(note);
            }
        }
        else
        {
            var modifyIndex = noteArray.length;
            while(modifyIndex --> 0)
            {
    			var note = noteArray[modifyIndex];
                modifyFunction(note);
            }
        }
    }

    DoActionOnAllNotes(transform)
    {
        c_this.Model.Score.some( function(note)
        {
            transform(note)
        });
    }

    CountSelectedNotes()
    {
        var selectCount = c_this.Model.SelectedNotes.length;
        c_this.console.log("Select count: " + selectCount + " notes.");

        return selectCount;
    }

    GetNextSequenceNumber()
    {
		var maximumSequenceNumber = 10000;//Number.MAX_SAFE_INTEGER-1;
        c_this.SequenceNumber = (c_this.SequenceNumber+1)%maximumSequenceNumber;
        return c_this.SequenceNumber;
    }

    PreparePasteBuffer(copyBuffer)
    {
        var pasteBuffer = [];
        copyBuffer.forEach(function(noteToCopy)
        {
            var noteCopy = new Note(noteToCopy.StartTimeTicks, noteToCopy.Pitch, noteToCopy.Duration, false);

            var offsetX = c_this.View.ConvertTicksToXIndex(noteToCopy.StartTimeTicks);
            var offsetY = c_this.View.ConvertPitchToYIndex(noteToCopy.Pitch);

            var noteCursorDisplacement =
            {
                x: offsetX - c_this.CursorPosition.x,
                y: offsetY - c_this.CursorPosition.y,
            }

            var pasteData =
            {
                NoteCopy:noteCopy,
                NoteCursorDisplacement: noteCursorDisplacement,
            }

            pasteBuffer.push(pasteData);
        });

        return pasteBuffer;
    }

    InstantiatePasteBuffer(pasteBuffer)
    {
        // c_this.console.log(pasteBuffer);
        //Reset the position of the selected notes in case they were dragged away from their start point
        c_this.HandleSelectionReset();

        //Instantiate the copied notes
        pasteBuffer.forEach(function(pasteData)
        {
            var noteToPaste = pasteData.NoteCopy;
            var noteCursorDisplacement = pasteData.NoteCursorDisplacement;

            var xticks = noteCursorDisplacement.x + c_this.CursorPosition.x;
            var yticks = noteCursorDisplacement.y + c_this.CursorPosition.y;

            var startTimeTicks =
                c_this.View.ConvertXIndexToTicks(c_this.CursorPosition.x) +
                c_this.View.ConvertXIndexToTicks(noteCursorDisplacement.x);

            var pitch =
                c_this.View.ConvertYIndexToPitch(c_this.CursorPosition.y) +
                c_this.View.ConvertYIndexToPitch(noteCursorDisplacement.y);

            var p2 = c_this.View.ConvertYIndexToPitch(yticks);

            var instantiatedNote = new Note(startTimeTicks, p2, noteToPaste.Duration, true);
            c_this.Model.AddNote(instantiatedNote, 0, c_this.Model.Score, false);
        });
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
                c_this.RefreshEditBoxNotes()
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
                c_this.RefreshEditBoxNotes();
            }

            break;
        case 68: //"d" key
            //Delete any selected notes, and enter delete mode
            c_this.DeleteSelectedNotes(true);
            c_this.RefreshGridPreview()
            break;
        case 9: //tab key
            event.preventDefault();
            var keys = 12;
            if(event.shiftKey)
            {
                c_this.TonicKey = (c_this.TonicKey+(keys-7))%keys;
            }
            else
            {
                c_this.TonicKey = (c_this.TonicKey+7)%keys;
            }
            c_this.SetKeyReference(c_this.TonicKey, c_this.MusicalModeIndex);

            break;
        case 192: //` tilde key
            c_this.MusicalModeIndex = (c_this.MusicalModeIndex+1) % Modes.length;
            c_this.SetKeyReference(c_this.TonicKey, c_this.MusicalModeIndex);
            break;
        case 32: //spacebar
            if(!c_this.Playing)
            {
                var playbackBuffer = []
				c_this.HandleSelectionReset();

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

                c_this.PlayNotes(playbackBuffer, false);
            }
            else
            {
                c_this.StopPlayingNotes();
            }
            event.preventDefault();
            break;

        case 67: //"c" key"
            var copyBuffer = [];

            //Copy all selected notes into a buffer
            c_this.ModifySelectedNotes(function(noteToCopy)
            {
                copyBuffer.push(noteToCopy);
            });

            c_this.PasteBuffer = c_this.PreparePasteBuffer(copyBuffer);
            c_this.InstantiatePasteBuffer(c_this.PasteBuffer);

            c_this.RefreshEditBoxNotes();
            break;

        case 86: //"v" key

            c_this.InstantiatePasteBuffer(c_this.PasteBuffer);
            c_this.RefreshGridPreview();
            break;

        case 65: //"a key"
            //ctrl+a: select all
            event.preventDefault();
            if(c_this.EditorMode != editModeEnumeration.SELECT)
            {
                c_this.EditorMode = editModeEnumeration.SELECT;
                c_this.HandleSelectionReset();
            }
            if(event.ctrlKey)
            {
                c_this.ModifyNoteArray(c_this.Model.Score, function(note)
                {
                    note.IsSelected = true;
                });
            }
            c_this.RefreshEditBoxNotes();
        case 81: //"q" key
            break;

        case 87: //"w" key: Halve durations
            break;

        case 69: //"e" key: Add new grid
            c_this.Model.CreateGridPreview();
            c_this.RefreshGridPreview();

            break;
        case 38: //up arrow: select grid
            event.preventDefault();
            c_this.HandleGridMove(true);
            c_this.RefreshGridPreview();
            break;

        case 40: //down arrow: select grid
            event.preventDefault();
            c_this.HandleGridMove(false);
            c_this.RefreshGridPreview();
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

        c_this.console.log("Transport begin");

        //Capture any selected notes and delete them before changing grids
        c_this.ModifyNoteArray(c_this.Model.SelectedNotes, function(note)
        {
			var copiedNote = note;
            c_this.console.log("Packing note: ", note);
            copyBuffer.push(copiedNote);
        });

        c_this.DeleteSelectedNotes(false, sequenceNumber);

        //Change to the next grid
        moveFunction();

        //Instantiate the copied notes in the next buffer
        copyBuffer.forEach(function(note)
        {
            c_this.console.log("Transporting note: ", note, newGridIndex);
			note.CurrentGridIndex = newGridIndex;
            c_this.Model.AddNote(note, sequenceNumber, c_this.Model.Score, false);
        });

        c_this.console.log("Transport end");

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
        var currentNoteStartTicks = currentNote.StartTimeTicks;
        var wholeNoteDurationTicks = 8;

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
            var searchNoteTickDifference = currentNoteStartTicks - leftSearchNote.StartTimeTicks;

            //Search until the start ticks are out of range (a whole note)
            //Search until the start ticks are no longer equivalent
            if((includeSuspensions && (searchNoteTickDifference > wholeNoteDurationTicks)) ||
                (!includeSuspensions && (searchNoteTickDifference != 0)))
            {
                break;
            }
            else
            {
                var noteInChord = includeNote(currentNote,leftSearchNote, includeSuspensions);
                if(noteInChord)
                {
                    chordNotes.push(leftSearchNote);
                }

                leftSearchIndex--;
            }
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

	OnStopNote()
	{
        c_this.RefreshEditBoxNotes();
	}

    PlayChord(noteArray, noteIndex, includeSuspensions)
    {
        //Get all notes that play during this note, return the index of the first note that won't be played in this chord

        var [chordNotes,returnIndex] = c_this.GetChordNotes(noteArray, noteIndex, includeSuspensions)

        chordNotes.forEach(function(note)
        {
            note.Play(c_this.MillisecondsPerTick, c_this, c_this.OnStopNote);
        });

        c_this.RefreshEditBoxNotes();

        return returnIndex;
    }

    OnPlayAllNotes(includeSuspensions=false)
    {
        includeSuspensions = c_this.IncludeSuspensions;
        var playbackNoteArray = c_this.PlaybackNoteArray;
        var noteIndex = c_this.NoteIndex;
        var currentNote = playbackNoteArray[noteIndex];
        var nextNoteIndex = c_this.PlayChord(playbackNoteArray, noteIndex, includeSuspensions);

        var delta = 0;
        var xOffset = c_this.View.ConvertTicksToXIndex(currentNote.StartTimeTicks);


        if(nextNoteIndex < playbackNoteArray.length)
        {
            var nextNote = playbackNoteArray[nextNoteIndex];
            var relativeDelta = nextNote.StartTimeTicks - currentNote.StartTimeTicks;
            delta = relativeDelta*c_this.MillisecondsPerTick;

            c_this.NoteIndex = nextNoteIndex;

            //Time correction code
            if(c_this.ExpectedTime == undefined)
            {
                c_this.ExpectedTime = Date.now() + delta;
            }
            else
            {
                var elapsedTime = Date.now() - c_this.ExpectedTime;
                c_this.ExpectedTime += delta;
                delta = Math.max(0, delta - elapsedTime);
            }

            c_this.PendingTimeout = setTimeout(c_this.OnPlayAllNotes, delta);
        }
        else
        {
            c_this.StopPlayingNotes();
        }
    }

    PlayNotes(noteArray, includeSuspensions)
    {
        c_this.NoteIndex = 0;
        c_this.StopPlayingNotes();
        if(noteArray.length > 0)
        {
            c_this.PlaybackNoteArray = noteArray

            c_this.Playing = true;
            var firstNote = noteArray[0];
            var lastNote = noteArray[noteArray.length-1];
            var startTime = firstNote.StartTimeTicks;
            var endTime = lastNote.StartTimeTicks + lastNote.Duration;
            var startX = c_this.View.ConvertTicksToXIndex(startTime);
            var endX = c_this.View.ConvertTicksToXIndex(endTime);
            var playbackDurationMilliseconds = (endTime - startTime)*c_this.MillisecondsPerTick;
            c_this.OnPlayAllNotes(includeSuspensions);

			if(firstNote.StartTimeTicks != lastNote.StartTimeTicks)
			{
				var [chord,x] = c_this.GetChordNotes(noteArray, 0, includeSuspensions);
				var averagePitchSum = 0;
				chord.forEach(function(note)
				{
					averagePitchSum += note.Pitch;
				});

				var averagePitch = averagePitchSum / chord.length;
				var ycoord = c_this.View.ConvertPitchToYIndex(averagePitch);
				c_this.View.SmoothScroll(startX, startX,ycoord, 500);
				c_this.View.SmoothScroll(startX, endX, undefined, c_this.MillisecondsPerTick);
			}

        }
    }

    StopPlayingNotes()
    {
        c_this.Playing = false;
        c_this.ExpectedTime = undefined;
        c_this.View.CancelScroll();
        clearTimeout(c_this.PendingTimeout);
        c_this.RefreshEditBoxNotes();

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
				c_this.RefreshEditBoxNotes()
            }
        }
    }

    OnHoverEnd(event)
    {
        if(c_this.Hovering)
        {
            c_this.Hovering = false;
            c_this.HandleSelectionReset();
        }

        c_this.RefreshEditBoxNotes()
    }

    OnButtonPress(event)
    {
        //Play

    }

    HandleSelectionReset()
    {
        c_this.ModifyNoteArray(c_this.Model.SelectedNotes, function(note)
        {
            //Unselect and reset the position of notes that existed before selection
			if(note.StateWhenSelected != null)
			{
				note.IsSelected = false;
				note.ResetPosition();
			}

			//Delete preview notes that were not initially selected
			else
			{
				c_this.Model.DeleteNote(note, 0, c_this.Model.Score, false);
			}
        }, false);
    }

    ///Update the cursor position, move all selected notes
    OnMouseMove(cursorPosition)
    {
		//Only process mouse move events if the position changes
        if(c_this.LastCursorPosition != c_this.CursorPosition)
        {
            c_this.LastCursorPosition = c_this.CursorPosition;
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

				c_this.ModifyNoteArray(c_this.Model.Score, function(note)
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

				c_this.RefreshEditBoxNotes()
			}

			//If no selection rectangle is being drawn, move all selected notes
			else if(selectCount > 0)
			{
				var x_offset = c_this.View.ConvertXIndexToTicks(c_this.CursorPosition.x) - c_this.View.ConvertXIndexToTicks(c_this.LastCursorPosition.x);
				var y_offset = c_this.View.ConvertYIndexToPitch(c_this.CursorPosition.y) - c_this.View.ConvertYIndexToPitch(c_this.LastCursorPosition.y);

				c_this.ModifyNoteArray(c_this.Model.SelectedNotes, function(note){
					note.Move(x_offset, y_offset);
				});
				c_this.Model.SortScoreByTicks();
				c_this.RefreshEditBoxNotes()
			}
		}
    }

    HandleIndividualNotePlayback(noteIndex)
    {
        var playbackMode = c_this.GetPlaybackMode();
        var score = c_this.Model.Score;
        var note = score[noteIndex];

		//Solo
        if(playbackMode == 0)
        {
            c_this.PlayChord([note], noteIndex, false)
        }

		//Chords: play notes that have the same start time
        else if(playbackMode == 1)
        {
            c_this.PlayChord(score, noteIndex, false)
        }

		//Suspensions: play notes whose durations extend over the selected note
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
                score[clickedNoteIndex].IsSelected = true;
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
			if(formData.id == 'PlayPreview')
			{
				playbackMode = formData.value;
			}
		});

		return playbackMode;
	}

    ///Unselect all selected notes to anchor them and play them
    OnMouseClickUp(event)
    {
        event.preventDefault();
        c_this.StopPlayingNotes();
        if(c_this.SelectingGroup === true)
        {
            c_this.View.DeleteSelectRectangle();
            c_this.SelectingGroup = false;
        }

        else
        {
            var selectCount = c_this.CountSelectedNotes();
			var playbackBuffer = [];
            var playbackMode = c_this.GetPlaybackMode();
            var sequenceNumber = sequenceNumber = c_this.GetNextSequenceNumber();

			//Play notes and handle move completion
            if(playbackMode == 0)
            {
                c_this.ModifyNoteArray(c_this.Model.SelectedNotes, function(note)
                {
                    playbackBuffer.push(note);
                    note.IsSelected = false;
                    note.OnMoveComplete(sequenceNumber);
                }, false);
            }

			//Play all intersecting chords and handle move completion
            else if(selectCount > 0)
            {
                //Push all selected notes to the playback buffer
                c_this.ModifyNoteArray(c_this.Model.SelectedNotes, function(note)
                {
                    playbackBuffer.push(note);
                });

                var startTickBoundary = playbackBuffer[0].StartTimeTicks;
                var playbackBufferEndIndex = playbackBuffer.length-1;

                var endTickBoundary =
                    playbackBuffer[playbackBufferEndIndex].StartTimeTicks +
                    playbackBuffer[playbackBufferEndIndex].Duration;

                //Find all notes in the score that intersect with the selected notes
                c_this.ModifyNoteArray(c_this.Model.Score, function(note)
                {
                    var intersectsSelectedNote =
                        (startTickBoundary <= note.StartTimeTicks) &&
                        (note.StartTimeTicks < endTickBoundary);

                    if(!note.IsSelected && intersectsSelectedNote)
                    {
                        playbackBuffer.push(note);
                    }
                });

                //Place all selected notes
                c_this.ModifyNoteArray(
                    c_this.Model.SelectedNotes,
                    function(note)
                    {
                        note.IsSelected = false;
                        note.OnMoveComplete(sequenceNumber);
                    }, false);

                //Sort the playback buffer
                playbackBuffer.sort(m_this.CompareNotes);
            }
            if(playbackMode == 1)
            {
                c_this.PlayNotes(playbackBuffer,false);
            }
            else
            {
                c_this.PlayNotes(playbackBuffer,true);
            }
        }

		//Create a new preview note if edit mode is active
        if(c_this.EditorMode == editModeEnumeration.EDIT)
        {
            var previewNote = c_this.CreatePreviewNote();
            c_this.Model.AddNote(previewNote, 0, c_this.Model.Score, false);
        }

        c_this.RefreshGridPreview();
    }

    //Resize notes
    HandleControlScroll(scrollUp)
    {
        var shouldScroll = true;
        var selectCount = c_this.CountSelectedNotes();
        var noteArray = c_this.Model.Score;

        if(selectCount != 0)
        {
            noteArray = c_this.Model.SelectedNotes;
        }

        //Only allow all selected notes or all unselected notes, depending on the select count.
        //case 1: select count of 0 -> resize all notes
        //case 2: select count != 0 -> resize all selected notes
        function evaluateNoteScrollBehavior(note, selectCount)
        {
            var selected = note.IsSelected;
            var resizeNote = (!selected && (selectCount == 0)) || (selected  && (selectCount > 0))
            var unselectedNotesOnly = !selected;

            return resizeNote
        };

        var firstNotePosition = undefined;
        //Determine if the resize request is valid
        c_this.ModifyNoteArray(noteArray, function(note)
        {
            var shouldResizeNote = evaluateNoteScrollBehavior(note,selectCount);

            if(shouldResizeNote)
            {
                if(firstNotePosition == undefined)
                {
                    firstNotePosition = note.StartTimeTicks;
                }

                var noteOffset = (note.StartTimeTicks - firstNotePosition);

                if(scrollUp)
                {
                    shouldScroll = note.Duration <= 8;
                }
                else
                {
                    shouldScroll = (note.Duration > 1)  && ((noteOffset % 2) == 0);
                }

                if(!shouldScroll)
                {
                    return;
                }
            }
        });

        firstNotePosition = undefined;
        if(shouldScroll)
        {
            var sequenceNumber = c_this.GetNextSequenceNumber();

            //Resize all notes as requested. If only one note is selected, treat it like a preview note and change the default preview note size
            c_this.ModifyNoteArray(noteArray, function(note)
            {
                var newDuration;
                var newPosition;
                var shouldResizeNote = evaluateNoteScrollBehavior(note,selectCount);

                if(shouldResizeNote)
                {
                    if(firstNotePosition == undefined)
                    {
                        firstNotePosition = note.StartTimeTicks;
                        //TODO: Not sure about this yet
                        // if(scrollUp && unselectedNotesOnly && (c_this.MillisecondsPerTick > 100))
                        // {
                        //     c_this.MillisecondsPerTick /= 2;
                        // }
                        // else if(!scrollUp && unselectedNotesOnly && (c_this.MillisecondsPerTick < 1000))
                        // {
                        //     c_this.MillisecondsPerTick *= 2;
                        // }
                    }

                    var noteOffset = (note.StartTimeTicks - firstNotePosition);

                    if(scrollUp)
                    {
                        newDuration = note.Duration*2;
                        newPosition = firstNotePosition + noteOffset*2;
                    }
                    else
                    {
                        newDuration = note.Duration/2;
                        newPosition = firstNotePosition + noteOffset/2;
                    }

                    //Update the default note duration when the preview note is resized
                    if(selectCount == 1)
                    {
                        c_this.DefaultNoteDuration = newDuration;
                    }

                    note.HorizontalModify(newPosition, newDuration, sequenceNumber);
                }
            });
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
            event.preventDefault();
            c_this.HandleControlScroll(scrollUp)
            c_this.RefreshEditBoxNotes();
        }
        else if(shift)
        {
            event.preventDefault();
            var xOffset = c_this.DefaultNoteDuration*c_this.View.gridSnap;

			var cursorPosition =
			{
				x:null,
				y:c_this.CursorPosition.y
			}

            if(scrollUp)
            {
                xOffset *= -1;
            }

            var actualXOffset = c_this.View.ScrollHorizontal(xOffset);
			cursorPosition.x = c_this.CursorPosition.x + actualXOffset
			c_this.OnMouseMove(cursorPosition);
        }

        else
        {
            event.preventDefault();
            var yOffset = c_this.View.gridSnap;

			var cursorPosition =
			{
				x:c_this.CursorPosition.x,
				y:null
			}

            if(scrollUp)
            {
                yOffset *= -1;
            }

            var actualYOffset = c_this.View.ScrollVertical(yOffset);
			cursorPosition.y = c_this.CursorPosition.y + actualYOffset
			c_this.OnMouseMove(cursorPosition);
        }
    }

    OnRadioButtonPress(eventData)
    {
        c_this.console.log(eventData);

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
        c_this.ModifyNoteArray(c_this.Model.Score, function(note)
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
            //c_this.console.log(entry);
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
        	//c_this.console.log('temporary: '+JSON.stringify(temporaryDict));
        	//c_this.console.log('main: '+JSON.stringify(temporalDict));

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
        	c_this.console.log(JSON.stringify(temporalDict));
        	for(var delta in temporalDict)
        	{
            //If more than 2 entries, just ignore it.. use outer eventually
            if(temporalDict[delta].length == 2)
            	intervals.push(Math.abs(temporalDict[delta][0].pitch - temporalDict[delta][1].pitch));

        	}
        	c_this.console.log(intervals);
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
