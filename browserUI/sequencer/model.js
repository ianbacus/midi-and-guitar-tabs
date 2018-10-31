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
    wave: "konami",
    //fami, saw, tri, pulse, konami, cos, sin
    mul: 0.5

}).play();

class Note
{
    constructor(startTimeTicks, pitch, duration)
    {
        this.Pitch = pitch;
        this.StartTimeTicks = startTimeTicks;
        this.Duration = duration;
        this.IsSelected = false;

    }

    Move(startTimeTicks, pitch)
    {
        this.Pitch = pitch;
        this.StartTimeTicks = startTimeTicks;
    }

    Play()
    {
        var numberOfPitches = pitchKey.length;
        var pitchIndex = this.Pitch % numberOfPitches;

        console.log("b",this.Pitch);
        Synthesizer.noteOnWithFreq(pitchKey[pitchIndex], 200);
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

    LinearSearch(array,element)
    {
        console.log(element)

        var returnIndex = array.length;
        var k = 0;

        console.log("Inserting:");
        console.log(element);

        console.log("Into:");
        array.forEach(function(note)
        {
            console.log(note);
        });
        console.log(".");

        array.forEach(function(note)
        {
            var cmp = m_this.CompareNotes(element, array[k]);
            if (cmp <= 0)
            {
                console.log("found insertion point"+k)
                returnIndex = k;
            }
            k += 1;
        })

        console.log("Sorted:");
        array.forEach(function(note)
        {
            console.log(note);
        });
        console.log(".");
        return k;
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

        if(nst1 > nst2)
        {
            //console.log(nst1+">"+nst2)
            return 1;
        }
        else if(nst1 < nst2)
        {
            //console.log(nst1+"<"+nst2)
            return -1;
        }
        else
        {
            //console.log(nst1+"==="+nst2)
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
        var deletionIndex = m_this.LinearSearch(m_this.Score, note, m_this.CompareNotes)
        m_this.DeleteNoteWithIndex(deletionIndex)
    }

    DeleteNoteWithIndex(deletionIndex)
    {
        var numberOfDeletions = 1;
        m_this.Score.splice(deletionIndex, numberOfDeletions)
    }

};
