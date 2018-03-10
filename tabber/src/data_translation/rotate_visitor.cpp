#include "rotate_visitor.h"
#include <algorithm>
#include <assert.h>

//Go through all of the chunks in a given bar and reconfigure them
void RotateVisitor::VisitBar(Bar* bar) 
{
    for(uint32_t barIndex=0; barIndex < bar->GetNumberOfElements(); barIndex++)
    {
        Chunk* candidateChunk = bar->GetElementWithIndex(barIndex);
        candidateChunk->DispatchVisitor(this);
        
        PreviousChunk = candidateChunk;
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

    while(permutationIndex < permutationsForThisChunk)
    {
        permutationIndex++;

        //Reconfigure or "rotate" the chunk until it is in another configuration
        ReconfigureChunk(candidateChunk,noteConfigurationIndex);
        
        //Compare the current best chunk configuration against the reconfigured one
        SelectOptimalFingering(candidateChunk);	
    }

    candidateChunk->RepositionNotesToCurrentOptimalPositions();
    ResetMarkedChunks();

}

//Change the note positions in this chunk to a configuration that has not 
//been checked yet. Skip configurations with impossible fingerings. 
uint32_t RotateVisitor::ReconfigureChunk(Chunk* candidateChunk, uint32_t& noteConfigurationIndex)
{
    uint32_t octaveShiftCost = 0;
    bool stringsOverlap = true;
    bool currentNotePositionsAlreadyTried = true;
    bool allNotePositionsTried = false;
    
    do
    {
        allNotePositionsTried = RotateNoteOrItsParent(candidateChunk,
                noteConfigurationIndex,octaveShiftCost);
        
        vector<NotePositionEntry> currentNotePositionsEntries =
                candidateChunk->GetCurrentNotePositionEntries();
        
        stringsOverlap =
                (1 < CountStringIntersectionsBetweenTwoChunkConfigurations(currentNotePositionsEntries,
                candidateChunk->GetCurrentNotePositionEntries()));
        
        currentNotePositionsAlreadyTried = 
                WasChunkProcessed(candidateChunk->GetCurrentNotePositionEntries());

    } while(stringsOverlap && !allNotePositionsTried && currentNotePositionsAlreadyTried);
    
    return octaveShiftCost;
    
} //end ReconfigureChunk


//Pick between the current note positions and the current optimum note positions
//Apply the lower cost one 
void RotateVisitor::SelectOptimalFingering(
        Chunk *chunkToConfigure) 
{
    vector<NotePositionEntry> candidateChunkFingering =
        chunkToConfigure->GetCurrentNotePositionEntries();
        
    vector<NotePositionEntry> currentChunkFingering =
        chunkToConfigure->GetCurrentOptimalNotePositionEntries();
    
    const bool alreadyCheckedThisConfiguration = WasChunkProcessed(candidateChunkFingering);
    
    if(!alreadyCheckedThisConfiguration)
    {
        const uint32_t candidateCost = CalculateConfigurationCost(candidateChunkFingering);
        const uint32_t currentCost = CalculateConfigurationCost(currentChunkFingering);  
        
        if(candidateCost < currentCost)
        {
            chunkToConfigure->SetOptimalNotePositions(candidateChunkFingering);
            chunkToConfigure->RepositionNotesToCurrentOptimalPositions();
            
            vector<NotePositionEntry> newCurrentChunkFingering =
                chunkToConfigure->GetCurrentOptimalNotePositionEntries();
            
            assert(newCurrentChunkFingering == candidateChunkFingering);
            
            cout <<  Chunk::PrintNoteIndices(currentChunkFingering) << "@ $" << currentCost << 
                    " (current) vs " << Chunk::PrintNoteIndices(candidateChunkFingering) << "@ $" << candidateCost << endl;
        }
        
        MarkChunkAsProcessed(candidateChunkFingering);   
        
    }
} //end SelectOptimalFingering


bool RotateVisitor::RotateNoteOrItsParent(
    Chunk* candidateChunk, 
    uint32_t& noteIndex, 
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
        const uint32_t attemptedRepositions = note->get_current_note_index()+1;
        const uint32_t possibleRepositions = note->GetNumberOfElements();

        const bool repositionParent = attemptedRepositions >= possibleRepositions;
        
        baseCase = (noteIndex == 0) && repositionParent;
        
        if(!baseCase) //(noteIndex != 0) || !repositionParentf
        {
            uint32_t parentIndex = noteIndex-1;
            Note* const parentNote = candidateChunk->GetElementWithIndex(parentIndex);
            
            //TODO: try shifting the note up and down an octave
            //octaveShiftCost++;
            
            if((repositionParent) && (parentNote != nullptr))
            {
                baseCase = RotateNoteOrItsParent(candidateChunk, parentIndex, octaveShiftCost);
                noteIndex++;// = parentIndex;
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
        
    bool chunkValid = GetChunkFeatures(
                        indices,
                        maximumFretInCandidateChunk,
                        fretSpacingInCandidateChunk, 
                        fretCenterInCandidateChunk,
                        stringPositions);
    
    //If the chunk is not valid, return the largest possible cost
    if(chunkValid)
    {
        chunkCost =  EvaluateConfigurationFeatures(
                        maximumFretInCandidateChunk,
                        fretSpacingInCandidateChunk, 
                        fretCenterInCandidateChunk,
                        stringPositions);
    }
    
    return chunkCost;
    
} //end CalculateConfigurationCost


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
    uint32_t numberOfFrettedNotes = chunkIndices.size();
    uint32_t fretSumForAverage = 0;
    
    //Variables for finding the note spacing and maximum
	uint32_t minimumFretInChunkConfiguration = UINT32_MAX;
    
    const bool validChunk = (numberOfFrettedNotes != 0);
    
    if(validChunk)
    {
        for(NotePositionEntry notePositionEntry : chunkIndices)
        {
            const uint32_t notePositionIndex = notePositionEntry.first;
            const uint32_t notePitchMidiValue = notePositionEntry.second;

            const uint32_t currentFret = Note::get_fret_at(notePositionIndex,
                                                     notePitchMidiValue);
            
            const uint32_t currentString = Note::get_string_at(notePositionIndex,
                                                     notePitchMidiValue);
            
            stringPositions.push_back(currentString);

            //Don't count 0 in the fret spacing calculation
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
        
    } //end if valid chunk
    
    else
    {
        uint32_t numberOfFrettedNotes = chunkIndices.size();
        //Todo: some chunks are invalid still, this shouldn't be so
    }
    
    return validChunk;
} //end GetChunkFeatures


uint32_t RotateVisitor::EvaluateConfigurationFeatures(
        uint32_t maximumFretInCandidateChunk,
        uint32_t fretSpacingInCandidateChunk,
        uint32_t fretCenterInCandidateChunk,
        vector<uint32_t> stringPositions)
{
    
    
    uint32_t maximumFretCost = 0;
    uint32_t fretSpanCost = 0;
    uint32_t interChunkSpacingCost = 0;
    uint32_t stringOverlapCost = 0;
    
    Chunk * const previousChunk = PreviousChunk;
    
    uint32_t candidateCost = 0;

    //Increase the cost of this chunk if its note center is far from the 
    //previous chunk 
    if(previousChunk != nullptr)
    {
        bool previousChunkValid = true;
        
        vector<NotePositionEntry > previousChunkIndices = 
            PreviousChunk->GetCurrentNotePositionEntries();
        
        uint32_t maximumFretInPreviousChunk;
        uint32_t fretSpacingInPreviousChunk;
        uint32_t fretCenterInPreviousChunk;
        vector<uint32_t> stringPositionsInPreviousChunk;
        
        previousChunkValid = GetChunkFeatures(previousChunkIndices,
                                              maximumFretInPreviousChunk,
                                              fretSpacingInPreviousChunk,
                                              fretCenterInPreviousChunk,
                                              stringPositionsInPreviousChunk);
    
        if(previousChunkValid)
        {        
            vector<uint32_t> duplicateStrings;
            const uint32_t candidateSpacingFromLastChunk = 
                std::abs((int32_t)fretCenterInCandidateChunk - (int32_t)fretCenterInPreviousChunk);
            
            std::set_intersection(stringPositionsInPreviousChunk.begin(),
                                  stringPositionsInPreviousChunk.end(),
                                  stringPositions.begin(),
                                  stringPositions.end(),
                                  std::back_inserter(duplicateStrings));
            
            interChunkSpacingCost = candidateSpacingFromLastChunk*InterChunkSpacingScalar;
            stringOverlapCost = duplicateStrings.size() * StringOverlapScalar;
        }        
    }
    
    //Increase the cost as the maximum fret and fret spacing increases 
    maximumFretCost = maximumFretInCandidateChunk*MaximumFretScalar;
    fretSpanCost = fretSpacingInCandidateChunk*FretSpanScalar;
    
    candidateCost = maximumFretCost + fretSpanCost + 
                     interChunkSpacingCost + stringOverlapCost;
    
    cout << "Costs: " << "MaxFret:" << maximumFretCost << " FretSpan:" << fretSpanCost \
                      << " Adjacency:" << interChunkSpacingCost << " Independence:" \
                      << stringOverlapCost << endl;
    
    return candidateCost;
    
} //end EvaluateConfigurationFeatures


vector<uint32_t> RotateVisitor::GetStringPositionsOfIndices(
        vector<NotePositionEntry > chunkIndices)
{
    vector<uint32_t> stringPositions;
    
    for(NotePositionEntry notePositionEntry : chunkIndices)
    {
        const uint32_t notePositionIndex = notePositionEntry.first;
        const uint32_t notePitchMidiValue = notePositionEntry.second;
        const uint32_t currentString = Note::get_string_at(notePositionIndex,
                                                 notePitchMidiValue);

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
    noteToReposition->increment_note_index();
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
