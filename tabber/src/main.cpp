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

void GenerateTab(
        string outputFile, vector<Bar*> score, 
        uint32_t upperBound, uint32_t lowerBound)
{


}


int ParseFileIntoTab(const string inputFile, const string outputFile,
        int noteOffset, uint32_t lowerBound,
        uint32_t upperBound, const uint32_t align) 
{
        uint32_t measureIndex = 0;
    TabberSettings tabSettings;
    
    memset(&tabSettings, 0, sizeof(tabSettings));
    
    ParseTabberSettingsFile("src/tabberSettings.txt", tabSettings);
    
    Note::PitchToFretMap = Note::GeneratePitchToFretMap(
            tabSettings.InstrumentInfo.StringIndexedNoteNames,
            tabSettings.InstrumentInfo.StringIndexedMidiPitches, 
            tabSettings.InstrumentInfo.NumberOfFrets,
            tabSettings.InstrumentInfo.CapoFret);

    RotateVisitor TablatureRearranger(
            tabSettings.InstrumentInfo.StringIndexedNoteNames.size(),
            tabSettings.CostScalars.NeckPositionCost,
            tabSettings.CostScalars.SpanCost,
            tabSettings.CostScalars.NeckDiffCost,
            tabSettings.CostScalars.SuppressedSustainCost);
    
    PrintVisitor TablaturePrinter(
            tabSettings.Formatting.NumberOfLinesPerTabRow,
            tabSettings.InstrumentInfo.StringIndexedNoteNames);

    
	//Limit transpositions to +/- 127 pitches 
	if (noteOffset < -127) 
	{
		noteOffset = -127;
	}
    
	if (noteOffset > 127)
	{
		noteOffset = 127;
	}
    
	vector<Bar*> score= ParseIntermediateFile(inputFile,noteOffset,align);
    
	//Swap bounds if they are incorrect
	if (upperBound < lowerBound)
	{
		const uint32_t tempBound = lowerBound;
		lowerBound = upperBound;
		upperBound = tempBound;
	}
	
	//Set the default upper bound to the maximum value
    if(upperBound > score.size())
    {
        upperBound = score.size();
    }
    
    std::cout << outputFile << ": Optimizing and printing "  << (upperBound - lowerBound)+1
    << " measures: m"  << lowerBound << " to m" << upperBound << endl;

    //Iterate through each bar, recursively apply the visitor pattern to fix note positions
    for (Bar* currentBar : score)
    {
        //Todo: fix bug with one of the visitors that causes arithmetic exception 
        //when there is a discontinuity between d0 and a1 low strings for drop d
        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
            currentBar->DispatchVisitor(&TablatureRearranger);
            currentBar->DispatchVisitor(&TablaturePrinter);
        }
        
        measureIndex++;
    }

    TablaturePrinter.WriteTablatureToOutputFile(outputFile);
    TablaturePrinter.WriteTablatureToOutputFile("data/outTab.txt");

    std::cout << "Done. " << std::endl;
  
    score.clear();

	return 1;
}

int main(int argc, char* argv[])
{
    bool Debug = true;

    if(argc != 7)
    {
        cout << "Invalid entry. use the following format:\n";
        cout << ">> ./gen <inputFile> <outputFile> <pitchShift#>,";
        cout << " <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
        
        if(Debug)
        {
            return ParseFileIntoTab("data/parsed_midi_data.txt", 
                                    "data/tabs/outTab.txt", 0, 0, -1, -1);
        }

        return 0;
    }

    const string inputFile = argv[1]; //name of input file
    const string outputFile = argv[2];

    int noteOffset=atoi(argv[3]); 

    uint32_t lowerBound=atoi(argv[4]);
    uint32_t upperBound=atoi(argv[5]);

    const unsigned int align = atoi(argv[6]);

    return ParseFileIntoTab(inputFile, outputFile, noteOffset, 
                            lowerBound, upperBound, align);

    
}
