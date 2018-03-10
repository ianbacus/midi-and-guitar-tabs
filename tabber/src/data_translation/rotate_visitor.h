#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>

class RotateVisitor : public Visitor
{
    private:
        
        static const uint32_t MaximumFretScalar = 20;//maximum fret in a chunk
        static const uint32_t FretSpanScalar = 150;//span of a chunk
        static const uint32_t InterChunkSpacingScalar = 100; //fret spacing between chunks
        static const uint32_t StringOverlapScalar = 50; //number of string overlaps
        
        static vector<uint32_t> GetStringPositionsOfIndices(
            vector<NotePositionEntry > chunkIndices);
        
        static uint32_t CountStringIntersectionsBetweenTwoChunkConfigurations(
            vector<NotePositionEntry> stringPositions1, 
            vector<NotePositionEntry> stringPositions2);
        
        Chunk* PreviousChunk;
        int32_t NoteConfigurationIndex;
        vector< vector<NotePositionEntry > > ProcessedChunkConfigurations;
        
        uint32_t CalculateConfigurationCost(
            vector<NotePositionEntry > chunkIndices);
        
        bool GetChunkFeatures(
            vector<NotePositionEntry > chunkIndices,
            uint32_t& maximumFretInCandidateChunk,
            uint32_t& fretSpacingInCandidateChunk,
            uint32_t& fretCenterInCandidateChunk,
            vector<uint32_t>& stringPositions);
        
        uint32_t EvaluateConfigurationFeatures(
            uint32_t maximumFretInCandidateChunk,
            uint32_t fretSpacingInCandidateChunk,
            uint32_t noteCenterInCandidateChunk,
            vector<uint32_t> stringPositions);
        
        bool MarkChunkAsProcessed(vector<NotePositionEntry > processedChunkConfiguration);
        bool WasChunkProcessed(vector<NotePositionEntry > input);
        void ResetMarkedChunks(void);
        
        bool RotateNoteOrItsParent(Chunk* candidateChunk, uint32_t& noteIndex, uint32_t& octaveShiftCost);
        uint32_t ReconfigureChunk(Chunk* c, uint32_t& noteConfigurationIndex);

    public:
        RotateVisitor() : PreviousChunk(nullptr) {}
        virtual ~RotateVisitor() {}

        void SelectOptimalFingering(Chunk *chunkToConfigure);
        
        virtual void VisitNote(Note* note);
        virtual void VisitBar(Bar* bar);
        virtual void VisitChunk(Chunk*);
};

#endif


