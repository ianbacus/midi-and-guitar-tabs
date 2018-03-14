#ifndef __CHUNK__
#define __CHUNK__

#include "note.h"
#include "base.h"

#include <vector>
#include <string>
#include <map>
#include <stack>

using std::vector;
using std::string;

class Chunk : public Base 
{
    private:

        int Delta;
        int _recursion_lock;
        vector<Note*> Notes;
        
        vector<NotePositionEntry > CurrentOptimalNotePositionEntries;

    public:

        static string PrintNoteIndices(vector<NotePositionEntry > currentNoteConfigurations);
        
        //Temporary note storage
        void PushElement(Note* note);
        void RemoveElement(Note* note);
        
        void SetOptimalNotePositions(vector<NotePositionEntry > set);
        void ResetAllNotesRepositions(void);
        void RepositionNotesToCurrentOptimalPositions(void);
        
        uint32_t GetOptimalChunkSize(void) const;
        vector<NotePositionEntry > GetCurrentOptimalNotePositionEntries() const;
        vector<NotePositionEntry > GetCurrentNotePositionEntries() const;
        uint32_t GetNumberOfPositionPermutations(void) const;
        
        //Note access
        vector<Note*> GetElements(void) const;
        Note* GetElementWithIndex(uint32_t index) const;
        Note* GetMostMobileNote(void) const; 
        int GetDelta() const;
        

        virtual uint32_t GetNumberOfElements() const ;
        virtual void DispatchVisitor(Visitor* v) ;
        bool CheckIfNoteIsAlreadyPresent(Note* note);

        Chunk(int delta=0);
        virtual ~Chunk(void);
        
        void CleanChunk(void);
        
};

#endif
