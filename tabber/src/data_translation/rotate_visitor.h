#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>

struct ChunkFeatures
{
    uint32_t maximumFretInCandidateChunk;
    uint32_t fretSpacingInCandidateChunk;
    uint32_t fretCenterInCandidateChunk;
    uint32_t fretDistanceFromSustainedNotes;
    
    uint32_t sustainInterruptions;
    uint32_t candidateSpacingFromLastChunk;
    uint32_t numberOfDuplicateStrings;
    
    vector<uint32_t> stringPositions;
};

class RotateVisitor : public Visitor
{
    private:
        
        const uint32_t MaximumFretScalar;//maximum fret in a chunk
        const uint32_t FretSpanScalar;//span of a chunk: more than 5 gets difficult
        const uint32_t InterChunkSpacingScalar; //fret spacing between chunks: 
        const uint32_t StringOverlapScalar; //number of string overlaps
        const uint32_t NumberOfStrings;
        
        static vector<uint32_t> GetStringPositionsOfIndices(
            vector<NotePositionEntry > chunkIndices);
        
        static vector<uint32_t> GetStringPositions(
            Chunk* chunk);
        
        static uint32_t CountStringIntersectionsBetweenTwoChunkConfigurations(
                Chunk* previousChunk,
                Chunk* currentChunk);
        
        
        static void GetChunkFeatures(
            Chunk* chunk,
            ChunkFeatures& chunkFeatures);
        
        static void GetAdjacentChunkRelativeFeatures(
            Chunk* chunk, 
            uint32_t& candidateSpacingFromLastChunk,
            uint32_t& intersectionsWithPreviousChunk);

        static uint32_t GetChunkFretCenter(
            Chunk* candidateChunk);
        
        static void CountStringIntersectionsWithFrettedNotes(
            Chunk* chunk,
            uint32_t& stringIntersections,
            uint32_t& fretCenterOfSustainedNotes);
        
        
        static Chunk* SearchForClosestOptimizedChunk(Chunk* currentChunk,
                bool searchForward, bool searchFrettedChunksOnly);
        
        void LockStringsInTheNextFewChunksForThisConfiguration(Chunk *chunk);
        void UpdateStringIndexedRemainingDeltaTicks(Chunk* candidateChunk);
        
        uint32_t EvaluateConfigurationFeatures(ChunkFeatures chunkFeatures);
        
        uint32_t CalculateConfigurationCost(Chunk* chunk);
        
        
        Chunk* PreviousFrettedChunky;
        Chunk* PreviousChunky;
        vector< vector<NotePositionEntry > > ProcessedChunkConfigurations;
        
        vector<uint32_t> StringIndexedRemainingDeltaTicks;
        vector<uint32_t> StringIndexedFrettedNotes;
        
        
        
        bool ValidateStringOverlapsForNotePositions(vector<NotePositionEntry> notePositionsEntries);
        
        
        bool MarkConfigurationProcessed(vector<NotePositionEntry > processedChunkConfiguration);
        bool WasConfigurationProcessed(vector<NotePositionEntry > input);
        void ResetMarkedChunks(void);
        
        bool RotateNoteOrItsParent(Chunk* candidateChunk, 
                const uint32_t noteIndex, uint32_t& octaveShiftCost);
        
        uint32_t ReconfigureChunk(Chunk* chunk, 
                uint32_t noteConfigurationIndex,
                bool& morePermutations);
        
        void SelectOptimalFingering(Chunk *chunkToConfigure, 
                uint32_t& currentLowestCost);
        

    public:
        RotateVisitor(
            uint32_t numberOfStrings,
            uint32_t maximumFretScalar,
            uint32_t fretSpanScalar,
            uint32_t interChunkSpacingScalar,
            uint32_t stringOverlapScalar);
        
        virtual ~RotateVisitor();

        
        virtual void VisitNote(Note* note);
        virtual void VisitBar(Bar* bar);
        virtual void VisitChunk(Chunk* chunk) {}
        virtual uint32_t OptimizeChunk(Chunk*);
};

#endif


