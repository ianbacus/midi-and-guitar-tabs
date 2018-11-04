let m_this = undefined;

var pitchKey = [

18, 19, 20, 21, 23, 24, 25, 27, 29, 30.86, 32.7, 32, 36, 38, 41, 43, 46, 48, 51, 55, 58, 61, 65, 69, 73, 77, 82, 87, 92, 103, 110, 116, 123, 130, 138, 146, 155, 164, 174, 184, 195, 207, 220, 233, 246, 261,

    261.626,277.183,293.665,311.127,
    329.628,349.228,369.994,391.995,
    415.305,440.000,466.164,493.883,

    523.251,554.365,587.330,622.254,
    659.255,698.456,739.989,783.991,
    830.609,880.000,932.328,987.767,

    1046.50,1108.73,1174.66,1244.51,
    1318.51,1396.61,1479.98,1567.98];

var Synthesizer = T("OscGen",
{
    //wave: "fami",
    wave: "fami",

    //fami, saw, tri, pulse, konami, cos, sin
    mul: 0.25

}).play();

class Note
{
    constructor(startTimeTicks, pitch, duration, selected)
    {
        this.Pitch = pitch;
        this.StartTimeTicks = startTimeTicks;
        this.Duration = duration;
        this._IsSelected = selected;
        this.SelectedPitchAndTicks = null;
    }

    ExactMatch(otherNote)
    {
        var exactMatch = otherNote === this;
            //console.log("Exact match?", exactMatch, otherNote, this)

        return exactMatch;
        /*
        this.Pitch = otherNote.Pitch;
        this.StartTimeTicks = otherNote.StartTimeTicks;
        this.Duration = otherNote.Duration;
        this._IsSelected = otherNote._IsSelected;
        this.SelectedPitchAndTicks = otherNote.SelectedPitchAndTicks
        */
    }

    Move(x_offset, y_offset)
    {
        this.StartTimeTicks += x_offset;
        this.Pitch += y_offset;
    }

    Play()
    {
        var numberOfPitches = pitchKey.length;
        var pitchIndex = this.Pitch % numberOfPitches;

        Synthesizer.noteOnWithFreq(pitchKey[pitchIndex], 200);
    }

    set IsSelected(selected)
    {
        this._IsSelected = selected;
        if(selected)
        {
            console.log("marked position")
            this.SelectedPitchAndTicks = [this.Pitch, this.StartTimeTicks]
        }
    }

    get IsSelected()
    {
        return this._IsSelected;
    }

    ResetPosition()
    {
        if(this.SelectedPitchAndTicks != null)
        {
            [this.Pitch,this.StartTimeTicks] = this.SelectedPitchAndTicks;
        }
    }


};

class Model
{
    //Private
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



    LinearSearchOfScore(note)
    {
        return m_this.LinearSearch(m_this.Score, note)
    }

    //Return the index of an exact match, or the index where the element would be if it were present
    LinearSearch(array,searchNote)
    {
        var returnIndex2 = undefined;
        var lastCompareResult = undefined;
        console.log("Begin searching for ", searchNote);

        for(var returnIndex in array)
        {
            var otherNote = array[returnIndex];
            var compareResult = m_this.CompareNotes(searchNote, otherNote);

            //1: searchNote > otherNote: keep going
            //-1: searchNote < otherNoteotherNote: stop
            //0: searchNote == otherNote: return the index of an exact match (the pitch and duration are the same), or the index after all 'inexact' matches

            console.log("Searching, index " + returnIndex + ", cmp=" + compareResult, otherNote)
            if((compareResult === 0) || ((lastCompareResult != undefined) && (lastCompareResult != compareResult)))
            {
                if(returnIndex2 == undefined)
                {
                    console.log("Search complete", compareResult, otherNote);
                    returnIndex2 = returnIndex;
                }
            }

            lastCompareResult = compareResult;

        }

        console.log("Search complete. Index ="+returnIndex);
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

        if(note1 === note2)
        {
            return 0;
        }
        if(nst1 > nst2)
        {
            return 1;
        }
        else if(nst1 < nst2)
        {
            return -1;
        }
        else {
            return 0;
        }
    }

    //Public
    constructor()
    {
        m_this = this;
        m_this.Score = [];
    }

    AddNote(note)
    {
        m_this.InsertSorted(m_this.Score, note);
    }

    DeleteNote(note)
    {
        var array = m_this.Score;

        for(var deletionIndex in array)
        {
            var otherNote = array[deletionIndex];
            if(otherNote === note)
            {
                break;
            }
        }
        m_this.DeleteNoteWithIndex(deletionIndex)
    }

    DeleteNoteWithIndex(deletionIndex)
    {
        var numberOfDeletions = 1;
        m_this.Score.splice(deletionIndex, numberOfDeletions)
    }

};
