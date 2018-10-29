
class Note
{
    constructor(pitch, startTimeTicks, endTimeTicks)
    {
        this.Pitch = pitch
        this.StartTimeTicks = startTimeTicks
        this.EndTimeTicks = endTimeTicks
        this.IsSelected = false
    }

    get Duration()
    {
        return this.EndTimeTicks - this.StartTimeTicks;
    }

};

class Model
{
    //Private
    InsertSorted(array, note)
    {
        array.push(note);
        array.Sort();

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
        this.Score = [];
        console.log("model");
    }

    Initialize()
    {

    }
    
    AddNote(note)
    {
        this.InsertSorted(this.Score, note);

        return id;
    }

    DeleteNote(note)
    {
        var numberOfDeletions = 1;
        var deletionIndex = BinarySearch(this.Score, note)
        this.Score.splice(deletionIndex, numberOfDeletions)
    }

    get FullScore()
    {
        return this.Score
    }

}
