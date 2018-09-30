#include "midi2melody.h"
#include "visitor.h"
#include "print_visitor.h"
#include "rotate_visitor.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>


void SortScoreByChunkFlexibility(vector<Chunk*>& sortedScore)
{
    sort(begin(sortedScore),end(sortedScore), [](const Chunk* left, const Chunk* right)
    {
        vector<Note*> leftNotes = left->GetElements();
        vector<Note*> rightNotes = right->GetElements();
        
        Note* leastMobileNoteLeft = *min_element(begin(leftNotes),end(leftNotes),
            [](const Note* lhs, const Note* rhs)
            {
                return lhs->GetProximityToNearestTuningBoundary() <
                       rhs->GetProximityToNearestTuningBoundary();
                        
            });
            
        Note* leastMobileNoteRight = *min_element(begin(rightNotes),end(rightNotes),
            [](const Note* lhs, const Note* rhs)
            {
                return lhs->GetProximityToNearestTuningBoundary() <
                       rhs->GetProximityToNearestTuningBoundary();
            });
            
        uint32_t leastMobileNotePlacementsLeft = leastMobileNoteLeft->GetProximityToNearestTuningBoundary();
        uint32_t leastMobileNotePlacementsRight = leastMobileNoteRight->GetProximityToNearestTuningBoundary();
        
        if(leastMobileNotePlacementsLeft == leastMobileNotePlacementsRight)
        {
            return left->GetNumberOfPositionPermutations() < right->GetNumberOfPositionPermutations();
        }
        
        else
        {
            return leastMobileNotePlacementsLeft < leastMobileNotePlacementsRight;
        }
    });
}

void FixInputs(
        const uint32_t maximumLength, 
        int32_t& noteOffset, 
        uint32_t& lowerBound, 
        uint32_t& upperBound)
{
    const int16_t transpositionLimit = 127;
    
    //Limit transpositions to +/- 127 pitches 
	if (noteOffset < -1*transpositionLimit) 
	{
		noteOffset = -1*transpositionLimit;
	}
    
	if (noteOffset > transpositionLimit)
	{
		noteOffset = transpositionLimit;
	}
    
	//Swap bounds if they inverted
	if (upperBound < lowerBound)
	{
		swap(lowerBound,upperBound);
	}
	
    //Clamp the bounds
    upperBound = min(upperBound, maximumLength);
    lowerBound = min(lowerBound, maximumLength);
}

uint32_t ProcessScore(
        string outputFile,
        const uint32_t lowerBound,
        const uint32_t upperBound, 
        vector<Chunk*>& score,
        TablatureOptimizer& tablatureRearranger,
        TablatureOutputFormatter& tablaturePrinter)

{
    uint32_t totalCost = 0;
    
    vector<Chunk*> sortedScore  = score;
    
    SortScoreByChunkFlexibility(sortedScore);
    //First pass: process and place chunks with fewer permutations before 
    //those with more permutations
    int i = 0;
    for (Chunk* currentChunk : sortedScore)
    {
        uint32_t measureIndex = currentChunk->GetMeasureIndex();
        
        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
            tablatureRearranger.OptimizeChunk(currentChunk);
        }
    }
  
    tablatureRearranger.EmitDebugString("First pass complete");

    //Second pass: reposition chunks in chronological order so that user's 
    //inter-chunk constraints can be satisfied
    for (Chunk* currentChunk : score)
    {
        uint32_t measureIndex = currentChunk->GetMeasureIndex();
        
        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
            if(measureIndex == upperBound)
            {
                currentChunk->SetIsMeasureEnd(true);
            }
            
            currentChunk->ResetAllNotesRepositions();
            currentChunk->SetIsOptimized(false);
            
            totalCost += tablatureRearranger.OptimizeChunk(currentChunk);
            tablaturePrinter.VisitChunk(currentChunk);
        }
        
    }
    
    tablaturePrinter.WriteTablatureToOutputFile(outputFile);
    tablaturePrinter.WriteTablatureToOutputFile("data/outTab.txt");
}

int ParseFileIntoTab(
        const string inputFile, 
        const string outputFile,
        int32_t noteOffset, 
        uint32_t lowerBound,
        uint32_t upperBound, 
        const uint32_t deltaExpansion) 
{
    
    uint32_t totalCost = 0;
    map<string,uint32_t> parsedConstants;
	
	parsedConstants["Frets"] = 10;
	parsedConstants["CapoFret"] = 0;
	parsedConstants["NeckPositionCost"] = 1500;
	parsedConstants["SpanCost"] = 3000;
	parsedConstants["NeckDiffCost"] = 7500;
	parsedConstants["SuppressedSustainCost"] = 1000;
	parsedConstants["ArpeggiationDeduction"] = 1000;
	parsedConstants["NumberOfLinesPerTabRow"] = 300;
    
    vector<Chunk*> score;
            
    TabberSettings tabSettings = {0};
    
    //Parse user settings file
    ParseTabberSettingsFile("src/tabberSettings.txt", parsedConstants, tabSettings);

    
    Note::InitializePitchToFretMap(
        tabSettings.InstrumentInfo.StringIndexedNoteNames,
        tabSettings.InstrumentInfo.StringIndexedMidiPitches, 
        parsedConstants["Frets"],
        parsedConstants["CapoFret"]);

    TablatureOptimizer tablatureRearranger(
        tabSettings.InstrumentInfo.StringIndexedNoteNames.size(),
        parsedConstants["NeckPositionCost"],
        parsedConstants["SpanCost"],
        parsedConstants["NeckDiffCost"],
        parsedConstants["SuppressedSustainCost"],
        parsedConstants["ArpeggiationDeduction"]);
    
    TablatureOutputFormatter tablaturePrinter(
        parsedConstants["NumberOfLinesPerTabRow"],
        tabSettings.InstrumentInfo.StringIndexedNoteNames);

    //Sort score by number of chunk permutations
	score = ParseIntermediateFile(
        inputFile,
        noteOffset,
        deltaExpansion,
        deltaExpansion);
    
    
    FixInputs(score.size(), noteOffset, lowerBound, upperBound);

    std::cout << outputFile << ": Optimizing and printing "  
            << (upperBound - lowerBound)+1
            << "measures, transposed " << noteOffset << " semi-tones: m"  
            << lowerBound << " to m" << upperBound << endl;
    
    
    totalCost = ProcessScore(
        outputFile,
        lowerBound, upperBound, score, 
        tablatureRearranger, tablaturePrinter);

    std::cout << "Done. " << std::endl;
    cout << "Total cost: $" << totalCost << endl;
    cout << Note::GetNotesLostCounterValue() << " notes were out of range" << endl;

  
    score.clear();

    return 0;
}

int main(int argc, char* argv[])
{
    const bool Debug = false;
    const uint8_t correctNumberOfArguments = 7;
    
    int returnValue = 0;
    
    if(Debug)
    {
        returnValue = ParseFileIntoTab(
            "data/parsed_midi_data.txt", 
            "data/tabs/outTab.txt", -12, 0, -1, -1);
    }

    if(argc != correctNumberOfArguments)
    {
        cout << "Invalid entry. use the following format:\n";
        cout << ">> ./gen <inputFile> <outputFile> <pitchShift#>,";
        cout << " <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
    }
    
    else
    {
        const string inputFile = argv[1]; //name of input file
        const string outputFile = argv[2];

        int noteOffset=atoi(argv[3]); 

        uint32_t lowerBound=atoi(argv[4]);
        uint32_t upperBound=atoi(argv[5]);

        unsigned int align = atoi(argv[6]);

        returnValue = ParseFileIntoTab(
            inputFile, 
            outputFile, 
            noteOffset, 
            lowerBound, 
            upperBound,
            align);   
    }
    
    return returnValue;
}
