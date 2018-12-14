
#include "base.h"
#include <string.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>

#ifndef __MIDI2MELODY__
#define __MIDI2MELODY__

using std::vector;
using std::string;

struct TabberSettings
{
    struct 
    {
        uint32_t SpanCost;
        uint32_t NeckPositionCost;
        uint32_t NeckDiffCost;
        uint32_t SuppressedSustainCost;
        
    } CostScalars;
    
    struct
    {
        uint32_t NumberOfFrets;
        uint32_t CapoFret;
        vector<uint16_t> StringIndexedMidiPitches;
        vector<std::string> StringIndexedNoteNames;
        
    } InstrumentInfo;
    
    struct 
    {
        uint32_t NumberOfLinesPerTabRow;
         
    } Formatting;
};

vector<Chunk*> ParseIntermediateFile(
    std::string infile, 
    int pitchOffset,
    int deltaExpansion,
    int durationExpansion);

vector<Chunk*> ParseIntermediateDataString(
    std::string& inputData,
    int pitchOffset,
    int deltaExpansion,
    int durationExpansion);

void ParseTabberSettingsFile(
    std::string infile, 
    std::map<std::string,uint32_t>& parsedConstants,
    TabberSettings& tabSettings);



#endif

