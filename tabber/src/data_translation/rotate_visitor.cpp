#include "rotate_visitor.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <assert.h>

using namespace std;

stringstream fileStringStream;
    
ostream& LogStream = fileStringStream;
ostream NullStream(nullptr);

#define FEATURE_STREAM NullStream << "\tFeatures: "
#define COST_STREAM NullStream << "\tCosts: "
#define ERROR_STREAM NullStream<< "\tError: "
#define RESULT_STREAM LogStream<< "Results: "



RotateVisitor::RotateVisitor(
        uint32_t numberOfStrings,
        uint32_t maximumFretScalar,
        uint32_t fretSpanScalar,
        uint32_t interChunkSpacingScalar,
        uint32_t stringOverlapScalar) 
    : 
        StringIndexedFrettedNotes(numberOfStrings),
        StringIndexedRemainingDeltaTicks(numberOfStrings),
        NumberOfStrings(numberOfStrings),
        MaximumFretScalar(maximumFretScalar),
        FretSpanScalar(fretSpanScalar),
        InterChunkSpacingScalar(interChunkSpacingScalar),
        StringOverlapScalar(stringOverlapScalar),
        PreviousChunky(nullptr),
        PreviousFrettedChunky(nullptr)
         
{
    
}

RotateVisitor::~RotateVisitor(void) 
{    
    ofstream outputFileStream;
    
	outputFileStream.open("debugOutput.txt");
	outputFileStream << LogStream.rdbuf();
	outputFileStream.close();
}

//Go through all of the chunks in a given bar and reconfigure them
void RotateVisitor::VisitBar(Bar* bar) 
{
    for(uint32_t barIndex=0; barIndex < bar->GetNumberOfElements(); barIndex++)
    {
        Chunk* candidateChunk = bar->GetElementWithIndex(barIndex);
        
        candidateChunk->CleanChunk();
        candidateChunk->DispatchVisitor(this);
    }
}

//Visit a chunk, and re-arrange all of its notes until they are valid/playable
void RotateVisitor::VisitChunk(Chunk* candidateChunk) 
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

    PreviousChunky = candidateChunk;
    
    if(chunkFretCenter != 0)
    {
        PreviousFrettedChunky = candidateChunk;
    }
    
    RESULT_STREAM << "Optimized chunk: " << Chunk::PrintNoteIndices(candidateChunk->GetCurrentNotePositionEntries()) << endl;
    

} //end VisitChunk

void RotateVisitor::UpdateStringIndexedRemainingDeltaTicks(
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
Chunk* RotateVisitor::SearchForClosestOptimizedChunk(
        Chunk* currentChunk, 
        bool searchForward,
        bool frettedChunksOnly)
{
    const uint32_t maximumUnOptimizedSearchLength = 2;
    
    Chunk* discoveredChunk = nullptr;
    uint32_t searchCounter = 0;
    
    while(currentChunk != nullptr)
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
        
        if((currentChunk != nullptr) && currentChunk->GetIsOptimized())
        {
            if(((frettedChunksOnly) && (GetChunkFretCenter(currentChunk) != 0)) ||
                !frettedChunksOnly)
            {
                discoveredChunk = currentChunk;
                break;
            }
        }
        
        else if(searchCounter == maximumUnOptimizedSearchLength)
        {
            break;
        }
    }
    
    return discoveredChunk;
}

void RotateVisitor::LockStringsInTheNextFewChunksForThisConfiguration(Chunk *chunk)
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
uint32_t RotateVisitor::ReconfigureChunk(
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
        if(exhaustedAllPermutations)
        {
            //todo: prevent this from happening by cleaning chunks properly
            errorCount++;
            ERROR_STREAM << "\tError: exhausted all permutations " <<errorCount << " " << 
                    Chunk::PrintNoteIndices(currentNotePositionsEntries) 
                    << ", overlap: " << stringsOverlap 
                    << ", alreadyTried: " << currentNotePositionsAlreadyTried 
                    << ", morePermutations: " << morePermutations << endl;
        }
        
        if(!(shouldContinue && errorCount < 10))
        {
            ERROR_STREAM << "\tExiting because " << 
                    Chunk::PrintNoteIndices(currentNotePositionsEntries) 
                    << ", permutations = " << permutationCount << "/" << permutationsForThisChunk
                    << ", errors = " << "errorCount" << "/" << maxErrorsAllowed
                    << ", overlap: " << stringsOverlap 
                    << ", alreadyTried: " << currentNotePositionsAlreadyTried 
                    << ", morePermutations: " << morePermutations << endl;
        }
        
    } while(shouldContinue && (errorCount < maxErrorsAllowed));
    
    
    return octaveShiftCost;
    
} //end ReconfigureChunk


//Pick between the current note positions and the current optimum note positions
//Apply the lower cost one 
void RotateVisitor::SelectOptimalFingering(
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


bool RotateVisitor::RotateNoteOrItsParent(
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
bool RotateVisitor::ValidateStringOverlapsForNotePositions(vector<NotePositionEntry> notePositionsEntries)
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
    
    for (auto kv : stringToOverlapCountMap)
    {
    }
    
    return stringsOverlap;
}


void RotateVisitor::GetAdjacentChunkRelativeFeatures(Chunk* chunk,
            uint32_t& distanceFromNearestFrettedChunks,
            uint32_t& intersectionsWithNearestChunks)
{
    
    Chunk * const previousFrettedChunk = SearchForClosestOptimizedChunk(chunk, false, true);
    Chunk * const previousChunk = SearchForClosestOptimizedChunk(chunk, false, false);
    
    Chunk * const nextFrettedChunk = SearchForClosestOptimizedChunk(chunk, true, true);
    Chunk * const nextChunk = SearchForClosestOptimizedChunk(chunk, true, false);
    
        
    uint32_t fretCenterInCandidateChunk = GetChunkFretCenter(chunk);
    uint32_t previousFrettedChunkFretCenter = GetChunkFretCenter(previousFrettedChunk);
    uint32_t nextFrettedChunkFretCenter = GetChunkFretCenter(nextFrettedChunk);
    
    uint32_t stringIntersectionsWithPrevious = 
            CountStringIntersectionsBetweenTwoChunkConfigurations(chunk,previousChunk);
    
    uint32_t stringIntersectionsWithNext = 
            CountStringIntersectionsBetweenTwoChunkConfigurations(chunk,nextChunk);
    
    uint32_t distanceFromPreviousFrettedChunk = 
            abs((int)fretCenterInCandidateChunk - (int)previousFrettedChunkFretCenter);
    
    uint32_t distanceFromNextFrettedChunk = 
            abs((int)fretCenterInCandidateChunk - (int)nextFrettedChunkFretCenter);
    
    intersectionsWithNearestChunks = 0;
    distanceFromNearestFrettedChunks = 0;
        
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
    
    if((previousChunk != nullptr) && (nextChunk != nullptr))
    {
        intersectionsWithNearestChunks = 
            (stringIntersectionsWithPrevious + stringIntersectionsWithNext)/2;
    }
    
    else if(previousChunk != nullptr)
    {
        intersectionsWithNearestChunks = stringIntersectionsWithPrevious;
    }
    
    else if(nextChunk != nullptr)
    {
        intersectionsWithNearestChunks = stringIntersectionsWithNext;
    }
    
    FEATURE_STREAM <<
            "\r\n\t\tPrevious fretted chunk:" 
                << Chunk::PrintChunk(previousFrettedChunk) 
                << ", center = " << previousFrettedChunkFretCenter
                << ", distance = " << distanceFromPreviousFrettedChunk <<
            
            
            "\r\n\t\tPrevious chunk:" 
                << Chunk::PrintChunk(previousChunk)
                << ", intersections = " << stringIntersectionsWithPrevious <<
            
            "\r\n\t\tNext fretted chunk:" 
                << Chunk::PrintChunk(nextFrettedChunk) << ", center = " 
                << nextFrettedChunkFretCenter
                << ", distance = " << distanceFromNextFrettedChunk <<
            
            "\r\n\t\tNext chunk:" 
                << Chunk::PrintChunk(nextChunk) 
                << ", intersections = " << stringIntersectionsWithNext <<                        
            endl;
} 

uint32_t RotateVisitor::CalculateConfigurationCost(
    Chunk* chunk)
{
    uint32_t chunkCost = UINT32_MAX;
    
   ChunkFeatures candidateChunkFeatures;
    
    vector<NotePositionEntry > indices = chunk->GetCurrentNotePositionEntries();
    
    FEATURE_STREAM << "Candidate chunk." << endl;
    GetChunkFeatures(chunk, candidateChunkFeatures);

    COST_STREAM << Chunk::PrintNoteIndices(indices) << endl;
    chunkCost =  EvaluateConfigurationFeatures(candidateChunkFeatures);
    
    
    return chunkCost;
    
} //end CalculateConfigurationCost


uint32_t RotateVisitor::GetChunkFretCenter(
        Chunk* candidateChunk)
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

/*
 * Find the maximum fret and total fret spacing in a vector of note grid positions.
 * Return the fret spacing and maximum fret as output parameters
*/
void RotateVisitor::GetChunkFeatures(
        Chunk* chunk,
        ChunkFeatures& chunkFeatures)
{
    vector<NotePositionEntry > chunkIndices = chunk->GetCurrentNotePositionEntries();
    uint32_t fretCenterOfSustainedNotes = 0;
    chunkFeatures.fretDistanceFromSustainedNotes = 0;
    chunkFeatures.maximumFretInCandidateChunk = 0;
	chunkFeatures.fretSpacingInCandidateChunk = 0;
    chunkFeatures.fretCenterInCandidateChunk = 0;
    
    //Variables for finding the note center
    uint32_t numberOfZeroFrets = 0;
    uint32_t numberOfFrettedNotes = 0;
    uint32_t fretSumForAverage = 0;
    uint32_t minimumFretInChunkConfiguration = UINT32_MAX;

    for(NotePositionEntry notePositionEntry : chunkIndices)
    {
        const uint32_t currentFret = Note::GetFretForNotePositionEntry(notePositionEntry);            
        const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);

        chunkFeatures.stringPositions.push_back(currentString);

        //Don't count 0 in the fret span or fret center calculations
        if(currentFret != 0) 
        {
            fretSumForAverage += currentFret;
            numberOfFrettedNotes++;

            //Find the new minimum and maximum for this iteration
            chunkFeatures.maximumFretInCandidateChunk = 
                    std::max(chunkFeatures.maximumFretInCandidateChunk, currentFret);

            minimumFretInChunkConfiguration = 
                    std::min(minimumFretInChunkConfiguration,currentFret);
        }

        //Lower the fret maximum
        else
        {
            numberOfZeroFrets++;
        }

    } //end loop

    if(numberOfFrettedNotes != 0)
    {
        chunkFeatures.fretCenterInCandidateChunk = fretSumForAverage / numberOfFrettedNotes;
    }

    //Spacing cannot go below 0
    if(minimumFretInChunkConfiguration <= chunkFeatures.maximumFretInCandidateChunk)
    {
        chunkFeatures.fretSpacingInCandidateChunk = chunkFeatures.maximumFretInCandidateChunk - 
                                        minimumFretInChunkConfiguration;
    }

    CountStringIntersectionsWithFrettedNotes(chunk,
            chunkFeatures.sustainInterruptions,
            fretCenterOfSustainedNotes);

    if(fretCenterOfSustainedNotes != 0)
    {
        chunkFeatures.fretDistanceFromSustainedNotes = 
            abs(chunkFeatures.fretCenterInCandidateChunk - fretCenterOfSustainedNotes);
    }

    GetAdjacentChunkRelativeFeatures(chunk,
        chunkFeatures.candidateSpacingFromLastChunk,
        chunkFeatures.numberOfDuplicateStrings);
    
    
    //Trace
    {
        stringstream stringIndices;

        stringIndices<< "|";

        for(uint32_t stringIndex : chunkFeatures.stringPositions)
        {
            stringIndices<< stringIndex << " ";
        }

        stringIndices<< "|";

        
        FEATURE_STREAM << Chunk::PrintNoteIndices(chunkIndices) << 
            "\r\n\t\tMax Fret:" << chunkFeatures.maximumFretInCandidateChunk << " " 
            "\r\n\t\tSpan:" << chunkFeatures.fretSpacingInCandidateChunk << " " <<
            "\r\n\t\tFret Center:" << chunkFeatures.fretCenterInCandidateChunk << 
            "\r\n\t\tDistance from sustained notes:" << chunkFeatures.fretDistanceFromSustainedNotes << 
            "\r\n\t\tSustain interruptions:" << chunkFeatures.sustainInterruptions <<
            "\r\n\t\tRelative spacing:" << chunkFeatures.candidateSpacingFromLastChunk << 
            endl;
    }
    
} //end GetChunkFeatures


uint32_t RotateVisitor::EvaluateConfigurationFeatures(
        ChunkFeatures chunkFeatures)
{
    //const float stringIntersectionsCost = stringIntersections*0;
    const float stringOverlapCost = chunkFeatures.sustainInterruptions * StringOverlapScalar;
    
    const float interChunkSpacingCost = pow(chunkFeatures.candidateSpacingFromLastChunk,2)*InterChunkSpacingScalar;
    
    const float maximumFretCost = (chunkFeatures.maximumFretInCandidateChunk)*MaximumFretScalar;
    const float fretSpanCost = pow(chunkFeatures.fretSpacingInCandidateChunk,2)*FretSpanScalar;
    
    const uint32_t candidateCost = stringOverlapCost + 
                        maximumFretCost + fretSpanCost + interChunkSpacingCost; 
        
    
    COST_STREAM << "$" << candidateCost << 
            "\r\n\t\tMaxFret:" << maximumFretCost << 
            "\r\n\t\tFretSpan:" << fretSpanCost << 
            "\r\n\t\tAdjacency:" << interChunkSpacingCost << 
            "\r\n\t\tIndependence:" << stringOverlapCost << endl;
    
    return candidateCost;
    
} //end EvaluateConfigurationFeatures


vector<uint32_t> RotateVisitor::GetStringPositionsOfIndices(
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


vector<uint32_t> RotateVisitor::GetStringPositions(
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


uint32_t RotateVisitor::CountStringIntersectionsBetweenTwoChunkConfigurations(
    Chunk* otherChunk,
    Chunk* currentChunk)
{    
    uint32_t numberOfDuplicates = 0;
    
    if((currentChunk !=nullptr) && (otherChunk != nullptr))
    {
    vector<Note*> otherChunkNotes = otherChunk->GetElements();
    vector<Note*> chunkNotes = currentChunk->GetElements();
    
    //Same track --> same string: increase cost if the tracks match but the strings dont,
    //so that voices in the same track tend to stay on the same string
    for(uint32_t noteIndex=0; noteIndex<chunkNotes.size();noteIndex++)
    {
        Note* currentNote = chunkNotes[noteIndex];
        
        uint32_t trackNumber = currentNote->GetTrackNumber();
        uint32_t stringIndex = 
                currentNote->GetStringIndexForCurrentNotePosition();
        
        for(uint32_t previousNoteIndex=0; 
                previousNoteIndex<otherChunkNotes.size();previousNoteIndex++)
        {
            Note* previousNote = otherChunkNotes[previousNoteIndex];
            uint32_t previousTrackNumber = previousNote->GetTrackNumber();
            
            uint32_t previousStringIndex = 
                    previousNote->GetStringIndexForCurrentNotePosition();
            
            if((trackNumber==previousTrackNumber) && 
               (stringIndex != previousStringIndex))
            {
                numberOfDuplicates++;
            }
        }
    }
    
//    vector<NotePositionEntry> notePositions1 = previousChunk->GetCurrentNotePositionEntries();
//    vector<NotePositionEntry> notePositions2 = currentChunk->GetCurrentNotePositionEntries();
//    vector<uint32_t> stringPositions1 = GetStringPositionsOfIndices(notePositions1);
//    vector<uint32_t> stringPositions2 = GetStringPositionsOfIndices(notePositions2);
//    vector<uint32_t> duplicateStrings;
//
//    std::set_intersection(stringPositions1.begin(),
//                          stringPositions1.end(),
//                          stringPositions2.begin(),
//                          stringPositions2.end(),
//                          std::back_inserter(duplicateStrings));
    
    //numberOfDuplicates = duplicateStrings.size();
    }
    
    return numberOfDuplicates;
}

void RotateVisitor::CountStringIntersectionsWithFrettedNotes(
    Chunk* chunk,
    uint32_t& stringIntersections,
    uint32_t& fretCenterOfSustainedNotes)
{
    vector<NotePositionEntry> notePositions = chunk->GetCurrentNotePositionEntries();
    vector<uint32_t> stringPositions = GetStringPositionsOfIndices(notePositions);
    
    vector<FretboardPosition> sustainedFretPositions = chunk->GetSustainedFretboardPositions();
    vector<uint32_t> duplicateStrings;
    vector<uint32_t> sustainedStringPositions;
    
    uint32_t frettedNotes;
    fretCenterOfSustainedNotes = 0;
    
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
    
    std::set_intersection(sustainedStringPositions.begin(),
                          sustainedStringPositions.end(),
                          stringPositions.begin(),
                          stringPositions.end(),
                          std::back_inserter(duplicateStrings));
    
    stringIntersections = duplicateStrings.size();
}

/*
 * Reposition a note on the fretboard
 */
void RotateVisitor::VisitNote(Note* noteToReposition) 
{
    noteToReposition->ReconfigureToNextPitchmapPosition();
}

bool RotateVisitor::MarkConfigurationProcessed(
        vector<NotePositionEntry > processedChunkConfiguration)
{
    bool chunkAlreadyProcessed = WasConfigurationProcessed(processedChunkConfiguration);
    
    if(!chunkAlreadyProcessed)
    {
        ProcessedChunkConfigurations.push_back(processedChunkConfiguration);
    }
}
 /*
  *	Determine if the chunk configuration has already been evaluated as an optimum
  */	
bool RotateVisitor::WasConfigurationProcessed(
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
void RotateVisitor::ResetMarkedChunks(void)
{
	ProcessedChunkConfigurations.clear();
}
