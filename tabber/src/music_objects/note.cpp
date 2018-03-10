#include "base.h"
#include "tuning.h"

#define ACCEPTABLE  5
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tab matrix, update by incrementing 

/*
 *	Print out initialization map
 */
void print_initmap(std::map<int,vector< pair<int,int> > > initmap)
{
	for(std::map<int,vector< pair<int,int> > >::iterator iter = initmap.begin(); iter != initmap.end(); ++iter)
	{
		int k =  iter->first;
		//ignore value
		vector< pair<int,int> > v = iter->second;
		cout << k;
		for(auto p : v)
		{
			cout << "(" << p.first << "s," << p.second << "f), ";
		}
		cout << endl;
	}
}

/*
 *	Configure the pitch map: maps midi pitch values to pairs of string and fret positions.
 *	note index indicates which fret+string position is currently being used
 */
PitchMap config()
{
    PitchMap initmap;

    int value;

    for(int string_ind = SIZEOF_TUNING-1;string_ind>= 0;string_ind--)
    {        
        for(int fret_ind = 0; fret_ind<=12; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
				pair<int,int> map_point = make_pair(string_ind,fret_ind);
                
                //Insert sorted by fret number
                initmap[value].insert(std::upper_bound( initmap[value].begin(), initmap[value].end(), map_point, \
                [](const std::pair<int,int> &left, const std::pair<int,int> &right) { return left.second < right.second; }),map_point ); 
            }
            
        }
    }
    
    return initmap;
}

/*
 *	Construct a note with pitch, delta, track number
 */
Note::Note(int pitch, int duration, int trackNumber) : 
    PitchMidiValue(pitch), 
    NoteDurationBeats(duration), 
    Repositions(0),
    TrackNumber(trackNumber), CurrentPitchMapRepositionIndex(0) 
{
	//Negative delta values are used to represent triples
	//Negative pitch values represent rests
	if(pitch < 0)
	{
		//TODO: Handle triplets
	} 
    
	else if ((PitchToFretMap.find(pitch)) == PitchToFretMap.end()) 
	{
		DeletedNotesCount++;
	}
}

/*
 *	Get track number note was taken from
 */
int Note::get_track_num() const
{
	return TrackNumber;
}


/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
int Note::get_fret() const
{

    return get_fret_at(CurrentPitchMapRepositionIndex, PitchMidiValue);
}


/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
int Note::get_fret_at(const int currentPitchMapRepositionIndex, const int pitchMidiValue)
{    
    uint32_t fretAtIndex = 0;
    uint32_t offsetIndex = 0;
	int octaveIndex = 0;
    
	if(pitchMidiValue<0) 
    {
        return -1;
    }
    
	while(++octaveIndex < MaximumNumberOfOctaves) 
    {
		const uint32_t adjustedPitchMidiValue = pitchMidiValue+offsetIndex;
        
		try 
        {
			fretAtIndex = PitchToFretMap.at(adjustedPitchMidiValue)
                    [currentPitchMapRepositionIndex].second;
			break;
		}
        
		catch (const std::out_of_range& oor) 
        {
			if(pitchMidiValue<tuningMinimum)
            { 
                offsetIndex += OctaveValueMidiPitches;
            }
            
			else
            { 
                offsetIndex -= OctaveValueMidiPitches;
            }
		}
	}
    
    return fretAtIndex;
}


/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
int Note::get_string() const
{
    return get_string_at(CurrentPitchMapRepositionIndex, PitchMidiValue);
}

/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
int Note::get_string_at(
        int currentPitchMapRepositionIndex, 
        int pitchMidiValue)
{
	uint32_t stringAtIndex = 0;
    uint32_t offsetIndex = 0;
	int octaveIndex = 0;
    
	while(++octaveIndex < MaximumNumberOfOctaves) 
    {
		const uint32_t adjustedPitchMidiValue = pitchMidiValue+offsetIndex;
        
		try 
        {
			stringAtIndex = PitchToFretMap.at(adjustedPitchMidiValue)
                    [currentPitchMapRepositionIndex].first;
                    
            break;
                    
		}
        
		catch (const std::out_of_range& oor) 
        {
			if(pitchMidiValue<tuningMinimum)
            { 
                offsetIndex += OctaveValueMidiPitches;
            }
            
			else
            { 
                offsetIndex -= OctaveValueMidiPitches;
            }
		}
	}
    
    return stringAtIndex;
}

/*
 *	Rotate the note index forward: return to 0 if the note index exceeds the pitchmap entry
 */		
void Note::increment_note_index()
{
	const uint32_t numberOfPossiblePositions = GetNumberOfElements();
    
	CurrentPitchMapRepositionIndex = (CurrentPitchMapRepositionIndex+1)%
                                        (numberOfPossiblePositions);
    
    Repositions++;
}


uint32_t Note::GetAttemptedRepositions(void) const
{
    return Repositions;
}

void Note::ResetAttemptedRepositions(void)
{
    Repositions = 0;
}
/*
 *	Get the number of fret positions for this note. Try 10 different octave offsets
 */
int Note::GetNumberOfElements() const
{
	int ret,n=0;
	if(PitchMidiValue<0) return 0;
	int i = 0;
	while(++i<MaximumNumberOfOctaves) {
		try {
			ret = PitchToFretMap.at(PitchMidiValue+n).size();
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(PitchMidiValue<tuningMinimum){n +=12;}
			else{ n -=12;	}
		}
	}
}

/*
 *	Get the number of deleted notes (notes whose pitch was not mapped)
 */	
int Note::get_noteslost() const 
{
	return DeletedNotesCount;
}

/*
 *	Set note index for note fret+string positions
 */
void Note::set_note_index(int n) 
{
	CurrentPitchMapRepositionIndex = n;
}

/*
 *	Get note index for note fret+string positions
 */
int Note::get_current_note_index() const 
{
	return CurrentPitchMapRepositionIndex;
}
	
/*
 *	Get midi pitch value of note
 */	
int Note::get_pitch() const 
{
	return PitchMidiValue;
}

/*
 *	Transpose note by given interval (in semitones)
 */
void Note::alter_pitch(int n)
{
	PitchMidiValue +=n; 
}

/*
 *	Get note delta
 */
int Note::get_delta()  const
{
	return NoteDurationBeats;
}

/*
 *	Visit note
 */
void Note::DispatchVisitor(Visitor* v) 
{
	v->VisitNote(this);
}


//At compile time: generate the pitch to fret mapping
PitchMap Note::PitchToFretMap = config();
int Note::DeletedNotesCount = 0;

