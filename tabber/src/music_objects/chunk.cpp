#include "base.h"
#include <cmath>
#include <set>
#include <unordered_map>
#include <sstream>
#include <assert.h>

using namespace std;

string Chunk::PrintNoteIndices(vector<NotePositionEntry > noteConfiguration)
{
    std::stringstream sstream;
    
    //Print the fret and string of each note in the current chunk configuration	
	for(NotePositionEntry notePositionEntry: noteConfiguration)
	{
		sstream << Note::PrintNote(notePositionEntry)<<" ";;
	}
    
    return sstream.str();
}


Chunk::Chunk(int d,uint32_t measureIndex) 
    : 
        Delta(d),
        MeasureIndex(measureIndex),
        IsOptimized(false),
        MeasureStart(false),
        PreviousChunk(nullptr),
        NextChunk(nullptr)
{

}

Chunk::~Chunk()
{		
    
	for (std::vector< Note* >::iterator it = Notes.begin() ; 
            it != Notes.end(); ++it)
    {
		delete (*it);
    }
    
	Notes.clear();
}

/* 
 *	Get length of chord/chunk (in notes)
 */
uint32_t Chunk::GetNumberOfElements() const 
{
	return Notes.size();
}

/* 
 *	Generate a vector of <note-index, pitch> pairs. The note index indicates 
 *	which fretboard position the pitch is currently set to
 */
vector<NotePositionEntry > Chunk::GetCurrentNotePositionEntries(void) const
{
	vector<NotePositionEntry> indices;
    const uint32_t numberOfNotes = GetNumberOfElements();
    
	for(uint32_t i=0; i<numberOfNotes; i++)
    {
        Note const * const note = GetElementWithIndex(i);
        
        assert(note != nullptr);
        
        if(note != nullptr)
        {
            NotePositionEntry noteData;
            noteData.PitchMidiValue = note->GetPitch();
            noteData.RepositioningIndex = note->GetCurrentPitchmapIndex();

            indices.push_back(noteData);
        }
	}
    
	return indices;
}

vector<FretboardPosition> Chunk::GetCurrentFretboardPositions()
{
    vector<NotePositionEntry> currentNotePositions =
        GetCurrentNotePositionEntries();
    
    vector<FretboardPosition> fretboardPositions;
    
    for(NotePositionEntry notePositionEntry : currentNotePositions)
    {
        const uint32_t currentString = 
            Note::GetStringForNotePositionEntry(notePositionEntry);
        
        const uint32_t currentFret = 
            Note::GetFretForNotePositionEntry(notePositionEntry);
        
        const FretboardPosition fretPosition(currentString,currentFret);

        fretboardPositions.push_back(fretPosition);
    }
    
    return fretboardPositions;
}


vector<NotePositionEntry > Chunk::GetCurrentOptimalNotePositionEntries(void) const
{
    return CurrentOptimalNotePositionEntries;
}

/* 
 *	Dispatch a Visitor on a chunk
 */
void Chunk::DispatchVisitor(Visitor* v) 
{
	v->VisitChunk(this);
}


bool IsChunkConfigurable(Chunk* chunk) 
{
    bool chunkValid = true;
    vector<Note*> chunkNotes = chunk->GetElements();
    
    unordered_map<uint32_t, uint32_t> stringToOverlapCountMap;
    
    
    //Pigeon-hole principle: if there are more notes than fretted strings, in
    //all possible configurations, this chunk cannot be reconfigured
    
    for (Note* note : chunkNotes) 
    {
        vector<FretboardPosition> notePositions = note->GetNotePositions();
        
        if(notePositions.size() == 1)
        {
            uint32_t stringIndex = notePositions.back().StringIndex;
            stringToOverlapCountMap[stringIndex]++;

            chunkValid = stringToOverlapCountMap[stringIndex] <= 1;
        }
        
        if(!chunkValid)
        {
            break;
        }
    }
    
    for (auto kv : stringToOverlapCountMap)
    {
    }
    
    return chunkValid;
}

bool ChunkIsValid(Chunk* chunk) 
{

    bool chunkValid = true;
    vector<Note*> chunkNotes = chunk->GetElements();
    uint32_t numberOfNotes = chunkNotes.size();

    set<uint32_t> stringsUsed;

    //Pigeon-hole principle: if there are more notes than fretted strings, in
    //all possible configurations, this chunk cannot be reconfigured
    
    for (Note* note : chunkNotes) 
    {
        vector<FretboardPosition> notePositions = note->GetNotePositions();

        for (FretboardPosition notePosition : notePositions) 
        {
            uint32_t stringIndex = notePosition.StringIndex;

            stringsUsed.insert(stringIndex);
        }
    }
    
    uint32_t numberOfStringsUsed = stringsUsed.size();

    if (numberOfStringsUsed < numberOfNotes) 
    {
        chunkValid = true;
    }
    
    return chunkValid;
}

uint32_t Chunk::GetAveragePitch(void)
{
    vector<Note*> chunkNotes = GetElements();
    uint32_t chunkSize = chunkNotes.size();
    uint32_t averagePitch = 0;
    
    if(chunkSize > 0)
    {
        //Find average pitch in this note
        for(Note* note : chunkNotes)
        {
            uint32_t octave = 0;
            int32_t pitch = 0;
            note->GetPlayablePitch(octave,pitch);
            averagePitch += pitch;
        }

        averagePitch = averagePitch/chunkSize;
    }
    return averagePitch;
}

//add extra note positions for middle-voice notes with note position entries
//of size 1 
void Chunk::CleanChunk(void)
{
    while(!IsChunkConfigurable(this))
    {
        vector<Note*> chunkNotes = GetElements();
        uint32_t chunkSize = chunkNotes.size();
        
        if(chunkSize > 0)
        {            
            uint32_t averagePitch = GetAveragePitch();
            
            sort(begin(chunkNotes),end(chunkNotes), [averagePitch](const Note* left, const Note* right)
            {
                uint32_t lhsPositions = left->GetNumberOfElements();
                uint32_t rhsPositions = right->GetNumberOfElements();
                
                //Make sure the center-most notes come first
                if(lhsPositions == rhsPositions)
                {
                    return abs((double)((left->GetPitch()) - averagePitch)) < 
                           abs((double)(right->GetPitch() - averagePitch));
                }
                
                //Make sure the more immobile notes come first
                else
                {
                    return lhsPositions < rhsPositions;
                }
            });
            
            Note* chosenNote = chunkNotes[0];
            
            //Make the least playable note more playable by shifting it up an octave
            chosenNote->MakeNoteMorePlayable();
        }
    }
}


bool Chunk::CheckIfNoteIsAlreadyPresent(Note* note)
{
    uint32_t octaves = 0;
    int32_t candidateNotePitch = note->GetPitch();

    bool duplicateNote = false;
    bool success = Note::GetPlayablePitch(octaves, candidateNotePitch);

    //Filter out duplicate notes
    if(success)
    {
        vector<Note*> chunkNotes = GetElements();
        for(Note* note : chunkNotes)
        {
            int32_t pitch = note->GetPitch();
            Note::GetPlayablePitch(octaves, pitch);

            //todo: some notes bypass this
            duplicateNote = (candidateNotePitch==pitch);
            if(duplicateNote)
            {
                break;
            }
        }
    }
    
    return duplicateNote;
}

/* 
 *	Insert a note into the vector of notes. Insert in sorted order by the number of frettable
 *	positions for that note.
 */
void Chunk::PushElement(Note* note) 
{
    if(note != nullptr)
    {
        
        bool duplicateNote = CheckIfNoteIsAlreadyPresent(note);
        //CleanChunk();
        
        if(!duplicateNote)
        {
            //insert at iterator to first element with a pitch-map entry larger than the candidate note
            Notes.insert(std::upper_bound( Notes.begin(), Notes.end(), note,  
                [](Note *a, Note*b) 
                { 															 
                    return (a->GetNumberOfElements() < b->GetNumberOfElements()); 				 
                }),
                note); 
        }   
    }
}

/* 
 *	Remove a given note from the chunk
 */
void Chunk::RemoveElement(Note* note) 
{
	Notes.erase(std::remove(Notes.begin(), 
                    Notes.end(), note), Notes.end()); 
}


vector<Note*> Chunk::GetElements() const
{
    return Notes;
}

/* 
 *	Get ith note in this chunk
 */
Note* Chunk::GetElementWithIndex(uint32_t noteIndex) const
{
    Note *note = nullptr;
    
    if(noteIndex<Notes.size())
    {
        note = Notes[noteIndex];
    }
    
    return note;
}

/* 
 *	Get last note in this chunk, this note will have the most fret+string positions
 */
Note* Chunk::GetMostMobileNote(void) const
{
    Note *note = nullptr;
    
    if(0 < Notes.size())
    {
        note = Notes.back();
    }
    
    return note;
}

/* 
 *	Get the offset of this chunk from the previous one
 */
int Chunk::GetDelta(void) const 
{
	return Delta;
}

bool Chunk::GetIsMeasureEnd(void)
{
    return MeasureStart;
}

uint32_t Chunk::GetNumberOfPositionPermutations(void) const
{
    const uint32_t numberOfNotes = GetNumberOfElements();
    uint32_t notePermutations = 1;
    
    for(uint32_t index = 0; index<numberOfNotes; index++)
    {
        Note const * const note = GetElementWithIndex(index);
        
        if(note != nullptr)
        {
            notePermutations *= note->GetNumberOfElements();
        }
    }
    
    return notePermutations;
}

void Chunk::SetIsOptimized(bool isOptimized)
{
    IsOptimized = isOptimized;
}

void Chunk::SetPreviousChunk(Chunk* previousChunk)
{
    //Only change the previous chunk if it has not been set
    if(PreviousChunk == nullptr)
    {
        PreviousChunk = previousChunk;
    }
}

void Chunk::SetNextChunk(Chunk* nextChunk)
{
    //Only change the next chunk if it has not been set
    if(NextChunk == nullptr)
    {
        NextChunk = nextChunk;
    }
}

void Chunk::SetIsMeasureEnd(bool isMeasureStart)
{
    MeasureStart = isMeasureStart;
}

//Set locked strings without clearing others
void Chunk::SetLockedStringIndices(vector<FretboardPosition> sustainedStrings)
{    
    const vector<FretboardPosition> currentFretboardPositions = 
        GetCurrentFretboardPositions();
    
    
    for(FretboardPosition fretboardPosition : sustainedStrings)
    {
        for(FretboardPosition currentFretboardPosition : currentFretboardPositions)
        {
            const uint32_t currentStringIndex = currentFretboardPosition.StringIndex;
            const uint32_t previousStringIndex = fretboardPosition.StringIndex;
            
            if(currentStringIndex == previousStringIndex)
            {
                SustainedFretboardPositions.push_back(currentFretboardPosition);
            }
        }
    }
}

Chunk* Chunk::GetPreviousChunk(void)
{
    return PreviousChunk;    
}

Chunk* Chunk::GetNextChunk(void)
{
    return NextChunk;
}

bool Chunk::GetIsOptimized(void)
{
    return IsOptimized;
}
        
uint32_t Chunk::GetMeasureIndex(void)
{
    return MeasureIndex;
}


vector<FretboardPosition> Chunk::GetSustainedFretboardPositions(void)
{
    return SustainedFretboardPositions;
}
        

/* 
 *	Get the number of fret+string positions in the current optima
 */
uint32_t Chunk::GetOptimalChunkSize(void)  const
{ 
	return CurrentOptimalNotePositionEntries.size(); 
}

/* 
 *	Reconfigure all notes in the chunk to the fret+string positions indicated in the 
 *	current optimum object.
 */
void Chunk::RepositionNotesToCurrentOptimalPositions(void)
{
	if (CurrentOptimalNotePositionEntries.size() == GetNumberOfElements())
    {
		for(uint32_t noteIndex=0; noteIndex<GetNumberOfElements(); noteIndex++)
        {
            NotePositionEntry notePositionEntry = 
                    CurrentOptimalNotePositionEntries[noteIndex];
            
            uint32_t noteRepositioningIndex = notePositionEntry.RepositioningIndex;
            
			Note* note = GetElementWithIndex(noteIndex);
            if(note != nullptr)
            {
                note->SetPitchmapPositionIndex(noteRepositioningIndex);
            }
        }
	}
}


/* 
 *	Set the current optimal configuration of fret+string positions for this chunk
 */
void Chunk::SetOptimalNotePositions(vector<NotePositionEntry> newNotePositions) 
{
	CurrentOptimalNotePositionEntries = (newNotePositions);
}

void Chunk::ResetAllNotesRepositions(void)
{
    for(uint32_t noteIndex=0; noteIndex<GetNumberOfElements(); noteIndex++)
    {
        Note* note = GetElementWithIndex(noteIndex);
        if(note != nullptr)
        {
            note->ResetAttemptedRepositions();
        }
    }
}

std::string Chunk::PrintChunk(Chunk* chunk)
{
    std::stringstream sstream;
    
    if(chunk != nullptr)
    {
        vector<NotePositionEntry > noteConfiguration = chunk->GetCurrentNotePositionEntries();

        //Print the fret and string of each note in the current chunk configuration	
        for(NotePositionEntry notePositionEntry: noteConfiguration)
        {
            sstream << Note::PrintNote(notePositionEntry)<<" ";
        }
        
        sstream << ", index: " << chunk->GetMeasureIndex();
    }
    
    else
    {
        sstream << "|null|";
    }
    
    return sstream.str();
//	
}