#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include "print_visitor.h"
#include <math.h>
#include <mutex>
#include <string>
#include <sstream>


struct ChunkFeatures
{
    uint32_t maximumFretInCandidateChunk;
    uint32_t minimumFretInCandidateChunk;
    uint32_t internalFretHorizontalDistance;
    uint32_t internalFretVerticalDistance;
    uint32_t fretCenterInCandidateChunk;
    
    uint32_t fretDistanceFromSustainedNotes;
    uint32_t sustainInterruptions;
    uint32_t fretDistanceFromAdjacentChunks;
    uint32_t numberOfDuplicateStrings;
    
    bool goodInternalFingerSpread;
    bool goodRelativeFingerSpread;
    
    vector<FretboardPosition> fretboardPositions;
};

class TablatureOptimizer : public Visitor
{
    private:
        
        const uint32_t MaximumFretScalar;//maximum fret in a chunk
        const uint32_t FretSpanScalar;//span of a chunk: more than 5 gets difficult
        const uint32_t InterChunkSpacingScalar; //fret spacing between chunks: 
        const uint32_t StringOverlapScalar; //number of string overlaps
        const uint32_t ArpeggiationDeductionScalar; //difference between adjacent strings
        
        const uint32_t NumberOfStrings;
        const uint32_t NumberOfFrets;
        const uint16_t AdjacentChunkSearchLength;
        
         vector<uint32_t> GetStringPositionsOfIndices(
            vector<NotePositionEntry > chunkIndices);
        
         vector<uint32_t> GetStringPositions(
            Chunk* chunk);
        
         uint32_t CountStringIntersectionsBetweenTwoChunkConfigurations(
                Chunk* previousChunk,
                Chunk* currentChunk);
        
        
         void GetChunkFeatures(
            Chunk* chunk,
            ChunkFeatures& chunkFeatures);
        
         void GetAdjacentChunkRelativeFeatures(
            Chunk* chunk, 
            uint32_t& candidateSpacingFromLastChunk,
            uint32_t& intersectionsWithPreviousChunk);

        uint32_t GetDistanceFromAdjacentNChunks(
            const Chunk* const chunk, 
            bool searchForward,
            uint16_t searchLength);

         void GetSustainedChunkRelativeFeatures(
            Chunk* chunk,
            uint32_t& stringIntersections,
            uint32_t& fretCenterOfSustainedNotes,
            bool& goodRelativeFingerSpread);
        
         void GetChunkInternalFeatures(
            Chunk* chunk,
            uint32_t& minimumFretInCandidateChunk,
            uint32_t& maximumFretInCandidateChunk,
            uint32_t& fretCenterInCandidateChunk,
            uint32_t& internalFretHorizontalDistance,
            uint32_t& internalFretVerticalDistance,
            bool& goodInternalFingerSpread);
        
        static uint32_t GetChunkFretCenter(
            const Chunk* candidateChunk);

        static uint32_t GetChunkStringCenter(
            Chunk* candidateChunk);
        
        Chunk* SearchForClosestOptimizedChunk(
            const Chunk* currentChunk,
            bool searchForward,
            bool searchFrettedChunksOnly);
        
        void LockStringsInTheNextFewChunksForThisConfiguration(Chunk *chunk);
        void UpdateStringIndexedRemainingDeltaTicks(Chunk* candidateChunk);
        
        uint32_t EvaluateConfigurationFeatures(ChunkFeatures chunkFeatures);
        
        uint32_t CalculateConfigurationCost(Chunk* chunk);
        
        vector< vector<NotePositionEntry > > ProcessedChunkConfigurations;
        
        vector<uint32_t> StringIndexedRemainingDeltaTicks;
        vector<uint32_t> StringIndexedFrettedNotes;        
        
        bool ValidateStringOverlapsForNotePositions(vector<NotePositionEntry> notePositionsEntries);
        
        bool MarkConfigurationProcessed(vector<NotePositionEntry >& processedChunkConfiguration);
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
        TablatureOptimizer(
            uint32_t numberOfStrings,
            uint32_t numberOfFrets,
            uint32_t maximumFretScalar,
            uint32_t fretSpanScalar,
            uint32_t interChunkSpacingScalar,
            uint32_t stringOverlapScalar,
            uint32_t arpeggiationDeductionScalar,
            uint32_t adjacentChunkSearchLength);
        
        virtual ~TablatureOptimizer();

        void EmitDebugString(std::string debugString);
        
        virtual void VisitNote(Note* note);
        virtual void VisitBar(Bar* bar);
        virtual void VisitChunk(Chunk* chunk) {}
        virtual uint32_t OptimizeChunk(Chunk*);
        
        
        TablatureOutputFormatter* Printer;
        void SetPrinter(TablatureOutputFormatter& printer)
        {
            Printer = &printer;
        }
        
        string PrintChunk(Chunk* chunk)
        {
            vector<Chunk*> chunks;
            chunks.push_back(chunk);
            return PrintChunks(chunks);
        }
        
        string PrintChunks(vector<Chunk*> chunks)
        {
            stringstream tablatureStringStream;
            vector<vector<string>> tablatureGroupBuffer;
            
            for(auto chunk : chunks)
            {
                if(chunk != nullptr)
                {
                    vector<string> chunkTablatureBuffer = 
                        Printer->GenerateDebugLogTablatureColumn(chunk);

                    tablatureGroupBuffer.push_back(chunkTablatureBuffer);
                }
            }
            
            vector<string> tablatureBuffer = 
                Printer->ConcatenateColumnsIntoMeasureStrings(tablatureGroupBuffer);
            
            for(string tablatureRowData : tablatureBuffer)
            {
                tablatureStringStream <<  tablatureRowData << "\r\n";
            }

            tablatureStringStream << "\r\n"; 
            
            string outputString = tablatureStringStream.str();
            return outputString;
            
        }
};

#endif


