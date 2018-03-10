#ifndef __CHUNK__
#define __CHUNK__

#include "note.h"
#include "base.h"

#include <vector>
#include <string>
#include <map>
#include <stack>

 //   uint32_t RepositioningIndex;
 //   uint32_t PitchMidiValue;
  
/*
struct NotePositionEntry
{
    uint32_t first;
    uint32_t second;
    
    NotePositionEntry() = default;
    
    NotePositionEntry(Note *note)
    {
        first = note->get_current_note_index();
        second = note->get_pitch(); 
    }
    
    inline bool operator==(const NotePositionEntry& rhs)
    { 
        return (second == rhs.second) && 
            (first == rhs.first);
    }
};*/

typedef pair<int, int> NotePositionEntry; //first = index, second = pitch

class Chunk : public Base 
{
    private:

        stack<Note*> NoteComparisonStack;
        int Delta;
        int _recursion_lock;
        vector<Note*> ChunkNotes;
        //Optimized chunks
        
        vector<NotePositionEntry > CurrentOptimalNotePositionEntries;

    public:

        static string PrintNoteIndices(vector<NotePositionEntry > currentNoteConfigurations);
        
        //Temporary note storage
        void PushElement(Note* n);
        void RemoveElement(Note* n);
        
        void SetOptimalNotePositions(vector<NotePositionEntry > set);
        void ResetAllNotesRepositions(void);
        void RepositionNotesToCurrentOptimalPositions(void);
        
        uint32_t GetOptimalChunkSize(void) const;
        vector<NotePositionEntry > GetCurrentOptimalNotePositionEntries() const;
        vector<NotePositionEntry > GetCurrentNotePositionEntries() const;
        uint32_t GetNumberOfPositionPermutations(void) const;
        
        //Note access
        Note* GetElementWithIndex(int i) const;
        Note* GetMostMobileNote(void) const; 
        int GetDelta() const;
        

        virtual int GetNumberOfElements() const ;
        virtual void DispatchVisitor(Visitor* v) ;

        Chunk(int delta=0);
        virtual ~Chunk(void);
        
};

#endif
