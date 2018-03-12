#include "base.h"
#include <sstream>
#include <assert.h>

using namespace std;

string Chunk::PrintNoteIndices(vector<NotePositionEntry > noteConfiguration)
{
    std::stringstream sstream;
    
    sstream << "-|";
    
    //Print the fret and string of each note in the current chunk configuration	
	for(NotePositionEntry notePositionEntry: noteConfiguration)
	{
		const uint32_t noteFret = Note::GetFretForNotePositionEntry(notePositionEntry);
        const uint32_t noteString = Note::GetStringForNotePositionEntry(notePositionEntry);
        
		sstream << noteFret << "+" << noteString << "|";
	}
    
    return sstream.str();
}

Chunk::Chunk(int d) 
    : 
        Delta(d), _recursion_lock(0) 
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

/* 
 *	Insert a note into the vector of notes. Insert in sorted order by the number of frettable
 *	positions for that note.
 */
void Chunk::PushElement(Note* note) 
{
	//insert at iterator to first element with a pitch-map entry larger than the candidate note
	Notes.insert(std::upper_bound( Notes.begin(), Notes.end(), note,  
        [](Note *a, Note*b) 
        { 															 
            return (a->GetNumberOfElements() < b->GetNumberOfElements()); 				 
        }),
        note); 
        
    assert(note != nullptr);
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