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

class TablatureOutputFormatter;

class Chunk : public Base 
{
    private:

        vector<Note*> Notes;
        vector<NotePositionEntry > CurrentOptimalNotePositionEntries;
        
        Chunk* PreviousChunk;
        Chunk* NextChunk;
        uint32_t Delta;
        uint32_t MeasureIndex;
        bool IsOptimized;
        bool MeasureStart;
        vector<FretboardPosition> SustainedFretboardPositions;
        

    public:

        static std::string PrintNoteIndices(vector<NotePositionEntry > currentNoteConfigurations);
        //static std::string PrintChunk(Chunk* chunk);
        
        //Temporary note storage
        void PushElement(Note* note);
        void RemoveElement(Note* note);
        
        void SetOptimalNotePositions(vector<NotePositionEntry > set);
        void ResetAllNotesRepositions(void);
        void RepositionNotesToCurrentOptimalPositions(void);
        
        uint32_t GetOptimalChunkSize(void) const;
        vector<NotePositionEntry > GetCurrentOptimalNotePositionEntries() const;
        vector<NotePositionEntry > GetCurrentNotePositionEntries() const;
        
        vector<FretboardPosition> GetCurrentFretboardPositions();
        
        uint32_t GetNumberOfPositionPermutations(void) const;
        
        //Note access
        vector<Note*> GetElements(void) const;
        Note* GetElementWithIndex(uint32_t index) const;
        Note* GetMostMobileNote(void) const; 
        int GetDelta() const;
        bool GetIsMeasureEnd(void);
        
        void SetIsOptimized(bool);
        void SetPreviousChunk(Chunk* previousChunk);
        void SetNextChunk(Chunk* nextChunk);
        void SetIsMeasureEnd(bool);
        void SetLockedStringIndices(vector<FretboardPosition> lockedStrings);
        
        Chunk* GetPreviousChunk(void);
        Chunk* GetNextChunk(void);
        bool GetIsOptimized(void);
        uint32_t GetMeasureIndex(void);
        uint32_t GetAveragePitch(void);
        vector<FretboardPosition> GetSustainedFretboardPositions(void);
        

        virtual uint32_t GetNumberOfElements() const ;
        virtual void DispatchVisitor(Visitor* v) ;
        bool CheckIfNoteIsAlreadyPresent(Note* note);

        Chunk(int delta, uint32_t measureIndex);
        virtual ~Chunk(void);
        
        void CleanChunk(void);
        
//        
        
        static std::string PrintChunk(Chunk* chunk);
        
};

#endif
