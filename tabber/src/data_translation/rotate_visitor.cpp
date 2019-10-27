#include "rotate_visitor.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <assert.h>

using namespace std;

stringstream fileStringStream;
    
ostream& FileStream = fileStringStream;
ostream NullStream(nullptr);
ostream& TerminalStream = cout;


#define FEATURE_STREAM FileStream << "\tFeatures: "
#define COST_STREAM FileStream << "\tCosts: "
#define ERROR_STREAM NullStream << "\tError: "
#define RESULT_STREAM FileStream<< "Results: "


void TablatureOptimizer::EmitDebugString(std::string debugString)
{
    RESULT_STREAM << debugString << endl;
}   

TablatureOptimizer::TablatureOptimizer(
        uint32_t numberOfStrings,
		uint32_t numberOfFrets,
        uint32_t maximumFretScalar,
        uint32_t fretSpanScalar,
        uint32_t interChunkSpacingScalar,
        uint32_t stringOverlapScalar,
        uint32_t arpeggiationDeductionScalar,
		uint32_t adjacentChunkSearchLength)
    : 
        StringIndexedFrettedNotes(numberOfStrings),
        StringIndexedRemainingDeltaTicks(numberOfStrings),
        
        NumberOfStrings(numberOfStrings),
		NumberOfFrets(numberOfFrets),
		
        
        MaximumFretScalar(maximumFretScalar),
        FretSpanScalar(fretSpanScalar),
        InterChunkSpacingScalar(interChunkSpacingScalar),
        StringOverlapScalar(stringOverlapScalar),
        ArpeggiationDeductionScalar(arpeggiationDeductionScalar),
        
		AdjacentChunkSearchLength(adjacentChunkSearchLength)
         
{
    
}

TablatureOptimizer::~TablatureOptimizer(void) 
{    
    ofstream outputFileStream;
    
	outputFileStream.open("debugOutput.txt");
	outputFileStream << FileStream.rdbuf();
	outputFileStream.close();
}

//Go through all of the chunks in a given bar and reconfigure them
void TablatureOptimizer::VisitBar(Bar* bar) 
{
    for(uint32_t barIndex=0; barIndex < bar->GetNumberOfElements(); barIndex++)
    {
        Chunk* candidateChunk = bar->GetElementWithIndex(barIndex);
        
        candidateChunk->CleanChunk();
        candidateChunk->DispatchVisitor(this);
    }
}

//Visit a chunk, and re-arrange all of its notes until they are valid/playable
uint32_t TablatureOptimizer::OptimizeChunk(Chunk* candidateChunk) 
{
    const uint32_t permutationsForThisChunk = 
        candidateChunk->GetNumberOfPositionPermutations();

    uint32_t permutationIndex = 0;
    uint32_t currentLowestCost = UINT32_MAX;
    uint32_t chunkFretCenter = 0;
    
    candidateChunk->CleanChunk();
    bool morePermutations = true;
    
    while(morePermutations)
    {
        permutationIndex++;
        
        //Reconfigure or "rotate" the chunk until it is in another configuration
        ReconfigureChunk(candidateChunk,permutationIndex,morePermutations);
        
        
        morePermutations = morePermutations && 
                            (permutationIndex < permutationsForThisChunk);
        
        //Compare the current best chunk configuration against the reconfigured one
        SelectOptimalFingering(candidateChunk, currentLowestCost);	
        

    }
    
    ResetMarkedChunks();
    
    candidateChunk->RepositionNotesToCurrentOptimalPositions();
    candidateChunk->SetIsOptimized(true);
    chunkFretCenter = GetChunkFretCenter(candidateChunk);
    
    //UpdateStringIndexedRemainingDeltaTicks(candidateChunk);
    LockStringsInTheNextFewChunksForThisConfiguration(candidateChunk);
    
    RESULT_STREAM << "Optimized chunk: " << 
        Chunk::PrintNoteIndices(candidateChunk->GetCurrentNotePositionEntries()) << 
        ", Index: " << candidateChunk->GetMeasureIndex() <<
        endl << "-------------" << endl;
    
    return currentLowestCost;
    

} //end VisitChunk

void TablatureOptimizer::UpdateStringIndexedRemainingDeltaTicks(
    Chunk* candidateChunk)
{
    Chunk * const previousChunk = SearchForClosestOptimizedChunk(candidateChunk, false, false);//PreviousChunk;
    
    vector<Note*> chunkNotes = candidateChunk->GetElements();
    
    if(previousChunk != nullptr)
    {
        uint32_t previousChunkDelta = previousChunk->GetDelta();
        
        
        for(uint32_t stringIndex = 0; 
                stringIndex<StringIndexedRemainingDeltaTicks.size();
                stringIndex++)
        {
            uint32_t originalValue =
                    StringIndexedRemainingDeltaTicks[stringIndex] ;
            
            uint32_t updatedValue = originalValue;
                
            if(originalValue > 0)
            {
                updatedValue -= min(originalValue, previousChunkDelta);
            }
            
            if(updatedValue == 0)
            {
                StringIndexedFrettedNotes[stringIndex] = 0;
            }
            
            StringIndexedRemainingDeltaTicks[stringIndex] = updatedValue;
        }
    }
    
    for(Note* note : chunkNotes)
    {
        uint32_t stringIndex = note->GetStringIndexForCurrentNotePosition();
        uint32_t fretNumber = note->GetFretForCurrentNotePosition();
        uint32_t noteDuration = note->GetNoteDurationBeats();
        
        if(noteDuration != 0)
        {
            StringIndexedRemainingDeltaTicks[stringIndex] = noteDuration;
            StringIndexedFrettedNotes[stringIndex] = fretNumber;
        }
    }
}

//Get the closest adjacent optimized chunk
Chunk* TablatureOptimizer::SearchForClosestOptimizedChunk(
	const Chunk* currentChunk, 
	bool searchForward,
	bool frettedChunksOnly)
//	uint16_t adjacentChunkSearchLength)
{
    const uint32_t maximumUnOptimizedSearchLength = 
		AdjacentChunkSearchLength+10;
    
    const Chunk* discoveredChunk = nullptr;
    uint32_t searchCounter = 0;
    
    while((currentChunk != nullptr) && (searchCounter < maximumUnOptimizedSearchLength))
    {
        searchCounter++;
        
        if(searchForward)
        {
            currentChunk = currentChunk->GetNextChunk();
        }

        else
        {
            currentChunk = currentChunk->GetPreviousChunk();
        }
        
        //Only consider optimized chunks
        if((currentChunk != nullptr) && currentChunk->GetIsOptimized())
        {
            const bool chunkSatisfiesFretCondition = 
                ((frettedChunksOnly) && (GetChunkFretCenter(currentChunk) != 0));
            
            const bool dontCareAboutFrets = !frettedChunksOnly;
            
            if(chunkSatisfiesFretCondition || dontCareAboutFrets)
            {
                discoveredChunk = currentChunk;
                break;
            }
        }        
        
    }
    //todo remove this const cast after testing
    return const_cast<Chunk*>(discoveredChunk);
}

void TablatureOptimizer::LockStringsInTheNextFewChunksForThisConfiguration(Chunk *chunk)
{
    vector<uint32_t> stringIndexedRemainingDeltaTicks(NumberOfStrings);
    vector<uint32_t> stringIndexedFrettedNotes(NumberOfStrings);
    
    for(Note* note : chunk->GetElements())
    {
        uint32_t stringIndex = note->GetStringIndexForCurrentNotePosition();
        uint32_t fretNumber = note->GetFretForCurrentNotePosition();
        uint32_t noteDuration = note->GetNoteDurationBeats();
        
        if(noteDuration != 0)
        {
            stringIndexedRemainingDeltaTicks[stringIndex] = noteDuration;
            stringIndexedFrettedNotes[stringIndex] = fretNumber;
        }
    }
    
    bool currentChunkSustained = true;
    
    while(currentChunkSustained)
    {
        chunk = chunk->GetNextChunk();
        
        currentChunkSustained = false;
        
        if(chunk != nullptr)
        {
            uint32_t delta = chunk->GetDelta();
            
            vector<FretboardPosition> sustainedFretboardPositions;
            
            for(uint32_t stringIndex=0;stringIndex<NumberOfStrings;stringIndex++)
            {
                uint32_t currentDeltaValue = 
                        stringIndexedRemainingDeltaTicks[stringIndex];
                
                stringIndexedRemainingDeltaTicks[stringIndex] -= 
                        min(currentDeltaValue,delta);
                
                if(stringIndexedRemainingDeltaTicks[stringIndex] > 0)
                {
                    const uint32_t fretNumberForString = 
                        stringIndexedFrettedNotes[stringIndex];
                    
                    const FretboardPosition 
                        fretPosition(stringIndex,fretNumberForString);
                    
                    sustainedFretboardPositions.push_back(fretPosition);
                    currentChunkSustained = true;
                }
            }
            
            chunk->SetLockedStringIndices(sustainedFretboardPositions);
        }
    }
}

//Change the note positions in this chunk to a configuration that has not 
//been checked yet. Skip configurations with impossible fingerings. 
uint32_t TablatureOptimizer::ReconfigureChunk(
        Chunk* candidateChunk, 
        uint32_t permutationCount,
        bool& morePermutations)
{
    uint32_t maxErrorsAllowed = 10;
    uint32_t errorCount = 0;
    uint32_t octaveShiftCost = 0;
    
    //todo: don't count all permutations worst case
    const uint32_t permutationsForThisChunk = 
        candidateChunk->GetNumberOfPositionPermutations();
    
    const uint32_t chunkSize = candidateChunk->GetNumberOfElements() ;
    uint32_t noteConfigurationIndex = chunkSize-1;
    
    volatile bool shouldContinue = true;
    
    do
    {
        volatile const bool exhaustedAllPermutations = !RotateNoteOrItsParent(candidateChunk,
                noteConfigurationIndex,octaveShiftCost);
        
        morePermutations = morePermutations && exhaustedAllPermutations;
        
        vector<NotePositionEntry> currentNotePositionsEntries =
                candidateChunk->GetCurrentNotePositionEntries();
       
        const bool stringsOverlap = ValidateStringOverlapsForNotePositions(currentNotePositionsEntries);
        
        const bool currentNotePositionsAlreadyTried = 
                WasConfigurationProcessed(candidateChunk->GetCurrentNotePositionEntries());
        
        shouldContinue = stringsOverlap ||  (currentNotePositionsAlreadyTried && 
                                             morePermutations);
        if(true)//exhaustedAllPermutations)
        {
            //todo: prevent this from happening by cleaning chunks properly
            errorCount++;
            ERROR_STREAM << "\tError: exhausted all permutations " <<errorCount << " " << 
                    Chunk::PrintNoteIndices(currentNotePositionsEntries) 
                    << ", overlap: " << stringsOverlap 
                    << ", alreadyTried: " << currentNotePositionsAlreadyTried 
                    << ", morePermutations: " << morePermutations << endl;
        }
        
        if(!((shouldContinue && errorCount < 10)) || (permutationCount > permutationsForThisChunk))
        {
            ERROR_STREAM << "\tExiting because " << 
                    Chunk::PrintNoteIndices(currentNotePositionsEntries) 
                    << ", permutations = " << permutationCount << "/" << permutationsForThisChunk
                    << ", errors = " << "errorCount" << "/" << maxErrorsAllowed
                    << ", overlap: " << stringsOverlap 
                    << ", alreadyTried: " << currentNotePositionsAlreadyTried 
                    << ", morePermutations: " << morePermutations << endl;
        }
        
    } while(shouldContinue && (errorCount < maxErrorsAllowed) );
    
    
    return octaveShiftCost;
    
} //end ReconfigureChunk


//Pick between the current note positions and the current optimum note positions
//Apply the lower cost one 
void TablatureOptimizer::SelectOptimalFingering(
        Chunk *chunkToConfigure,
        uint32_t& currentLowestCost) 
{
    vector<NotePositionEntry> candidateChunkFingering =
        chunkToConfigure->GetCurrentNotePositionEntries();
    
    const bool alreadyCheckedThisConfiguration = WasConfigurationProcessed(candidateChunkFingering);
    
    if(!alreadyCheckedThisConfiguration)
    {
        const uint32_t candidateCost = CalculateConfigurationCost(chunkToConfigure);
        
        if(candidateCost < currentLowestCost)
        {            
            chunkToConfigure->SetOptimalNotePositions(candidateChunkFingering);
            chunkToConfigure->RepositionNotesToCurrentOptimalPositions();
            
            currentLowestCost = candidateCost;
        }
        
        MarkConfigurationProcessed(candidateChunkFingering);   
        
    }
} //end SelectOptimalFingering


bool TablatureOptimizer::RotateNoteOrItsParent(
    Chunk* candidateChunk, 
    const uint32_t noteIndex, 
    uint32_t& octaveShiftCost)
{
    bool baseCase = true;
    
    Note* const note = candidateChunk->GetElementWithIndex(noteIndex);
    
    if(note == nullptr)
    {
        candidateChunk->RemoveElement(note);
    }
    
    else
    {
        const uint32_t attemptedRepositions = note->GetCurrentPitchmapIndex()+1;
        const uint32_t possibleRepositions = note->GetNumberOfElements();

        const bool repositionParent = attemptedRepositions >= possibleRepositions;
        
        baseCase = (noteIndex == 0) && repositionParent;
        
        if(!baseCase)
        {
            uint32_t parentIndex = noteIndex-1;
            Note* const parentNote = candidateChunk->GetElementWithIndex(parentIndex);
            
            //TODO: try shifting the note up and down an octave
            //octaveShiftCost++;
            
            if((repositionParent) && (parentNote != nullptr))
            {
                baseCase = RotateNoteOrItsParent(candidateChunk, parentIndex, octaveShiftCost);
            }
        }
        
        note->DispatchVisitor(this);
    }
    
    return baseCase;
} //end RotateNoteOrItsParent

 //Count string overlaps in a chunk
bool TablatureOptimizer::ValidateStringOverlapsForNotePositions(vector<NotePositionEntry> notePositionsEntries)
{
    volatile bool stringsOverlap;
    
    vector<uint32_t> stringIndices = GetStringPositionsOfIndices(notePositionsEntries);
    uint32_t numberOfStringIndices = stringIndices.size();
    unordered_map<uint32_t, uint32_t> stringToOverlapCountMap;
    
    for(uint32_t index=0; 
        (index<numberOfStringIndices)&&(!stringsOverlap); index++)
    {
        uint32_t stringIndex = stringIndices[index];
        stringToOverlapCountMap[stringIndex]++;
        
        stringsOverlap = stringToOverlapCountMap[stringIndex] > 1;
    }
    
    //for (auto kv : stringToOverlapCountMap)
    //{
    //}
    
    return stringsOverlap;
}



uint32_t TablatureOptimizer::CalculateConfigurationCost(
    Chunk* chunk)
{
    uint32_t chunkCost = UINT32_MAX;
    
    ChunkFeatures candidateChunkFeatures;
    
    //FEATURE_STREAM << PrintChunk(chunk) << endl;
    GetChunkFeatures(chunk, candidateChunkFeatures);

    //COST_STREAM << PrintChunk(chunk) << endl;
    chunkCost =  EvaluateConfigurationFeatures(candidateChunkFeatures);
    
    return chunkCost;
    
} //end CalculateConfigurationCost


uint32_t TablatureOptimizer::GetChunkFretCenter(
	const Chunk* candidateChunk)
{
    uint32_t fretCenterInCandidateChunk = 0;
    
    if(candidateChunk != nullptr)
    {
        uint32_t numberOfFrettedNotes = 0;
        uint32_t fretSumForAverage = 0;
        vector<NotePositionEntry > chunkIndices = 
            candidateChunk->GetCurrentNotePositionEntries();

        for(NotePositionEntry notePositionEntry : chunkIndices)
        {
            const uint32_t currentFret = Note::GetFretForNotePositionEntry(notePositionEntry);        

            //Don't count 0 in the fret center calculations
            if(currentFret != 0) 
            {
                fretSumForAverage += currentFret;
                numberOfFrettedNotes++;
            }

        } //end loop

        if(numberOfFrettedNotes != 0)
        {
            fretCenterInCandidateChunk = fretSumForAverage / numberOfFrettedNotes;
        }
    }
    
    return fretCenterInCandidateChunk;
}

uint32_t TablatureOptimizer::GetChunkStringCenter(
        Chunk* candidateChunk)
{
    uint32_t stringCenterInCandidateChunk = 0;
    
    if(candidateChunk != nullptr)
    {
        const uint32_t numberOfNotes = candidateChunk->GetNumberOfElements();
        uint32_t stringSumForAverage = 0;
        vector<NotePositionEntry > chunkIndices = 
            candidateChunk->GetCurrentNotePositionEntries();

        for(NotePositionEntry notePositionEntry : chunkIndices)
        {
            const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);        

            stringSumForAverage += currentString;
        } 

        if(numberOfNotes != 0)
        {
            stringCenterInCandidateChunk = stringSumForAverage / numberOfNotes;
        }
    }
    
    return stringCenterInCandidateChunk;
}

/*
 * Find the maximum fret and total fret spacing in a vector of note grid positions.
 * Return the fret spacing and maximum fret as output parameters
*/
void TablatureOptimizer::GetChunkFeatures(
        Chunk* chunk,
        ChunkFeatures& chunkFeatures)
{
    chunkFeatures.fretDistanceFromSustainedNotes = 0;
    chunkFeatures.maximumFretInCandidateChunk = 0;
    chunkFeatures.minimumFretInCandidateChunk = 0;
    chunkFeatures.minimumFretInCandidateChunk = 0;
	chunkFeatures.internalFretHorizontalDistance = 0;
	chunkFeatures.internalFretVerticalDistance = 0;
    chunkFeatures.fretCenterInCandidateChunk = 0;
    
    GetChunkInternalFeatures(
		chunk,
		chunkFeatures.minimumFretInCandidateChunk,
		chunkFeatures.maximumFretInCandidateChunk,
		chunkFeatures.fretCenterInCandidateChunk,
		chunkFeatures.internalFretHorizontalDistance,
		chunkFeatures.internalFretVerticalDistance,
		chunkFeatures.goodInternalFingerSpread);
    
    GetSustainedChunkRelativeFeatures(
		chunk,
		chunkFeatures.sustainInterruptions,
		chunkFeatures.fretDistanceFromSustainedNotes,
		chunkFeatures.goodRelativeFingerSpread);

    GetAdjacentChunkRelativeFeatures(
		chunk,
        chunkFeatures.fretDistanceFromAdjacentChunks,
        chunkFeatures.numberOfDuplicateStrings);
    
    //Trace
    {
        stringstream stringIndices;

        stringIndices<< "|";

        for(FretboardPosition stringFret : chunkFeatures.fretboardPositions)
        {
            stringIndices<< stringFret.StringIndex << stringFret.FretNumber << "|";
        }

        stringIndices<< "|";
        
        FEATURE_STREAM << "Inherent" << 
            "\r\n\t\tMax Fret:" << chunkFeatures.maximumFretInCandidateChunk << " " 
            "\r\n\t\tH. Span:" << chunkFeatures.internalFretHorizontalDistance << " " <<
            "\r\n\t\tV. Span:" << chunkFeatures.internalFretVerticalDistance << " " <<
            "\r\n\t\tFret Center:" << chunkFeatures.fretCenterInCandidateChunk << 
            "\r\n\t\tDistance from sustained notes:" << chunkFeatures.fretDistanceFromSustainedNotes << 
            "\r\n\t\tSustain interruptions:" << chunkFeatures.sustainInterruptions <<
            "\r\n\t\tRelative spacing:" << chunkFeatures.fretDistanceFromAdjacentChunks << 
            endl;
    }
    
} //end GetChunkFeatures

#ifndef TESTEXP

uint32_t TablatureOptimizer::EvaluateConfigurationFeatures(
    ChunkFeatures chunkFeatures)
//
// chunkFeatures:
// maximumFretInCandidateChunk;
// internalFretHorizontalDistance;
// fretCenterInCandidateChunk;
// fretDistanceFromSustainedNotes;
// sustainInterruptions;
// fretDistanceFromAdjacentChunks;
// numberOfDuplicateStrings;
//
{
	//Inter-chunk costs
	//const float fretSpanScaleFactor = minimumFretInCandidateChunk;
	
	const float distanceMin = 
		NumberOfFrets - 
		(NumberOfFrets / 
		pow(2,(chunkFeatures.minimumFretInCandidateChunk / 12.0f)));
	
	const float distanceMax = 
		NumberOfFrets - 
		(NumberOfFrets / 
		pow(2,(chunkFeatures.maximumFretInCandidateChunk / 12.0f)));
	
	const float correctedFretSpan = distanceMax - distanceMin;
	//cout << distanceMin << " " << distanceMax << " " << correctedFretSpan << " " << chunkFeatures.internalFretHorizontalDistance << endl;
	chunkFeatures.internalFretHorizontalDistance *= correctedFretSpan;
	
	
    const float maximumFretCost = 
		chunkFeatures.maximumFretInCandidateChunk * MaximumFretScalar;
	
	//Raise by power of 2: the span cost should grow exponentially as the spans 
	//start to become impossible
    const float fretSpanCost = 
		pow(chunkFeatures.internalFretHorizontalDistance, 2.0f) * FretSpanScalar;
	
	//Divide by 3: each vertical span should only count a third as much as 
	//each horizontal span
    const float stringSpanCost = 
		chunkFeatures.internalFretVerticalDistance * FretSpanScalar / 3.0f;
	
	const float fretCenterCost = 
		chunkFeatures.fretCenterInCandidateChunk * MaximumFretScalar;
	
	//Intra-chunk costs
	const float arpeggiationCost = 
		chunkFeatures.numberOfDuplicateStrings * ArpeggiationDeductionScalar;
    
    const float sustainBreakCost = 
        chunkFeatures.sustainInterruptions * StringOverlapScalar;
	
	const float interChunkSpacingCost =
        pow(chunkFeatures.fretDistanceFromAdjacentChunks, 2.0f) * InterChunkSpacingScalar;
    
	const float sustainSpacingCost = 
        pow(chunkFeatures.fretDistanceFromSustainedNotes, 2.0f) * 
		(InterChunkSpacingScalar+StringOverlapScalar)/2.0f;
	
	const uint32_t interChunkCost = 
		maximumFretCost + 
		fretSpanCost + 
		stringSpanCost +
		fretCenterCost;
	
	const uint32_t intraChunkCost =	
		arpeggiationCost + 
		sustainBreakCost +
		interChunkSpacingCost +
		sustainSpacingCost;
	
	const uint32_t candidateCost = interChunkCost + intraChunkCost;

    COST_STREAM << 
		"$" << candidateCost << 
        "\r\n\t\tInter-cost: $" << interChunkCost << 
        "\r\n\t\tIntra-cost: $" << intraChunkCost << 
		endl;
	
	COST_STREAM << 
		"Inter-chunk cost:" <<
        "\r\n\t\tMax Fret: $" << maximumFretCost << 
        "\r\n\t\tFret Span: $" << fretSpanCost << 
        "\r\n\t\tString Span: $" << stringSpanCost << 
		endl;
	
	COST_STREAM << 
		"Intra-chunk cost:" <<
        "\r\n\t\tArpeggiation: $" << arpeggiationCost << 
        "\r\n\t\tSustain: $" << sustainBreakCost <<
        "\r\n\t\tAdjacency: $" << interChunkSpacingCost << 
        "\r\n\t\tSustain breaking: $" << sustainSpacingCost << 
		endl;		
			
    
    return candidateCost;
}

#else

uint32_t TablatureOptimizer::EvaluateConfigurationFeatures(
    ChunkFeatures chunkFeatures)
{
    uint32_t maximumPlayableSpan = 6;
    
    //Adjust the internal fret distance 
    if(!chunkFeatures.goodInternalFingerSpread)
    {
        chunkFeatures.internalFretHorizontalDistance *= 1.5f;
        maximumPlayableSpan = 4;
    }
    
    if(chunkFeatures.internalFretHorizontalDistance >= maximumPlayableSpan)
    {
        chunkFeatures.internalFretHorizontalDistance *= 2.0f;
    }
	
	//For higher frets, reduce the cost of fret internal and relative distance 
	
//	const uint32_t fretCenter = 1+chunkFeatures.fretCenterInCandidateChunk;
//	chunkFeatures.fretDistanceFromAdjacentChunks *= (float)(12 / fretCenter);
//	chunkFeatures.fretDistanceFromSustainedNotes *= (float)(12 / fretCenter);
//	chunkFeatures.internalFretHorizontalDistance *= (float)(12 / fretCenter);
	
    const float trackStringDivergenceCost = 
        pow(chunkFeatures.numberOfDuplicateStrings, 1.25f) * ArpeggiationDeductionScalar;
    
    const float stringOverlapCost = 
        pow(chunkFeatures.sustainInterruptions, 1.25f) * StringOverlapScalar;
    
    //Count sustained notes in the interchunk spacing by taking the average of 
    //the inter chunk cost and the string overlap cost for a part of the total
    //spacing cost
    const float interChunkSpacingCost =
        (pow(chunkFeatures.fretDistanceFromAdjacentChunks,1.5f)*
            InterChunkSpacingScalar +
    
        pow(chunkFeatures.fretDistanceFromSustainedNotes,1.5f)*
            (InterChunkSpacingScalar+StringOverlapScalar)/2.0f)/2.0f;
    
    const float maximumFretCost = (chunkFeatures.maximumFretInCandidateChunk)*MaximumFretScalar;
    const float fretSpanCost = pow(chunkFeatures.internalFretHorizontalDistance,2.0f)*FretSpanScalar;
    
    const uint32_t candidateCost = stringOverlapCost + trackStringDivergenceCost +
                        maximumFretCost + fretSpanCost + interChunkSpacingCost; 
        
    
    COST_STREAM << "$" << candidateCost << 
        "\r\n\t\tMax Fret:" << maximumFretCost << 
        "\r\n\t\tFret Span:" << fretSpanCost << 
        "\r\n\t\tAdjacency:" << interChunkSpacingCost << 
        "\r\n\t\tSustain:" << stringOverlapCost <<
        "\r\n\t\tArpeggiation:" << trackStringDivergenceCost << endl;
    
    return candidateCost;
    
} //end EvaluateConfigurationFeatures

#endif 

vector<uint32_t> TablatureOptimizer::GetStringPositionsOfIndices(
        vector<NotePositionEntry > chunkIndices)
{
    vector<uint32_t> stringPositions;
    
    for(NotePositionEntry notePositionEntry : chunkIndices)
    {
        const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);

        stringPositions.push_back(currentString);
    }
    
    return stringPositions;
}


vector<uint32_t> TablatureOptimizer::GetStringPositions(
        Chunk* chunk)
{
    vector<uint32_t> stringPositions;
   
    if(chunk != nullptr)
    {
        vector<NotePositionEntry > chunkIndices = chunk->GetCurrentNotePositionEntries();

        for(NotePositionEntry notePositionEntry : chunkIndices)
        {
            const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);

            stringPositions.push_back(currentString);
        }
    }
    
    return stringPositions;
}


uint32_t TablatureOptimizer::CountStringIntersectionsBetweenTwoChunkConfigurations(
    Chunk* otherChunk,
    Chunk* currentChunk)
{    
    uint32_t intersectionCost = 0;
    
    if((currentChunk ==nullptr) || (otherChunk == nullptr))
    {
    }
    
    else if(abs((int)currentChunk->GetMeasureIndex() - (int)otherChunk->GetMeasureIndex()) > 1)
    {
    }
    
    else
    {
        const uint32_t otherChunkStringCenter = GetChunkStringCenter(otherChunk);
        const uint32_t currentChunkStringCenter = GetChunkStringCenter(currentChunk);
        
        const int32_t otherChunkStringDifferenceMagnitude =
            currentChunkStringCenter - otherChunkStringCenter;
        
        const uint32_t worstCaseCost = 4;
        
        if(otherChunkStringDifferenceMagnitude == 0)
        {
            intersectionCost = worstCaseCost;
        }

        else
        {
            intersectionCost = 1+(std::abs(otherChunkStringDifferenceMagnitude)/2);
        }
        
        Chunk* farthestChunk = otherChunk->GetPreviousChunk();
        uint32_t furthestChunkStringCenter = 0; 
        
        if((farthestChunk == nullptr) || (farthestChunk == currentChunk))
        {
            farthestChunk = otherChunk->GetNextChunk();
        }
        
        if(farthestChunk != nullptr)
        {
            furthestChunkStringCenter = GetChunkStringCenter(farthestChunk); 
            
            const int32_t furthestChunkStringDifferenceMagnitude = 
                otherChunkStringCenter - furthestChunkStringCenter;
            
            const bool furthestHigherThanClosest = 
                furthestChunkStringDifferenceMagnitude < 0;
            
            const bool closestHigherThanCurrent = 
                otherChunkStringDifferenceMagnitude < 0;
            
            const bool arpeggiationDirectionDiffers = 
                furthestHigherThanClosest != closestHigherThanCurrent;
            
            const bool arpeggiationDirectionMatters = 
                otherChunkStringDifferenceMagnitude != 0;
            
            //Zig-zagging is bad, so double the cost
            if(arpeggiationDirectionDiffers && arpeggiationDirectionMatters)
            {
                intersectionCost *= 2;
            }
        }
        
//        vector<Note*> otherChunkNotes = otherChunk->GetElements();
//        vector<Note*> chunkNotes = currentChunk->GetElements();
//        
//
//        //Same track --> same string: increase cost if the tracks match but the strings dont,
//        //so that voices in the same track tend to stay on the same string
//        for(uint32_t noteIndex=0; noteIndex<chunkNotes.size();noteIndex++)
//        {
//            Note* currentNote = chunkNotes[noteIndex];
//
//            uint32_t trackNumber = currentNote->GetTrackNumber();
//            uint32_t stringIndex = 
//                    currentNote->GetStringIndexForCurrentNotePosition();
//
//            for(uint32_t previousNoteIndex=0; 
//                    previousNoteIndex<otherChunkNotes.size();previousNoteIndex++)
//            {
//                Note* previousNote = otherChunkNotes[previousNoteIndex];
//                uint32_t previousTrackNumber = previousNote->GetTrackNumber();
//
//                uint32_t previousStringIndex = 
//                        previousNote->GetStringIndexForCurrentNotePosition();
//                
//                if(stringIndex == previousStringIndex)
//                {
//                    numberOfDuplicates+= 7;
//                }
//                
//                else
//                {
//                    numberOfDuplicates += std::abs((int)stringIndex - (int)previousStringIndex);
//                }
//            }
//            
//            cout << "Number of duplicates = " << numberOfDuplicates << endl;
//        }
    }
    
    return intersectionCost;
}

void TablatureOptimizer::GetSustainedChunkRelativeFeatures(
    Chunk* chunk,
    uint32_t& stringIntersections,
    uint32_t& fretDistanceFromSustainedNotes,
    bool& goodRelativeFingerSpread)
{
    vector<NotePositionEntry> notePositions = chunk->GetCurrentNotePositionEntries();
    vector<uint32_t> stringPositions = GetStringPositionsOfIndices(notePositions);
    
    uint32_t fretCenterInCandidateChunk = GetChunkFretCenter(chunk);
    
    vector<FretboardPosition> sustainedFretPositions = chunk->GetSustainedFretboardPositions();
    vector<uint32_t> duplicateStrings;
    vector<uint32_t> sustainedStringPositions;
    
    uint32_t frettedNotes;
    uint32_t fretCenterOfSustainedNotes = 0;
    
    for(FretboardPosition fretPosition : sustainedFretPositions)
    {
        const uint32_t fretNumber = fretPosition.FretNumber;
        const uint32_t stringIndex = fretPosition.StringIndex;
        
        sustainedStringPositions.push_back(stringIndex);
        
        if(fretNumber != 0)
        {
            fretCenterOfSustainedNotes += fretNumber;
            frettedNotes++;
        }
    }
    
    if(frettedNotes > 0)
    {
        fretCenterOfSustainedNotes = fretCenterOfSustainedNotes / frettedNotes;
    }
    
    //Find all intersections between sustained string positions and current
    //string positions
    std::set_intersection(
        sustainedStringPositions.begin(),
        sustainedStringPositions.end(),
        stringPositions.begin(),
        stringPositions.end(),
        std::back_inserter(duplicateStrings));
    
    if(fretCenterOfSustainedNotes != 0)
    {
		int32_t fretDistanceSigned = 
			(fretCenterInCandidateChunk - fretCenterOfSustainedNotes);
		
        fretDistanceFromSustainedNotes = 
            abs(fretDistanceSigned);
    }
    
    stringIntersections = duplicateStrings.size();
}

void TablatureOptimizer::GetChunkInternalFeatures(
    Chunk* chunk,
	uint32_t& minimumFretInCandidateChunk,
    uint32_t& maximumFretInCandidateChunk,
    uint32_t& fretCenterInCandidateChunk,
    uint32_t& internalFretHorizontalDistance,
    uint32_t& internalFretVerticalDistance,
    bool& goodInternalFingerSpread)

{
    vector<NotePositionEntry > chunkIndices = 
		chunk->GetCurrentNotePositionEntries();

    //Variables for finding the note center
    uint32_t numberOfZeroFrets = 0;
    uint32_t numberOfFrettedNotes = 0;
    uint32_t fretSumForAverage = 0;
    minimumFretInCandidateChunk = UINT32_MAX;
	uint32_t minimumStringIndex = UINT32_MAX;
	uint32_t maximumStringIndex = 0;

    FretboardPosition maxFretPosition(0,0);
    FretboardPosition minFretPosition(0,0);
    
    for(NotePositionEntry notePositionEntry : chunkIndices)
    {
        const uint32_t currentFret = Note::GetFretForNotePositionEntry(notePositionEntry);            
        const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);
        const FretboardPosition fretPosition(currentString, currentFret);

        //Don't count 0 in the fret span or fret center calculations
        if(currentFret != 0) 
        {
            fretSumForAverage += currentFret;
            numberOfFrettedNotes++;

            //Find the new minimum and maximum for this iteration
            if(currentFret > maximumFretInCandidateChunk)
            {
                maximumFretInCandidateChunk =  currentFret;
                
                maxFretPosition = fretPosition;
            }

            if(currentFret < minimumFretInCandidateChunk)
            {
                minimumFretInCandidateChunk = currentFret;
            
                minFretPosition = fretPosition;
            }
        }

        //Lower the fret maximum
        else
        {
            numberOfZeroFrets++;
        }
		
		minimumStringIndex = std::min(currentString, minimumStringIndex);
		maximumStringIndex = std::max(currentString, maximumStringIndex);
		

    } //end loop

    if(numberOfFrettedNotes != 0)
    {
        fretCenterInCandidateChunk = fretSumForAverage / numberOfFrettedNotes;
    }

    //Spacing cannot go below 0
    if(minimumFretInCandidateChunk <= maximumFretInCandidateChunk)
    {
        internalFretHorizontalDistance = 
            1 + (maximumFretInCandidateChunk - minimumFretInCandidateChunk);
    }
	
    if(minimumStringIndex <= maximumStringIndex)
    {
        internalFretVerticalDistance = 
			1 + (maximumStringIndex - minimumStringIndex);
    }	

    //Prefer higher frets on higher strings than lower strings
    if((minFretPosition.FretNumber != 0) && (maxFretPosition.FretNumber != 0))
    {
        //goodInternalFingerSpread = minFretPosition.StringIndex <= maxFretPosition.StringIndex;
    }
    
    maximumFretInCandidateChunk -= min(maximumFretInCandidateChunk, numberOfZeroFrets);
} // GetChunkInternalFeatures

//Get the average distance from the next N fretted chunks in either the forward
//or reverse direction.
uint32_t TablatureOptimizer::GetDistanceFromAdjacentNChunks(
	const Chunk* const chunk, 
	bool searchForward,
	uint16_t searchLength)
{
	const bool filterForFrettedChunks = true;
	
    const Chunk* const nextFrettedChunk = chunk;
	
	//SearchForClosestOptimizedChunk(chunk, searchForward, filterForFrettedChunks);
    uint32_t fretCenterInCandidateChunk = GetChunkFretCenter(chunk);
	uint32_t sumDifferenceFromPreviousChunks = 0;
	uint32_t numberOfAdjacentFrettedChunks = 0;
	uint32_t averageFretDistance = 0;
	
	//Get the next N fretted chunks
	while(
		(searchLength-- > 0) && 
		(nextFrettedChunk != nullptr))
	{
		const Chunk* const filterForFrettedChunks = 
			SearchForClosestOptimizedChunk(chunk, searchForward, filterForFrettedChunks);
		
		if(filterForFrettedChunks != nullptr)
		{
			uint32_t nextFrettedChunkFretCenter = 
				GetChunkFretCenter(filterForFrettedChunks);
			
			uint32_t distanceFromNextFrettedChunk = 
				abs((int)fretCenterInCandidateChunk - 
				(int)nextFrettedChunkFretCenter);
			
			sumDifferenceFromPreviousChunks += distanceFromNextFrettedChunk;
			numberOfAdjacentFrettedChunks++;
		}
	}
	
	if(numberOfAdjacentFrettedChunks > 0)
	{
		averageFretDistance = 
			sumDifferenceFromPreviousChunks / numberOfAdjacentFrettedChunks;
	}
	
	return averageFretDistance;
}

void TablatureOptimizer::GetAdjacentChunkRelativeFeatures(
	Chunk* chunk,
    uint32_t& distanceFromNearestFrettedChunks,
    uint32_t& intersectionsWithNearestChunks)
{
	uint32_t numberOfPreviousFrettedChunks;
	uint32_t numberOfNextFrettedChunks;
	
    Chunk * const previousFrettedChunk = SearchForClosestOptimizedChunk(chunk, false, true);
    Chunk * const previousChunk = SearchForClosestOptimizedChunk(chunk, false, false);
    
    Chunk * const nextFrettedChunk = SearchForClosestOptimizedChunk(chunk, true, true);
    Chunk * const nextChunk = SearchForClosestOptimizedChunk(chunk, true, false);
	
	uint16_t searchLength = AdjacentChunkSearchLength;
	
	uint32_t distanceFromNextFrettedChunk = GetDistanceFromAdjacentNChunks(
		chunk, 
		true,
		searchLength);
	
	uint32_t distanceFromPreviousFrettedChunk = GetDistanceFromAdjacentNChunks(
		chunk, 
		false,
		searchLength);
	
    uint32_t fretCenterInCandidateChunk = GetChunkFretCenter(chunk);
	
    uint32_t previousFrettedChunkFretCenter = GetChunkFretCenter(previousFrettedChunk);
    uint32_t nextFrettedChunkFretCenter = GetChunkFretCenter(nextFrettedChunk);
    
    uint32_t stringIntersectionsWithPrevious = 
            CountStringIntersectionsBetweenTwoChunkConfigurations(chunk,previousChunk);
    
    uint32_t stringIntersectionsWithNext = 
            CountStringIntersectionsBetweenTwoChunkConfigurations(chunk,nextChunk);
    
//    uint32_t distanceFromPreviousFrettedChunk = 
//            abs((int)fretCenterInCandidateChunk - (int)previousFrettedChunkFretCenter);
//    
//    uint32_t distanceFromNextFrettedChunk = 
//            abs((int)fretCenterInCandidateChunk - (int)nextFrettedChunkFretCenter);
    
    intersectionsWithNearestChunks = 0;
    distanceFromNearestFrettedChunks = 0;
        
    if(fretCenterInCandidateChunk != 0)
    {
        if((previousFrettedChunk != nullptr) && (nextFrettedChunk != nullptr))
        {
            distanceFromNearestFrettedChunks = 
                (distanceFromPreviousFrettedChunk + distanceFromNextFrettedChunk)/2;
        }

        else if(previousFrettedChunk != nullptr)
        {
            distanceFromNearestFrettedChunks = distanceFromPreviousFrettedChunk;
        }

        else if(nextFrettedChunk != nullptr)
        {
            distanceFromNearestFrettedChunks = distanceFromNextFrettedChunk;
        }
    }
    
    if((previousChunk != nullptr) && (nextChunk != nullptr))
    {
        intersectionsWithNearestChunks = 
            stringIntersectionsWithPrevious;// + stringIntersectionsWithNext;
    }
    
    else if(previousChunk != nullptr)
    {
        intersectionsWithNearestChunks = stringIntersectionsWithPrevious;
    }
    
    else if(nextChunk != nullptr)
    {
        //intersectionsWithNearestChunks = stringIntersectionsWithNext;
    }
    
	vector<Chunk*> chunks;
	chunks.push_back(previousFrettedChunk);
	if(previousFrettedChunk != previousChunk)
	{
		chunks.push_back(previousChunk);
	}
	chunks.push_back(chunk);
	
	chunks.push_back(nextChunk);
	if(nextFrettedChunk != nextChunk)
	{
		chunks.push_back(nextFrettedChunk);
	}
	
	const string chunksString = PrintChunks(chunks);
	
    FEATURE_STREAM << "Relative" <<
			chunksString << 
            "\r\n\t\tPrevious fretted chunk:" 
                << ", center = " << previousFrettedChunkFretCenter
                << ", distance = " << distanceFromPreviousFrettedChunk <<
            
            "\r\n\t\tPrevious chunk:" 
                << ", intersections = " << stringIntersectionsWithPrevious <<
            
            "\r\n\t\tNext fretted chunk:" 
                << ", center = " << nextFrettedChunkFretCenter
                << ", distance = " << distanceFromNextFrettedChunk <<
            
            "\r\n\t\tNext chunk:" 
                << ", intersections = " << stringIntersectionsWithNext <<  
            
            "\r\n\t\tResulting features:" 
                << ", avg distance = " << distanceFromNearestFrettedChunks  
                << ", avg intersections = " << intersectionsWithNearestChunks <<  
            
            endl;
} 

/*
 * Reposition a note on the fretboard
 */
void TablatureOptimizer::VisitNote(Note* noteToReposition) 
{
    noteToReposition->ReconfigureToNextPitchmapPosition();
}

bool TablatureOptimizer::MarkConfigurationProcessed(
        vector<NotePositionEntry >& processedChunkConfiguration)
{
    bool chunkAlreadyProcessed = WasConfigurationProcessed(processedChunkConfiguration);
    
    if(!chunkAlreadyProcessed)
    {
        ProcessedChunkConfigurations.push_back(processedChunkConfiguration);
    }
	
	return chunkAlreadyProcessed;
}
 /*
  *	Determine if the chunk configuration has already been evaluated as an optimum
  */	
bool TablatureOptimizer::WasConfigurationProcessed(
        vector<NotePositionEntry > input)
{
    const bool foundEntryInProcessedConfiguration =
        std::end(ProcessedChunkConfigurations) != 
        (std::find(std::begin(ProcessedChunkConfigurations), 
                   std::end(ProcessedChunkConfigurations), input));

    return foundEntryInProcessedConfiguration;
}


/*
 *	Empty cache of already checked configurations
 */	
void TablatureOptimizer::ResetMarkedChunks(void)
{
	ProcessedChunkConfigurations.clear();
}
