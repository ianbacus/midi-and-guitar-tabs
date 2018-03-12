#include "base.h"
#include "note.h"
#include "tuning.h"
#include <iostream>
#include <sstream>
#include <algorithm>


using namespace std;

int Note::DeletedNotesCount = 0;
uint32_t Note::TuningMinimum = 0;
PitchMap Note::PitchToFretMap;
/*
 *	Configure the pitch map: maps midi pitch values to pairs of string and fret positions.
 *	note index indicates which fret+string position is currently being used
 */
PitchMap Note::GeneratePitchToFretMap(
        vector<uint16_t> instrumentCoursePitchValues, 
        const uint32_t numberOfFrets,
        const uint32_t capoFret)
{
    PitchMap initMap;
   
    auto minimumElementIterator = 
        min_element(begin(instrumentCoursePitchValues),end(instrumentCoursePitchValues));
    
    if(minimumElementIterator != end(instrumentCoursePitchValues))
    {
        TuningMinimum = (*minimumElementIterator)+capoFret;
    }
    
    else
    {
        TuningMinimum = 0;
    }
    
    const uint32_t numberOfStrings = instrumentCoursePitchValues.size();

    for(uint32_t stringIndex =0;stringIndex<numberOfStrings;stringIndex++)
    {        
        cout << "SI" << stringIndex << endl;
        //Assume that the frets are separated by a semitone
        for(uint32_t fretNumber = capoFret; fretNumber < numberOfFrets; fretNumber++)
        {
            const uint32_t pitchMidiValue = 
                instrumentCoursePitchValues[stringIndex] + fretNumber;
            
            FretboardPosition map_point(stringIndex,fretNumber);

            //Insert sorted by fret number
            initMap[pitchMidiValue].insert(
                upper_bound(begin(initMap[pitchMidiValue]), 
                end(initMap[pitchMidiValue]), map_point, 
                [](const FretboardPosition &left, const FretboardPosition &right) 
                { 
                    return left.FretNumber < right.FretNumber; 
                }), map_point); 
        }
    }
    PitchToFretMap = initMap;
    return initMap;
    
}

/*
 *	Print out initialization map
 */
string PrintPitchMap(PitchMap pitchMap)
{
    stringstream outputStream;
    
	for(auto KeyVal : pitchMap)
	{
		int k =  KeyVal.first;
        
		vector< FretboardPosition > v = KeyVal.second;
        
		outputStream << k;
		for(auto p : v)
		{
			outputStream << "(" << p.StringIndex << "s," << p.FretNumber << "f), ";
		}
	}
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
uint32_t Note::GetTrackNumber() const
{
	return TrackNumber;
}


/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
uint32_t Note::GetFretForCurrentNotePosition() const
{
    NotePositionEntry notePosition = GetCurrentNotePosition();
    
    return GetFretForNotePositionEntry(notePosition);
}

NotePositionEntry Note::GetCurrentNotePosition(void) const
{
    NotePositionEntry notePosition;
    
    notePosition.PitchMidiValue = PitchMidiValue;
    notePosition.RepositioningIndex = CurrentPitchMapRepositionIndex;
    
    return notePosition;
}

/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
uint32_t Note::GetFretForNotePositionEntry(
        NotePositionEntry notePositionEntry)
{
    
    uint32_t currentPitchMapRepositionIndex = notePositionEntry.RepositioningIndex;
    int32_t pitchMidiValue = notePositionEntry.PitchMidiValue;
    
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
                    [currentPitchMapRepositionIndex].FretNumber;
			break;
		}
        
		catch (const std::out_of_range& oor) 
        {
			if(pitchMidiValue<TuningMinimum)
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
uint32_t Note::GetStringIndexForCurrentNotePosition() const
{
    NotePositionEntry notePosition = GetCurrentNotePosition();
    
    return GetStringForNotePositionEntry(notePosition);
}

/*
 *	Get string for a given note index and pitch. The pitch is used to index
 *  into the main fretboard map, and the note index is used to determine which
 *  of the possible fretboard positions is currently in use.
 * 
 */
uint32_t Note::GetStringForNotePositionEntry(
        NotePositionEntry notePositionEntry)
{
    
    uint32_t currentPitchMapRepositionIndex = notePositionEntry.RepositioningIndex;
    uint32_t pitchMidiValue = notePositionEntry.PitchMidiValue;
        
	uint32_t stringAtIndex = 0;
    uint32_t offsetIndex = 0;
	int octaveIndex = 0;
    
	while(++octaveIndex < MaximumNumberOfOctaves) 
    {
		const uint32_t adjustedPitchMidiValue = pitchMidiValue+offsetIndex;
        
		try 
        {
			stringAtIndex = PitchToFretMap.at(adjustedPitchMidiValue)
                    [currentPitchMapRepositionIndex].StringIndex;
                    
            break;
                    
		}
        
		catch (const std::out_of_range& oor) 
        {
			if(pitchMidiValue<TuningMinimum)
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
void Note::ReconfigureToNextPitchmapPosition()
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
uint32_t Note::GetNumberOfElements() const
{
    int32_t noteOffset=0;
    uint32_t numberOfElements = 0;
	
    
	int octaveOffsetCount = 0;
	while(++octaveOffsetCount < MaximumNumberOfOctaves) 
    {
		try 
        {
			numberOfElements = PitchToFretMap.at(PitchMidiValue+noteOffset).size();
			break;
		}
        
        //If this pitch is not frettable, try moving its pitch up or down
		catch (const std::out_of_range& oor) 
        {
			if(PitchMidiValue<TuningMinimum)
            {
                noteOffset +=12;
            }
            
			else if(PitchMidiValue>TuningMinimum)
            {
                noteOffset -=12;	
            }
            
            else 
            {
                numberOfElements = 0;
                break;
            }
		}
	}
    
    return numberOfElements;
}

/*
 *	Get the number of deleted notes (notes whose pitch was not mapped)
 */	
uint32_t Note::GetNotesLostCounterValue() const 
{
	return DeletedNotesCount;
}

/*
 *	Set note index for note fret+string positions
 */
void Note::SetPitchmapPositionIndex(uint32_t n) 
{
	CurrentPitchMapRepositionIndex = n;
}

/*
 *	Get note index for note fret+string positions
 */
uint32_t Note::GetCurrentPitchmapIndex() const 
{
	return CurrentPitchMapRepositionIndex;
}
	
/*
 *	Get midi pitch value of note
 */	
uint32_t Note::GetPitch() const 
{
	return PitchMidiValue;
}

/*
 *	Transpose note by given interval (in semitones)
 */
void Note::AddOffsetToPitchMidiValue(int offset)
{
	PitchMidiValue += offset; 
}

/*
 *	Get note delta
 */
int Note::GetNoteDurationBeats()  const
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
