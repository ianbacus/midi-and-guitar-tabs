let m_this = undefined;

var InstrumentEnum = {
	Violin: 0,
	Guitar: 1,
	Flute: 2
};

class Note
{
    constructor(startTimeTicks, pitch, duration, selected, currentGridIndex=m_this.GridPreviewIndex)
    {
        this.Pitch = pitch;
        this.StartTimeTicks = startTimeTicks;
        this.Duration = duration;
        this._IsSelected = selected;
        this.CurrentGridIndex = currentGridIndex;

        this.StateWhenSelected = null
        this.SelectedGridIndex = m_this.GridPreviewIndex;
        this.console = null;

        this.IsHighlighted = false;
    }

    Move(x_offset, y_offset)
    {
        this.StartTimeTicks += x_offset;
        this.Pitch += y_offset;
    }

    Play(millisecondsPerTick, caller, onStopCallback, instrumentCode=InstrumentEnum.Flute)
    {
        var milliseconds = millisecondsPerTick * this.Duration

        var env;
        var synth;
        switch(instrumentCode)
        {
            //TODO: improve instruments
            case InstrumentEnum.Violin:
                env = T("perc", {a:655, r:milliseconds*1.5});
                synth  = T("PluckGen", {env:env, mul:0.75}).play();
                break;

            case InstrumentEnum.Guitar:
                env = T("perc", {a:100, r:milliseconds*1.0});
                synth  = T("PluckGen", {env:env, mul:0.75}).play();
                break;

            case InstrumentEnum.Flute:
                //fami, saw, tri, pulse, konami, cos, sin
                var table = [10,
                    [10, milliseconds/10], [50, milliseconds/9], [100, milliseconds/8],
                    [50, milliseconds/7], [150, milliseconds/6], [200, milliseconds/5],
                    [150, milliseconds/4], [100, milliseconds/3], [50, milliseconds/2],
                    [10, milliseconds]];
                env   = T("env", {table:table, loopNode:0}).bang();
                //env = T("perc", {a:250, ar:true, r:milliseconds*1.0});
                synth = T("OscGen", {env:env, wave: "konami", mul: 0.75 }).play();
                break;

            default:
                env = T("perc", {a:5, r:milliseconds*1.0});
                synth  = T("PluckGen", {env:env, mul:0.75}).play();
                break;
        }
        synth.noteOn(this.Pitch, 200);
		this.OnStopCallback = {Caller:caller, Callback: onStopCallback};
        this.IsHighlighted = true;

        this.PendingTimeout = setTimeout(
            $.proxy(this.StopPlaying, this),milliseconds);
    }

    StopPlaying()
    {
        this.IsHighlighted = false;
		this.OnStopCallback.Callback.call(this.OnStopCallback.Caller);
    }

    HorizontalModify(startTime,duration, sequenceNumber)
    {
        var initialState = this.CaptureState();

        this.Duration = duration;
        this.StartTimeTicks = startTime;
        if(!this.IsSelected)
        {
            var currentState = this.CaptureState();

            m_this.PushAction({
                Action:'MODIFY',
                SequenceNumber:sequenceNumber,
                GridIndex:m_this.GridPreviewIndex,
                MoveBuffer:[],
                MoveData:{
                    Note:this,
                    OriginalState:initialState,
                    TargetState:currentState
                }
            });
        }
    }

    OnMoveComplete(sequenceNumber)
    {
        var currentState = this.CaptureState();

        if(this.StateWhenSelected == null)
        {
            m_this.PushAction({
                Action:'ADD',
                SequenceNumber:sequenceNumber,
                GridIndex:m_this.GridPreviewIndex,
                MoveBuffer:[],
                MoveData:{
                    Note:this
                }
            });
        }

        else if(!this.StatesAreEqual(currentState, this.StateWhenSelected))
        {
            m_this.PushAction({
                Action:'MODIFY',
                SequenceNumber:sequenceNumber,
                GridIndex:m_this.GridPreviewIndex,
                MoveBuffer:[],
                MoveData:{
                    Note:this,
                    OriginalState:this.StateWhenSelected,
                    TargetState:currentState
                }
            });
        }

        this.StateWhenSelected = null;
    }

    StatesAreEqual(state1, state2)
    {
        var exactMatch =
            (state1.Pitch === state2.Pitch) &&
            (state1.StartTimeTicks === state2.StartTimeTicks) &&
            (state1.Duration === state2.Duration) &&
            (state1.GridIndex === state2.GridIndex);

        return exactMatch;
    }

    CaptureState()
    {
        var capturedState =
        {
            Pitch : this.Pitch,
            StartTimeTicks : this.StartTimeTicks,
            Duration : this.Duration,
            GridIndex : this.CurrentGridIndex
        }

        return capturedState;
    }

    RestoreState(capturedState)
    {
        var currentGridIndex = this.CurrentGridIndex;
        var selectedGridIndex = capturedState.GridIndex;

        if(currentGridIndex != selectedGridIndex)
        {
            this.HandleGridMoveReset(currentGridIndex,selectedGridIndex);
        }

        this.Pitch = capturedState.Pitch;
        this.StartTimeTicks = capturedState.StartTimeTicks;
        this.Duration = capturedState.Duration;
        this.CurrentGridIndex = selectedGridIndex;

        this.StateWhenSelected = null;
    }

    ResetPosition()
    {
        if(this.StateWhenSelected != null)
        {
            this.RestoreState(this.StateWhenSelected);
        }
    }

    set IsSelected(selected)
    {
        //When selecting an unselected note, capture its state
        if((this._IsSelected != selected) && (selected))
        {
            this.StateWhenSelected = this.CaptureState();
        }

        this._IsSelected = selected;
    }

    get IsSelected()
    {
        return this._IsSelected;
    }

	HandleGridMoveReset(currentGridIndex,selectedGridIndex)
	{
		var selectStartGridBuffer = m_this.GridPreviewList[selectedGridIndex];
		var currentGridBuffer = m_this.GridPreviewList[currentGridIndex];

		m_this.DeleteNote(this, 0, currentGridBuffer, false);
		m_this.AddNote(this, 0, selectStartGridBuffer, false);
	}



};

class Model
{
    constructor()
    {
        m_this = this;
        this.Score = [];
        this.GridPreviewList = [this.Score];
        this.GridImageList = [null]
        this.GridPreviewIndex = 0;
        this.ActivityStack = []
        this.ActivityIndex = 0;
        this.MaximumActivityStackLength = 100;
    }

    SetCurrentGridPreview(noteArray)
    {
        this.GridPreviewList[this.GridPreviewIndex] = noteArray;
    }

    GotoPreviousGrid()
    {
        if(m_this.GridPreviewIndex > 0)
        {
            m_this.GridPreviewIndex--;
            m_this.Score = m_this.GridPreviewList[m_this.GridPreviewIndex];
        }
    }

    GotoNextGrid()
    {
        if(m_this.GridPreviewIndex < m_this.GridPreviewList.length-1)
        {
            m_this.GridPreviewIndex++;
            m_this.Score = m_this.GridPreviewList[m_this.GridPreviewIndex];
        }
    }

    CreateGridPreview()
    {
        this.GridPreviewList.push([]);
        this.GridImageList.push([]);
    }

    InsertSorted(array, note)
    {
        array.push(note);
        array.sort(m_this.CompareNotes);
        //TODO: efficient sort
        //var arrayLength = array.length;
        //var index = m_this.BinarySearch(array, note, m_this.CompareNotes)
        //var index = m_this.LinearSearch(array,note);
        //array.splice( index, 0, note );
    }

    SortScoreByTicks()
    {
        m_this.Score.sort(m_this.CompareNotes);
    }

    LinearSearchOfScore(note)
    {
        return m_this.LinearSearch(m_this.Score, note)
    }

    //Return the index of an exact match, or the index where the element would be if it were present
    LinearSearch(array,searchNote)
    {
        var returnIndex2 = undefined;
        var lastCompareResult = undefined;
        m_this.console.log("Begin searching for ", searchNote);

        for(var returnIndex in array)
        {
            var otherNote = array[returnIndex];
            var compareResult = m_this.CompareNotes(searchNote, otherNote);

            //1: searchNote > otherNote: keep going
            //-1: searchNote < otherNoteotherNote: stop
            //0: searchNote == otherNote: return the index of an exact match (the pitch and duration are the same), or the index after all 'inexact' matches

            m_this.console.log("Searching, index " + returnIndex + ", cmp=" + compareResult, otherNote)
            if((compareResult === 0) || ((lastCompareResult != undefined) && (lastCompareResult != compareResult)))
            {
                if(returnIndex2 == undefined)
                {
                    m_this.console.log("Search complete", compareResult, otherNote);
                    returnIndex2 = returnIndex;
                }
            }

            lastCompareResult = compareResult;

        }

        m_this.console.log("Search complete. Index ="+returnIndex);
        return returnIndex2;
    }

    BinarySearch(array, element, compare_fn)
    {
        var m = 0;
        var n = array.length - 1;
        while (m <= n) {
            var k = (n + m) >> 1;
            var cmp = m_this.CompareNotes(element, array[k]);
            if (cmp > 0) {
                m = k + 1;
            } else if(cmp < 0) {
                n = k - 1;
            } else {
                return k;
            }
        }
        var returnIndex = -m - 1;
    }


    CompareNotes(note1, note2)
    {
        var nst1 = note1.StartTimeTicks;
        var nst2 = note2.StartTimeTicks;

        //Same note: return immediately
        if(note1 === note2)
        {
            return 0;
        }

        //Note 1 starts after note 2: place note 1 after note 2
        if(nst1 > nst2)
        {
            return 1;
        }

        //Note 1 starts before note 2: place note 1 before note 2
        else if(nst1 < nst2)
        {
            return -1;
        }

        //if the notes have the same start time, put longer duration notes after short duration notes
        //so that searches can find suspensions in adjacent notes more easily
        else
        {
            //Note 1 longer than note 2: place note 1 after note 2
            if(note1.Duration > note2.Duration)
            {
                return 1;
            }
            //Note 1 shorter: place note 1 before note 2
            else if(note1.Duration < note2.Duration)
            {
                return -1;
            }

            //Same duration: doesn't matter
            else
            {
                return 0;
            }
        }
    }

    HandleBatchInsertion(activityStack, action, targetString)
    {
        var stackLength = activityStack.length;
        var pushSuccessful = false;

        if((action.Action === targetString) && (stackLength > 0))
        {
            var stackTop = activityStack[stackLength - 1];

            //If this move is part of the same sequence number's move, combine it with the top of the stack
            //if((stackTop.Action === targetString) && (stackTop.SequenceNumber == action.SequenceNumber))
            if(stackTop.SequenceNumber == action.SequenceNumber)
            {
                stackTop.MoveBuffer.push(action.MoveData);
                pushSuccessful = true;
                m_this.console.log("Group "+action.Action + ": " + stackTop.MoveBuffer.length + " datums")
            }
        }

        return pushSuccessful;
    }

    PushAction(action)
    {
        var activityStack = m_this.ActivityStack;
        var stackLength = this.ActivityStack.length;
        var actionCases = ["MODIFY", "ADD", "DELETE"]
        var pushSuccessful = false;

        //If the index doesn't point to the end of the stack, dump all changes
        if(this.ActivityIndex != stackLength-1)
        {
            var resetIndex = this.ActivityIndex+1;
            m_this.console.log("Resetting stack up to and including index "+resetIndex);
            this.ActivityStack = this.ActivityStack.slice(0,resetIndex);
        }

        //Lose the last action if the stack is full
        if(stackLength >= this.MaximumActivityStackLength)
        {
            m_this.console.log("Maximum undo length reached. Discarding old state information.")
            this.ActivityStack.pop();
        }

        actionCases.forEach(function(caseString)
        {
            //If a group of actions are happening, push them together
            var pushedToBatch = m_this.HandleBatchInsertion(activityStack, action, caseString);
            if(pushedToBatch)
            {
                pushSuccessful = true;
                return;
            }
        });

        //If a distinct move is happening, push it separately
        if(!pushSuccessful)
        {
            action.MoveBuffer.push(action.MoveData);
            m_this.ActivityStack.push(action)
            m_this.console.log("Distinct "+action.Action +". New stack length: "+this.ActivityStack.length);
        }

        this.ActivityIndex = this.ActivityStack.length - 1;
        m_this.console.log("Push complete. Activity stack index: "+ this.ActivityIndex+"/"+(this.ActivityStack.length-1));
    }

    Undo()
    {
        if(this.ActivityIndex >= 0)
        {
            var mostRecentAction = this.ActivityStack[this.ActivityIndex];
            var moveBuffer = mostRecentAction.MoveBuffer;
            var gridBuffer = this.GridPreviewList[mostRecentAction.GridIndex];

            m_this.console.log("Undoing " + mostRecentAction.Action + " on " + moveBuffer.length + " notes, actionID = " + mostRecentAction.SequenceNumber);

            this.ActivityIndex--;

            //Undo the addition of a note by deleting it
            if(mostRecentAction.Action === 'ADD')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    m_this.DeleteNote(note, 0, gridBuffer, false)
                });
            }

            //Undo the deletion of a note by adding it
            else if(mostRecentAction.Action === 'DELETE')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    m_this.AddNote(note,  0, gridBuffer, false)
                });
            }

            //Undo a move by moving in the opposite direction
            else if(mostRecentAction.Action === 'MODIFY')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    var state = moveData.OriginalState;
                    note.RestoreState(state);
                });
            }

            gridBuffer.sort(m_this.CompareNotes);
            m_this.console.log("Undo complete. Activity stack index: "+ this.ActivityIndex+"/"+(this.ActivityStack.length-1));
        }

    }

    Redo()
    {
        if(this.ActivityIndex < this.ActivityStack.length-1)
        {
            this.ActivityIndex++;
            var mostRecentAction = this.ActivityStack[this.ActivityIndex]
            var moveBuffer = mostRecentAction.MoveBuffer;
            var gridBuffer = this.GridPreviewList[mostRecentAction.GridIndex];

            m_this.console.log("Redoing " + mostRecentAction.Action + " on " + moveBuffer.length + " notes, actionID = " + mostRecentAction.SequenceNumber);

            //Redo addition
            if(mostRecentAction.Action === 'ADD')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    m_this.AddNote(note, 0, gridBuffer, false)
                });
            }

            //Redo deletion
            else if(mostRecentAction.Action === 'DELETE')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    m_this.DeleteNote(note, 0, gridBuffer, false)
                });
            }

            //Redo a move
            else if(mostRecentAction.Action === 'MODIFY')
            {
                moveBuffer.forEach(function(moveData)
                {
                    var note = moveData.Note;
                    var state = moveData.TargetState;
                    note.RestoreState(state);

                    //var [startTimeDifference, pitchDifference] = moveData.Move;
                    //note.Move(startTimeDifference, pitchDifference);
                });
            }

            gridBuffer.sort(m_this.CompareNotes);
            m_this.console.log("Redo complete. Activity stack index: "+ this.ActivityIndex+"/"+(this.ActivityStack.length-1));
        }


    }

    //Public
    AddNote(note, sequenceNumber, array=this.Score, pushAction=true)
    {
        var gridIndex = this.GridPreviewIndex;

        if(pushAction)
        {
            this.PushAction({
                Action:'ADD',
                SequenceNumber:sequenceNumber,
                GridIndex:m_this.GridPreviewIndex,
                MoveBuffer:[],
                MoveData:{
                    Note:note
                }
            });
        }

        m_this.InsertSorted(array, note);
    }

    DeleteNoteWithIndex(deletionIndex, sequenceNumber, array=this.Score, pushAction=true)
    {
        var numberOfDeletions = 1;
        var deletedNote = this.Score[deletionIndex];
        var gridIndex = this.GridPreviewIndex;

        if(pushAction)
        {
			deletedNote.ResetPosition();
			deletedNote.IsSelected = false;
            this.PushAction({
                Action:'DELETE',
                SequenceNumber:sequenceNumber,
                GridIndex:m_this.GridPreviewIndex,
                MoveBuffer:[],
                MoveData:{
                    Note:deletedNote
                }
            });
        }

        array.splice(deletionIndex, numberOfDeletions)
    }

    DeleteNote(note, sequenceNumber, array=this.Score, pushAction=true)
    {
        for(var deletionIndex in array)
        {
            var otherNote = array[deletionIndex];
            if(otherNote === note)
            {
                break;
            }
        }

        m_this.DeleteNoteWithIndex(deletionIndex,sequenceNumber,array,pushAction)
    }
};
