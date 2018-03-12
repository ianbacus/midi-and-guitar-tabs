#include "rotate_visitor.h"
#include <algorithm>
#include <unordered_map>
#include <assert.h>

using namespace std;

ostream LogStream(nullptr);

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
    const uint32_t chunkSize = candidateChunk->GetNumberOfElements() ;

    const uint32_t permutationsForThisChunk = 
        candidateChunk->GetNumberOfPositionPermutations();

    uint32_t noteConfigurationIndex = chunkSize-1;

    uint32_t permutationIndex = 0;
    uint32_t currentLowestCost = UINT32_MAX;
    
    
    while(permutationIndex < permutationsForThisChunk)
    {
        permutationIndex++;
        
        //Compare the current best chunk configuration against the reconfigured one
        SelectOptimalFingering(candidateChunk, currentLowestCost);	
        
        //Reconfigure or "rotate" the chunk until it is in another configuration
        ReconfigureChunk(candidateChunk,noteConfigurationIndex);

    }
    
    candidateChunk->RepositionNotesToCurrentOptimalPositions();
    uint32_t chunkFretCenter = GetChunkFretCenter(candidateChunk);
    if(chunkFretCenter != 0)
    {
        PreviousChunk = candidateChunk;
    }
    
    LogStream << "Optimized chunk: " << Chunk::PrintNoteIndices(candidateChunk->GetCurrentNotePositionEntries());
    
    ResetMarkedChunks();

} //end VisitChunk

//Change the note positions in this chunk to a configuration that has not 
//been checked yet. Skip configurations with impossible fingerings. 
uint32_t RotateVisitor::ReconfigureChunk(
        Chunk* candidateChunk, 
        uint32_t& noteConfigurationIndex)
{
    uint32_t octaveShiftCost = 0;
    bool stringsOverlap = false;
    bool currentNotePositionsAlreadyTried = true;
    bool allNotePositionsTried = false;
    
    do
    {
        allNotePositionsTried = RotateNoteOrItsParent(candidateChunk,
                noteConfigurationIndex,octaveShiftCost);
        
        vector<NotePositionEntry> currentNotePositionsEntries =
                candidateChunk->GetCurrentNotePositionEntries();
        
        vector<uint32_t> stringIndices = GetStringPositionsOfIndices(currentNotePositionsEntries);
        uint32_t numberOfStringIndices = stringIndices.size();
        unordered_map<uint32_t, uint32_t> stringToOverlapCountMap;
        for(uint32_t index=0; index<numberOfStringIndices; index++)
        {
            uint32_t stringIndex = stringIndices[index];
            stringToOverlapCountMap[stringIndex]++;
        }
        for(auto keyVal : stringToOverlapCountMap)
        {
            stringsOverlap = stringsOverlap || (keyVal.second > 1);
        }
        
        currentNotePositionsAlreadyTried = 
                WasChunkProcessed(candidateChunk->GetCurrentNotePositionEntries());
        
        if(stringsOverlap)
        {
            MarkChunkAsProcessed(currentNotePositionsEntries);
        }

    } while((stringsOverlap && currentNotePositionsAlreadyTried) && !allNotePositionsTried);
    
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

uint32_t RotateVisitor::CalculateConfigurationCost(
        vector<NotePositionEntry > indices)
{
    uint32_t chunkCost = UINT32_MAX;
    
    uint32_t maximumFretInCandidateChunk;
    uint32_t fretSpacingInCandidateChunk;
    uint32_t fretCenterInCandidateChunk;
    vector<uint32_t> stringPositions;
    
    uint32_t maximumFretInPreviousChunk = 0;
    uint32_t fretSpacingInPreviousChunk = 0;
    uint32_t fretCenterInPreviousChunk = 0;
    vector<uint32_t> stringPositionsInPreviousChunk;
    
    vector<uint32_t> duplicateStrings;
    uint32_t candidateSpacingFromLastChunk = 0;
    
    Chunk * const previousChunk = PreviousChunk;

    bool chunkValid = GetChunkFeatures(
        indices, maximumFretInCandidateChunk,
        fretSpacingInCandidateChunk,fretCenterInCandidateChunk,
        stringPositions);
    
    if(previousChunk != nullptr)
    {
        bool previousChunkValid = true;
        
        vector<NotePositionEntry > previousChunkIndices = 
            previousChunk->GetCurrentNotePositionEntries();
        
        previousChunkValid = GetChunkFeatures(previousChunkIndices,
                maximumFretInPreviousChunk, fretSpacingInPreviousChunk,
                fretCenterInPreviousChunk, stringPositionsInPreviousChunk);
    
        if(previousChunkValid)
        {        
            vector<uint32_t> duplicateStrings;
            
            //If either fret center is 0, the fret center is not a good measure of 
            //how far the hand has to move between the two chunks
            if((fretCenterInCandidateChunk != 0) && 
               (fretCenterInPreviousChunk != 0))
            {
                candidateSpacingFromLastChunk = 
                    std::abs((int32_t)fretCenterInCandidateChunk - (int32_t)fretCenterInPreviousChunk);
            }
            
            std::set_intersection(stringPositionsInPreviousChunk.begin(),
                stringPositionsInPreviousChunk.end(),
                stringPositions.begin(),stringPositions.end(),
                std::back_inserter(duplicateStrings));
        }
    }
    
    //If the chunk is not valid, return the largest possible cost
    if(chunkValid)
    {
        LogStream << "CostTrace:" << Chunk::PrintNoteIndices(indices) << endl;
        chunkCost =  EvaluateConfigurationFeatures(
            maximumFretInCandidateChunk,fretSpacingInCandidateChunk, 
            candidateSpacingFromLastChunk,duplicateStrings);
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
        
        LogStream << Chunk::PrintNoteIndices(chunkIndices) << " FeaturesTrace:" << 
            "\r\n\tMax Fret:" << maximumFretInCandidateChunk << " " 
            "\r\n\tSpan:" <<fretSpacingInCandidateChunk << " " <<
            "\r\n\tFret Center:" << fretCenterInCandidateChunk << " = " << fretSumForAverage << "/" << numberOfFrettedNotes << endl;
            
        
    } //end if valid chunk
    
    return validChunk;
    
} //end GetChunkFeatures


uint32_t RotateVisitor::EvaluateConfigurationFeatures(
        uint32_t maximumFretInCandidateChunk,
        uint32_t fretSpacingInCandidateChunk,
        uint32_t distanceFromPreviousChunk,
        vector<uint32_t> stringIntersections)
{    
    const uint32_t interChunkSpacingCost = distanceFromPreviousChunk*InterChunkSpacingScalar;
    const uint32_t stringOverlapCost = stringIntersections.size() * StringOverlapScalar;
    
    const uint32_t maximumFretCost = maximumFretInCandidateChunk*MaximumFretScalar;
    const uint32_t fretSpanCost = fretSpacingInCandidateChunk*FretSpanScalar;
    
    const uint32_t candidateCost = maximumFretCost + fretSpanCost + 
                     interChunkSpacingCost + stringOverlapCost;
    
    LogStream << "CostTrace: $" << candidateCost << "\r\n\tMaxFret:" << maximumFretCost << "\r\n\tFretSpan:" << fretSpanCost \
                      << "\r\n\tAdjacency:" << interChunkSpacingCost << "\r\n\tIndependence:" \
                      << stringOverlapCost << endl;
    
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
    
    return duplicateStrings.size();
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
