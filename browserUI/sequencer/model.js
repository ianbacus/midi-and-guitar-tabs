let m_this = undefined;

var noteInstance;

var pitchKey = [
    261.626,277.183,293.665,311.127,
    329.628,349.228,369.994,391.995,
    415.305,440.000,466.164,493.883,

    523.251,554.365,587.330,622.254,
    659.255,698.456,739.989,783.991,
    830.609,880.000,932.328,987.767,

    1046.50,1108.73,1174.66,1244.51,
    1318.51,1396.61,1479.98,1567.98];

var Synthesizer = T("OscGen", {
    wave: "cos",
    //fami, saw, tri, pulse, konami, cos, sin
    mul: 0.5
    }).play();

class Note
{
    constructor(pitch, startTimeTicks, duration)
    {
        noteInstance = this;
        noteInstance.Pitch = pitch;
        noteInstance.StartTimeTicks = startTimeTicks;
        noteInstance.Duration = duration;
        noteInstance.IsSelected = false;
    }

    Move(startTimeTicks, pitch)
    {
        noteInstance.Pitch = pitch;
        noteInstance.StartTimeTicks = startTimeTicks;
    }

    Play()
    {
        var pitchIndex = pitch % 32;
        Synthesizer.noteOnWithFreq(pitchKey[pitchIndex], 200);
    }

};

class Model
{
    //Private
    InsertSorted(array, note)
    {
        array.push(note);
        //array.Sort();

        //TODO: efficient sort
        //var arrayLength = array.length;
        //BinarySearch(array, note, 0, arrayLength)
        //array.splice( index, 0, note );
    }

    BinarySearch(array, element, compare_fn)
    {
        var m = 0;
        var n = array.length - 1;
        while (m <= n) {
            var k = (n + m) >> 1;
            var cmp = compare_fn(element, array[k]);
            if (cmp > 0) {
                m = k + 1;
            } else if(cmp < 0) {
                n = k - 1;
            } else {
                return k;
            }
        }
        return -m - 1;
    }

    CompareNotes(note1, note2)
    {
        if(note1.StartTimeTicks > note2.StartTimeTicks)
        {
            return 1;
        }
        else if(note1.StartTimeTicks < note2.StartTimeTicks)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }


    //Public
    constructor()
    {
        m_this = this;
        m_this.Score = [];
        console.log("model");
    }

    AddNote(note)
    {
        m_this.InsertSorted(m_this.Score, note);
        console.log("added note");

    }

    DeleteNote(note)
    {
        var deletionIndex = BinarySearch(m_this.Score, note, CompareNotes)
        DeleteNoteWithIndex(deletionIndex)
    }

    DeleteNoteWithIndex(deletionIndex)
    {
        var numberOfDeletions = 1;
        m_this.Score.splice(deletionIndex, numberOfDeletions)
    }

};
