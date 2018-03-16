#include "rotate_visitor.h"
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <assert.h>

using namespace std;

//ostream& LogStream = cout;
ostream LogStream(nullptr);


RotateVisitor::RotateVisitor(
        uint32_t numberOfStrings,
        uint32_t maximumFretScalar,
        uint32_t fretSpanScalar,
        uint32_t interChunkSpacingScalar,
        uint32_t stringOverlapScalar) 
    : 
        StringIndexedFrettedNotes(numberOfStrings),
        StringIndexedRemainingDeltaTicks(numberOfStrings),
        MaximumFretScalar(maximumFretScalar),
        FretSpanScalar(fretSpanScalar),
        InterChunkSpacingScalar(interChunkSpacingScalar),
        StringOverlapScalar(stringOverlapScalar),
        PreviousChunk(nullptr),
        PreviousFrettedChunk(nullptr)
         
{
    
}

RotateVisitor::~RotateVisitor(void) 
{
}

//Go through all of the chunks in a given bar and reconfigure them
void RotateVisitor::VisitBar(Bar* bar) 
{
    for(uint32_t barIndex=0; barIndex < bar->GetNumberOfElements(); barIndex++)
    {
        Chunk* candidateChunk = bar->GetElementWithIndex(barIndex);
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
    chunkFretCenter = GetChunkFretCenter(candidateChunk);
    
    UpdateStringIndexedRemainingDeltaTicks(candidateChunk);

    PreviousChunk = candidateChunk;
    
    if(chunkFretCenter != 0)
    {
        PreviousFrettedChunk = candidateChunk;
    }
    
    LogStream << "Optimized chunk: " << Chunk::PrintNoteIndices(candidateChunk->GetCurrentNotePositionEntries()) << endl;
    

} //end VisitChunk

void RotateVisitor::UpdateStringIndexedRemainingDeltaTicks(
    Chunk* candidateChunk)
{
    Chunk* previousChunk = PreviousChunk;
    
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

//Change the note positions in this chunk to a configuration that has not 
//been checked yet. Skip configurations with impossible fingerings. 
uint32_t RotateVisitor::ReconfigureChunk(
        Chunk* candidateChunk, 
        uint32_t permutationCount,
        bool& morePermutations)
{
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
                WasChunkProcessed(candidateChunk->GetCurrentNotePositionEntries());
        
        shouldContinue = stringsOverlap ||  (currentNotePositionsAlreadyTried && 
                                             morePermutations);
        
        if(exhaustedAllPermutations)
        {
            //todo: prevent this from happening by cleaning chunks properly
            errorCount++;
            LogStream << "\tError " <<errorCount << " " << Chunk::PrintNoteIndices(currentNotePositionsEntries) << " " << stringsOverlap << " " << currentNotePositionsAlreadyTried << " " << morePermutations << endl;
        }
        
    } while(shouldContinue && (permutationCount < permutationsForThisChunk) && errorCount < 10);
    
    
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
    
    const bool alreadyCheckedThisConfiguration = WasChunkProcessed(candidateChunkFingering);
    
    if(!alreadyCheckedThisConfiguration)
    {
        const uint32_t candidateCost = CalculateConfigurationCost(candidateChunkFingering);
        
        if(candidateCost < currentLowestCost)
        {            
            chunkToConfigure->SetOptimalNotePositions(candidateChunkFingering);
            chunkToConfigure->RepositionNotesToCurrentOptimalPositions();
            
            currentLowestCost = candidateCost;
        }
        
        MarkChunkAsProcessed(candidateChunkFingering);   
        
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

void RotateVisitor::GetStringOverlapStuff(
            vector<NotePositionEntry> notePositions, 
            uint32_t fretCenterInCandidateChunk, 
            uint32_t& candidateSpacingFromLastChunk,
            uint32_t& intersectionsWithPreviousChunk)
{
    uint32_t candidateSpacingFromPreviousChunk = 0;
    uint32_t candidateSpacingFromFrettedNotes = 0;
    uint32_t fretCenterInFrettedNotes = 0;
    
    uint32_t maximumFretInPreviousChunk = 0;
    uint32_t fretSpacingInPreviousChunk = 0;
    uint32_t fretCenterInPreviousChunk = 0;
    uint32_t sustainDontCare = 0;
    
    vector<uint32_t> stringPositionsInPreviousChunk;
    
    Chunk * const previousFrettedChunk = PreviousFrettedChunk;
    Chunk * const previousChunk = PreviousChunk;
    
    if(previousChunk != nullptr)
    {
        vector<NotePositionEntry > previousChunkIndices = 
            previousChunk->GetCurrentNotePositionEntries();
        
        intersectionsWithPreviousChunk = 
                CountStringIntersectionsBetweenTwoChunkConfigurations(
                previousChunkIndices, 
                notePositions);
    }
    
    {
        uint32_t numberOfFrettedNotes = 0;
        uint32_t fretSumForAverage = 0;

        for(uint32_t fretNumber : StringIndexedFrettedNotes)
        {
            //Don't count 0 in the fret span or fret center calculations
            if(fretNumber != 0) 
            {
                fretSumForAverage += fretNumber;
                numberOfFrettedNotes++;
            }

        } //end loop

        if(numberOfFrettedNotes != 0)
        {
            fretCenterInFrettedNotes = fretSumForAverage / numberOfFrettedNotes;
        }
    }
    
    if(previousFrettedChunk != nullptr)
    {
        bool previousChunkValid = true;
        
        vector<NotePositionEntry > previousChunkIndices = 
            previousFrettedChunk->GetCurrentNotePositionEntries();
        
        LogStream << "\tFeaturesTrace: Previous chunk." << endl;
        previousChunkValid = GetChunkFeatures(previousChunkIndices,
                maximumFretInPreviousChunk, fretSpacingInPreviousChunk,
                fretCenterInPreviousChunk, sustainDontCare,
                stringPositionsInPreviousChunk);
    
        if(previousChunkValid)
        {
            //If either fret center is 0, the fret center is not a good measure of 
            //how far the hand has to move between the two chunks
            if(fretCenterInCandidateChunk != 0)
            {
                candidateSpacingFromPreviousChunk = 
                    std::abs((int32_t)fretCenterInCandidateChunk - 
                             (int32_t)fretCenterInPreviousChunk);
            }
        }
    }
    
    if((fretCenterInCandidateChunk != 0) && (fretCenterInFrettedNotes != 0))
    {
        candidateSpacingFromFrettedNotes = 
            std::abs((int32_t)fretCenterInCandidateChunk - 
                     (int32_t)fretCenterInFrettedNotes);
    }
    
    //Todo: incorporate other chunk data?
    candidateSpacingFromLastChunk = candidateSpacingFromFrettedNotes;
    
}

uint32_t RotateVisitor::CalculateConfigurationCost(
        vector<NotePositionEntry > indices)
{
    uint32_t chunkCost = UINT32_MAX;
    
    uint32_t maximumFretInCandidateChunk;
    uint32_t fretSpacingInCandidateChunk;
    uint32_t fretCenterInCandidateChunk;
    uint32_t sustainInterruptionsInCandidateChunk;
    vector<uint32_t> stringPositions;
    
    
    LogStream << "\tFeaturesTrace: Candidate chunk." << endl;
    bool chunkValid = GetChunkFeatures(
        indices, maximumFretInCandidateChunk,
        fretSpacingInCandidateChunk,fretCenterInCandidateChunk,
        sustainInterruptionsInCandidateChunk, stringPositions);

    
    //If the chunk is not valid, return the largest possible cost
    if(chunkValid)
    {
        uint32_t candidateSpacingFromLastChunk = 0;
        uint32_t numberOfDuplicateStrings = 0;
        
        GetStringOverlapStuff(indices, fretCenterInCandidateChunk, 
            candidateSpacingFromLastChunk,numberOfDuplicateStrings);
    
        LogStream << "\tCostTrace:" << Chunk::PrintNoteIndices(indices) << endl;
        chunkCost =  EvaluateConfigurationFeatures(
            maximumFretInCandidateChunk,
            fretSpacingInCandidateChunk, 
            candidateSpacingFromLastChunk,
            sustainInterruptionsInCandidateChunk, 
            numberOfDuplicateStrings);
    }
    
    return chunkCost;
    
} //end CalculateConfigurationCost


uint32_t RotateVisitor::GetChunkFretCenter(
        Chunk* candidateChunk)
{
    uint32_t fretCenterInCandidateChunk = 0;
    uint32_t numberOfFrettedNotes = 0;
    uint32_t fretSumForAverage = 0;
    vector<NotePositionEntry > chunkIndices = 
        candidateChunk->GetCurrentNotePositionEntries();
    
    for(NotePositionEntry notePositionEntry : chunkIndices)
    {
        const uint32_t currentFret = Note::GetFretForNotePositionEntry(notePositionEntry);        

        //Don't count 0 in the fret span or fret center calculations
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
    
    return fretCenterInCandidateChunk;
}

/*
 * Find the maximum fret and total fret spacing in a vector of note grid positions.
 * Return the fret spacing and maximum fret as output parameters
*/
bool RotateVisitor::GetChunkFeatures(
        vector<NotePositionEntry > chunkIndices,
        uint32_t& maximumFretInCandidateChunk,
        uint32_t& fretSpacingInCandidateChunk,
        uint32_t& fretCenterInCandidateChunk,
        uint32_t& sustainedNoteInterruptions,
        vector<uint32_t>& stringPositions)
{
    maximumFretInCandidateChunk = 0;
	fretSpacingInCandidateChunk = 0;
    fretCenterInCandidateChunk = 0;
    
    //Variables for finding the note center
    uint32_t numberOfNotes = chunkIndices.size();
    
    const bool validChunk = (numberOfNotes != 0);
    
    if(validChunk)
    {
        uint32_t numberOfFrettedNotes = 0;
        uint32_t fretSumForAverage = 0;
        uint32_t minimumFretInChunkConfiguration = UINT32_MAX;
        
        for(NotePositionEntry notePositionEntry : chunkIndices)
        {
            const uint32_t currentFret = Note::GetFretForNotePositionEntry(notePositionEntry);            
            const uint32_t currentString = Note::GetStringForNotePositionEntry(notePositionEntry);
            
            stringPositions.push_back(currentString);

            //Don't count 0 in the fret span or fret center calculations
            if(currentFret != 0) 
            {
                fretSumForAverage += currentFret;
                numberOfFrettedNotes++;

                //Find the new minimum and maximum for this iteration
                maximumFretInCandidateChunk = 
                        std::max(maximumFretInCandidateChunk, currentFret);

                minimumFretInChunkConfiguration = 
                        std::min(minimumFretInChunkConfiguration,currentFret);
            }
        } //end loop

        if(numberOfFrettedNotes != 0)
        {
            fretCenterInCandidateChunk = fretSumForAverage / numberOfFrettedNotes;
        }
        
        //Spacing cannot go below 0
        if(minimumFretInChunkConfiguration <= maximumFretInCandidateChunk)
        {
            fretSpacingInCandidateChunk = maximumFretInCandidateChunk - 
                                            minimumFretInChunkConfiguration;
        }
        
        sustainedNoteInterruptions = CountStringIntersectionsWithFrettedNotes(chunkIndices);
        
        //Trace
        {
            stringstream stringIndices;

            stringIndices<< "|";

            for(uint32_t stringIndex : stringPositions)
            {
                stringIndices<< stringIndex << " ";
            }

            stringIndices<< "|";

            LogStream << "\t" << Chunk::PrintNoteIndices(chunkIndices) << "FeaturesTrace:" << 
                "\r\n\t\tMax Fret:" << maximumFretInCandidateChunk << " " 
                "\r\n\t\tSpan:" <<fretSpacingInCandidateChunk << " " <<
                "\r\n\t\tFret Center:" << fretCenterInCandidateChunk << " = " << fretSumForAverage << "/" << numberOfFrettedNotes <<
                "\r\n\t\tString positions:" << stringIndices.str() << endl;
        } 
        
    } //end if valid chunk
    
    return validChunk;
    
} //end GetChunkFeatures


uint32_t RotateVisitor::EvaluateConfigurationFeatures(
        uint32_t maximumFretInCandidateChunk,
        uint32_t fretSpacingInCandidateChunk,
        uint32_t distanceFromPreviousChunk,
        uint32_t sustainInterruptionsInCandidateChunk,
        uint32_t stringIntersections)
{
    //TODO: should stringIntersections matter?
    const uint32_t interChunkSpacingCost = distanceFromPreviousChunk*InterChunkSpacingScalar;
    const uint32_t stringOverlapCost = sustainInterruptionsInCandidateChunk * StringOverlapScalar;
    
    const uint32_t maximumFretCost = maximumFretInCandidateChunk*MaximumFretScalar;
    const uint32_t fretSpanCost = fretSpacingInCandidateChunk*FretSpanScalar;
    
    const uint32_t candidateCost = maximumFretCost + fretSpanCost + 
                     interChunkSpacingCost + stringOverlapCost;
    
    LogStream << "\tCostTrace: $" << candidateCost << 
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

uint32_t RotateVisitor::CountStringIntersectionsBetweenTwoChunkConfigurations(
    vector<NotePositionEntry> notePositions1, 
    vector<NotePositionEntry> notePositions2)
{
    vector<uint32_t> duplicateStrings;
    
    vector<uint32_t> stringPositions1 = GetStringPositionsOfIndices(notePositions1);
    vector<uint32_t> stringPositions2 = GetStringPositionsOfIndices(notePositions2);

    std::set_intersection(stringPositions1.begin(),
                          stringPositions1.end(),
                          stringPositions2.begin(),
                          stringPositions2.end(),
                          std::back_inserter(duplicateStrings));
    
    const uint32_t numberOfDuplicates = duplicateStrings.size();
    
    return numberOfDuplicates;
}

uint32_t RotateVisitor::CountStringIntersectionsWithFrettedNotes(
    vector<NotePositionEntry> notePositions)
{
    uint32_t stringIntersectionsWithFrettedNotes = 0;
    uint32_t comparisonDelta = 0;
    
    if(PreviousChunk != nullptr)
    {
        comparisonDelta = PreviousChunk->GetDelta(); //compare with 0 if this doesnt work TODO
    }
    
    vector<uint32_t> stringPositions = GetStringPositionsOfIndices(notePositions);
    for(uint32_t stringIndex : stringPositions)
    {
        bool stringAlreadyInUse = StringIndexedRemainingDeltaTicks[stringIndex] > comparisonDelta;
        if(stringAlreadyInUse)
        {
            stringIntersectionsWithFrettedNotes++;
        }
    }
    
    return stringIntersectionsWithFrettedNotes;
}

/*
 * Reposition a note on the fretboard
 */
void RotateVisitor::VisitNote(Note* noteToReposition) 
{
    noteToReposition->ReconfigureToNextPitchmapPosition();
}

bool RotateVisitor::MarkChunkAsProcessed(
        vector<NotePositionEntry > processedChunkConfiguration)
{
    bool chunkAlreadyProcessed = WasChunkProcessed(processedChunkConfiguration);
    
    if(!chunkAlreadyProcessed)
    {
        ProcessedChunkConfigurations.push_back(processedChunkConfiguration);
    }
}
 /*
  *	Determine if the chunk configuration has already been evaluated as an optimum
  */	
bool RotateVisitor::WasChunkProcessed(
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
