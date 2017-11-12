#include "base.h"

#include "note.h"
#include "bar.h"
#include "chunk.h"

#include "tuning.h"

#define ACCEPTABLE  5
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 

//At compile time: generate the pitch to fret mapping
PitchMap Note::pitch_to_frets_map = ConfigurePitchMap();
int Note::noteslost = 0;



/*
 *	Construct a note with pitch, delta, track number
 */
Note::Note(int pitch, int delta, int trackNumber) : Pitch(pitch), Delta(delta), TrackNumber(trackNumber), NotePositionPitchMapIndex(0) 
{
	//Negative delta values are used to represent triples
	//Negative pitch values represent rests

	if ( (pitch >= 0) && (pitch_to_frets_map.find(pitch)) == pitch_to_frets_map.end()) 
	{
		noteslost++;
	}
}

Note::~Note(void)
{
    //Nothing
}

/*
 *	Get track number note was taken from
 */
int Note::get_track_number() const
{
	return TrackNumber;
}

/*
 *	Get fret for the current note index
 */
int Note::get_fret(void) 
{
    int pitch = get_pitch();
	return get_fret_at(NotePositionPitchMapIndex, pitch);
}

/*
 *	Get the fret for a given note index and pitch
 */
int Note::get_fret_at(int note_index, int pitch)
{
	int fret = -1;
    
	if(pitch >= 0)
    {
        int temporaryNoteOffset = 0;
        int maximumOctavesCount = MaximumOctavesCount;
        
        while(maximumOctavesCount-- > 10) 
        {
            try 
            {
                fret = pitch_to_frets_map.at(pitch+temporaryNoteOffset)[note_index].second;
                return fret;
            }
            
            catch (const std::out_of_range& oor) 
            {
                if(pitch < tuningMinimum)
                { 
                    temporaryNoteOffset += PitchOctave;
                }
                
                else
                { 
                    temporaryNoteOffset -= PitchOctave;
                }
            }
        }
    }
    
    return fret;
}

/*
 *	Get string for the current note index
 */
int Note::get_string() 
{
    return get_string_at(NotePositionPitchMapIndex, Pitch);
    
}

/*
 *	Get string for a given note index and pitch
 */
int Note::get_string_at(int note_index, int pitch)
{
	int noteString = 0;
    int noteTemporaryOffset = 0;
    
	int maximumOctavesCount = MaximumOctavesCount;
        
    while(maximumOctavesCount-- > 10) 
    {
		try 
        {
			noteString = pitch_to_frets_map.at(pitch+noteTemporaryOffset)[note_index].first;
            
			return noteString;
		}
        
		catch (const std::out_of_range& oor) 
        {
			if(pitch < tuningMinimum)
            {
                noteTemporaryOffset += PitchOctave;
            }
            
			else
            {
                noteTemporaryOffset -= PitchOctave;
            }
		}
	}
    
    return noteString;
}

/*
 *	Get the number of fret positions for this note. Try 10 different octave offsets
 */
int Note::get_children_size() const
{
    int childrenSize = 0;
    int pitch = get_pitch();
    
	if(pitch >= 0) 
    {
        int noteTemporaryOffset = 0;
        
        int maximumOctavesCount = MaximumOctavesCount;
            
        while(maximumOctavesCount-- > 10) 
        {
            try 
            {
                childrenSize = pitch_to_frets_map.at(pitch+noteTemporaryOffset).size();
                break;
            }
            
            catch (const std::out_of_range& oor) 
            {
                if(pitch<tuningMinimum)
                {
                    noteTemporaryOffset += PitchOctave;
                }
                
                else
                { 
                    noteTemporaryOffset -= PitchOctave;	
                }
            }
        }
    }
    
    return childrenSize;
}

/*
 *	Transpose a note down by one octave
 */
void Note::decrement_octave()
{
    Pitch -= PitchOctave;
    NumberOfOctaveOffsets--;
}

/*
 *	Transpose a note up by one octave
 */
void Note::increment_octave()
{
    Pitch += PitchOctave;
    NumberOfOctaveOffsets++;
}

/*
 *	Return a note to its original position
 */
void Note::rebalance_note()
{
    Pitch -= NumberOfOctaveOffsets*PitchOctave;
    NumberOfOctaveOffsets = 0;
}

/*
 *	Rotate the note index forward: return to 0 if the note index exceeds the pitchmap entry
 */		
void Note::increment_note_index()
{
	int sizetemp = get_children_size();
	NotePositionPitchMapIndex = (NotePositionPitchMapIndex+1)%(sizetemp);
}

/*
 *	Get the number of notes that were attempted to be used which did not exist in the pitch map
 */	
int Note::get_noteslost() const 
{
	return noteslost;
}

/*
 *	Set note index for note fret+string positions
 */
void Note::set_note_index(int n) 
{
	NotePositionPitchMapIndex = n;
}

/*
 *	Get note index for note fret+string positions
 */
int Note::get_note_position_pitch_map_index() const 
{
	return NotePositionPitchMapIndex;
}
	
/*
 *	Get midi pitch value of note
 */	
int Note::get_pitch() const 
{
	return Pitch;
}

/*
 *	Transpose note by given interval (in semitones)
 */
void Note::alter_pitch(int n)
{
	Pitch +=n; 
}

/*
 *	Get note delta
 */
int Note::get_delta()  const
{
	return Delta;
}

/*
 *	Visit note
 */
void Note::accept(Visitor* visitor) 
{
    if(visitor != nullptr)
    {
        visitor->VisitNote(this);
    }
}

/*
 *	Configure the pitch map: maps midi pitch values to pairs of string and fret positions.
 *	note index indicates which fret+string position is currently being used
 */
PitchMap Note::ConfigurePitchMap()
{
    PitchMap initialMap;

    int value;

    for(int string_ind = SIZEOF_TUNING-1;string_ind>= 0;string_ind--)
    {
        for(int fret_ind = 0; fret_ind<14; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
				pair<int,int> map_point = make_pair(string_ind,fret_ind);
                
                //Insert sorted by fret number
                initialMap[value].insert(std::upper_bound( initialMap[value].begin(), initialMap[value].end(), map_point, 
                [](const std::pair<int,int> &left, const std::pair<int,int> &right) { return left.second < right.second; }),
                map_point ); 
            }
            
        }
    }
    
    return initialMap;
}


