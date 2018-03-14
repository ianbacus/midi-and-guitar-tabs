#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>

class RotateVisitor : public Visitor
{
    private:
        
        const uint32_t MaximumFretScalar;//maximum fret in a chunk
        const uint32_t FretSpanScalar;//span of a chunk: more than 5 gets difficult
        const uint32_t InterChunkSpacingScalar; //fret spacing between chunks: 
        const uint32_t StringOverlapScalar; //number of string overlaps
        
        static vector<uint32_t> GetStringPositionsOfIndices(
            vector<NotePositionEntry > chunkIndices);
        
        static uint32_t CountStringIntersectionsBetweenTwoChunkConfigurations(
            vector<NotePositionEntry> stringPositions1, 
            vector<NotePositionEntry> stringPositions2);
        
        Chunk* PreviousChunk;
        vector< vector<NotePositionEntry > > ProcessedChunkConfigurations;
        
        uint32_t CalculateConfigurationCost(
            vector<NotePositionEntry > chunkIndices);
        
        bool GetChunkFeatures(
            vector<NotePositionEntry > chunkIndices,
            uint32_t& maximumFretInCandidateChunk,
            uint32_t& fretSpacingInCandidateChunk,
            uint32_t& fretCenterInCandidateChunk,
            vector<uint32_t>& stringPositions);
        
        uint32_t GetChunkFretCenter(
            Chunk* candidateChunk);
        
        
        bool CountStringOverlaps(vector<NotePositionEntry> notePositionsEntries);
        
        uint32_t EvaluateConfigurationFeatures(
            uint32_t maximumFretInCandidateChunk,
            uint32_t fretSpacingInCandidateChunk,
            uint32_t fretCenterDifferenceFromLastChunk,
            vector<uint32_t> stringIntersections);
        
        bool MarkChunkAsProcessed(vector<NotePositionEntry > processedChunkConfiguration);
        bool WasChunkProcessed(vector<NotePositionEntry > input);
        void ResetMarkedChunks(void);
        
        bool RotateNoteOrItsParent(Chunk* candidateChunk, 
                const uint32_t noteIndex, uint32_t& octaveShiftCost);
        
        uint32_t ReconfigureChunk(Chunk* chunk, 
                uint32_t noteConfigurationIndex,
                bool& morePermutations);
        
        void SelectOptimalFingering(Chunk *chunkToConfigure, uint32_t& currentLowestCost);

    public:
        RotateVisitor(
            uint32_t maximumFretScalar,
            uint32_t fretSpanScalar,
            uint32_t interChunkSpacingScalar,
            uint32_t stringOverlapScalar);
        
        virtual ~RotateVisitor();

        
        virtual void VisitNote(Note* note);
        virtual void VisitBar(Bar* bar);
        virtual void VisitChunk(Chunk*);
};

#endif


