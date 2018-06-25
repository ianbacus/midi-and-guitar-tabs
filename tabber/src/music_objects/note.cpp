#include "base.h"
#include "note.h"
#include <iostream>
#include <sstream>
#include <algorithm>


using namespace std;

using std::string;

int Note::OutOfRangeNotesCount = 0;
int32_t Note::TuningMinimum = 0;
int32_t Note::TuningMaximum = 0;
PitchMap Note::PitchToFretMap;
vector<string> Note::StringIndexedNoteNames;


/*
 *	Construct a note with pitch, delta, track number
 */
Note::Note(int pitch, int duration, int trackNumber) : 
    PitchMidiValue(pitch), 
    NoteDurationBeats(duration), 
    Repositions(0),PitchOffset(0),
    TrackNumber(trackNumber), CurrentPitchMapRepositionIndex(0) 
{
    
	if ((PitchToFretMap.find(pitch)) == PitchToFretMap.end()) 
	{
		OutOfRangeNotesCount++;
	}
}


/*
 *	Configure the pitch map: maps midi pitch values to pairs of string and fret positions.
 *	note index indicates which fret+string position is currently being used
 */
void Note::InitializePitchToFretMap(
        vector<string> stringIndexedNoteNames,
        vector<uint16_t> instrumentCoursePitchValues, 
        const uint32_t numberOfFrets,
        const uint32_t capoFret)
{
    PitchMap initMap;
    
    StringIndexedNoteNames = stringIndexedNoteNames;
    
    auto minimumElementIterator = 
        min_element(begin(instrumentCoursePitchValues),end(instrumentCoursePitchValues));
    
    auto maximumElementIterator = 
        max_element(begin(instrumentCoursePitchValues),end(instrumentCoursePitchValues));
    
    if(minimumElementIterator != end(instrumentCoursePitchValues))
    {
        TuningMinimum = (*minimumElementIterator)+capoFret;
    }
    
    if(maximumElementIterator != end(instrumentCoursePitchValues))
    {
        TuningMaximum = (*maximumElementIterator)+numberOfFrets;
    }
    
    const uint32_t numberOfStrings = instrumentCoursePitchValues.size();

    for(uint32_t stringIndex =0;stringIndex<numberOfStrings;stringIndex++)
    {
        
        //Assume that the frets are separated by a semitone
        uint32_t startFret = capoFret;
        


        for(uint32_t fretNumber = startFret; fretNumber < numberOfFrets; fretNumber++)
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
    
            cout << "wee" << endl;
        
    PitchToFretMap = initMap;
}

/*
 *	Print out initialization map
 */
string Note::PrintPitchMap(PitchMap pitchMap)
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
        
        outputStream << "\r\n";
	}
    
    string outputString = outputStream.str();

    cout << outputString << endl;
    
    return outputString;
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
    uint32_t fretAtIndex;
    uint32_t octaveCount = 0;
    
    uint32_t currentPitchMapRepositionIndex = notePositionEntry.RepositioningIndex;
    int32_t pitchMidiValue = notePositionEntry.PitchMidiValue;
    
	const bool success = GetPlayablePitch(octaveCount,pitchMidiValue);
    
    if(success)
    {
        fretAtIndex = PitchToFretMap[pitchMidiValue]
                    [currentPitchMapRepositionIndex].FretNumber;;
    }
    
    return fretAtIndex;
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
    uint32_t stringIndex;
    uint32_t octaveCount = 0;
    
    uint32_t currentPitchMapRepositionIndex = notePositionEntry.RepositioningIndex;
    int pitchMidiValue = notePositionEntry.PitchMidiValue;
    
    
	const bool success = GetPlayablePitch(octaveCount,pitchMidiValue);
    
    if(success)
    {
        stringIndex = PitchToFretMap[pitchMidiValue]
                    [currentPitchMapRepositionIndex].StringIndex;;
    }
    
    return stringIndex;
}


bool Note::GetPlayablePitch(uint32_t& numberOfOctaves, int32_t& pitchMidiValue) 
{
    uint32_t numberOfElements = 0;
    numberOfOctaves = 0;
    
    bool success = true;
    
    while(numberOfOctaves < MaximumNumberOfOctaves) 
    {
		try 
        {
			(void)PitchToFretMap.at(pitchMidiValue);
			break;
		}
        
        //If this pitch is not frettable, try moving its pitch up or down
		catch (const std::out_of_range& oor) 
        {
            numberOfOctaves++;
			if(pitchMidiValue<=TuningMinimum)
            {
                pitchMidiValue +=OctaveValueMidiPitches;
            }
            
			else if(pitchMidiValue>=TuningMaximum)
            {
                pitchMidiValue -=OctaveValueMidiPitches;	
            }
            
            else 
            {
                numberOfElements = 0;
                success = false;
                break;
            }
		}
	}
	return success;
}

/*
 *	Get track number note was taken from
 */
uint32_t Note::GetTrackNumber() const
{
	return TrackNumber;
}

uint32_t Note::GetPitchOffset() const
{
	return PitchOffset;
}

void Note::MakeNoteMorePlayable()
{
    const uint32_t pitchMidiValue = GetPitch();
    const uint32_t differenceFromTuningMinimum = abs((int)(pitchMidiValue - TuningMinimum));
    const uint32_t differenceFromTuningMaximum = abs((int)(pitchMidiValue - TuningMaximum));
    
    if(differenceFromTuningMinimum < differenceFromTuningMaximum)
    {
        PitchOffset += OctaveValueMidiPitches;
    }

    else
    {
        PitchOffset -= OctaveValueMidiPitches;	
    }
}

vector<FretboardPosition> Note::GetNotePositions() const
{
    uint32_t octaveCount = 0;
    
    int32_t pitchMidiValue = GetPitch();
    vector<FretboardPosition> notePositions;
    
	const bool success = GetPlayablePitch(octaveCount,pitchMidiValue);
    
    if(success)
    {
        notePositions = PitchToFretMap[pitchMidiValue];
    }
    
    return notePositions;
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
    
    notePosition.PitchMidiValue = GetPitch();
    notePosition.RepositioningIndex = CurrentPitchMapRepositionIndex;
    
    return notePosition;
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


string Note::PrintNote(NotePositionEntry notePositionEntry)
{
    string resultString = "|XX|";
    uint32_t octaveCount = 0;
    
    int32_t pitchMidiValue = notePositionEntry.PitchMidiValue;
    uint32_t currentPitchMapRepositionIndex = notePositionEntry.RepositioningIndex;
    
    const bool success = GetPlayablePitch(octaveCount,pitchMidiValue);
    
    if(success)
    {
        const FretboardPosition notePosition = PitchToFretMap[pitchMidiValue]
                                             [currentPitchMapRepositionIndex];
        
        const uint32_t stringIndex = notePosition.StringIndex;
        const uint32_t fretNumber = notePosition.FretNumber;
        const string stringName = StringIndexedNoteNames[stringIndex];
        
        stringstream resultStream;
        
        if(octaveCount > 1)
        {
            resultStream << "/";
        }
        
        else
        {
            resultStream << "|";
        }
        
        resultStream << stringName << std::hex << fretNumber << "|" << currentPitchMapRepositionIndex;
        resultString = resultStream.str();
    }
    
    return resultString;
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
    CurrentPitchMapRepositionIndex = 0;
    Repositions = 0;
}
/*
 *	Get the number of fret positions for this note. Try 10 different octave offsets
 */
uint32_t Note::GetNumberOfElements() const
{
    uint32_t numberOfElements = 0;
    uint32_t octaveCount = 0;
    int32_t adjustedPitch = GetPitch();
    
	const bool success = GetPlayablePitch(octaveCount,adjustedPitch);
    
    if(success)
    {
        numberOfElements = PitchToFretMap[adjustedPitch].size();
    }
    
    return numberOfElements;
}

/*
 *	Get the number of deleted notes (notes whose pitch was not mapped)
 */	
uint32_t Note::GetNotesLostCounterValue() 
{
	return OutOfRangeNotesCount;
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
	return PitchMidiValue+PitchOffset;
}


uint32_t Note::GetProximityToNearestTuningBoundary() const
{
    uint32_t proximityToNearestTuningBoundary = UINT32_MAX;
    uint32_t octave;
    int32_t pitchMidiValue = GetPitch();
    bool success = GetPlayablePitch(octave, pitchMidiValue);
    
    if(success)
    {
        const uint32_t differenceFromTuningMinimum = 
            abs((int)(pitchMidiValue - TuningMinimum));
        
        const uint32_t differenceFromTuningMaximum = 
            abs((int)(pitchMidiValue - TuningMaximum));
        
        proximityToNearestTuningBoundary = 
                min(differenceFromTuningMinimum,differenceFromTuningMaximum);
    }
    
    return proximityToNearestTuningBoundary;
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
uint32_t Note::GetNoteDurationBeats()  const
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
