//Translate an intermediate file into a c++ representation of the pitches and deltas

#include "midi2melody.h"
#include <iomanip>

#include <stdlib.h>
#include <cmath>
#include <assert.h>
#include <regex>
#include <map>

using namespace std;


void ParseTabberSettingsFile(std::string infile, TabberSettings& tabSettings)
{
    const regex keyValueRegex("\\s*([a-zA-Z]+?)\\s*?:\\s*?([0-9]+)");
    const regex tuningRegex("\\s*Tuning\\s*?:\\s*?([a-gA-G][\\-0-9][,\\s]*)+");
    const regex tuningRegex2("([a-gA-G])([0-9])[,\\s]*");
    
    ifstream file( infile );
    
    string line;
    
    map<string,uint32_t> parsedConstants;
    
    while(getline( file, line ) ) 
    {
        smatch keyValMatchResults;
        smatch tuningMatchResults;
        
        regex_match(line, keyValMatchResults, keyValueRegex);
        regex_match(line, tuningMatchResults, tuningRegex);
        
        
        if(keyValMatchResults.size() > 0)
        {
            string key = keyValMatchResults[1];
            uint32_t value = stoi(keyValMatchResults[2]);
            
            parsedConstants[key] = value;
        }
        
        if(tuningMatchResults.size() > 0) 
        {
            //Note length indications to print above the notes

            map<string,uint32_t> noteAlphabetToPitchMidiValue = 
            {
                {"c",12},{"d",14},{"e",16},{"f",17},{"g",19},{"a",21},{"b",23},
            };
            
            vector<uint16_t> midiPitches;
            vector<string> midiPitchStrings;
            
            while(regex_search(line, tuningMatchResults, tuningRegex2, regex_constants::match_any))
            {
                const uint32_t octavePitchMidiValue = 12;
                
                const string stringName = tuningMatchResults[1];
                const uint32_t stringOctave = stoi(tuningMatchResults[2]);
                const uint32_t octaveOffset = octavePitchMidiValue*stringOctave;
                
                uint32_t pitchMidiValue = noteAlphabetToPitchMidiValue[stringName]+octaveOffset;
                
                midiPitches.push_back(pitchMidiValue);
                midiPitchStrings.push_back(stringName);
                
                line = tuningMatchResults.suffix();
            }

            tabSettings.InstrumentInfo.StringIndexedMidiPitches = midiPitches;
            tabSettings.InstrumentInfo.StringIndexedNoteNames = midiPitchStrings;
        }
        
       
        tabSettings.InstrumentInfo.NumberOfFrets = parsedConstants["Frets"];
        tabSettings.InstrumentInfo.CapoFret = parsedConstants["CapoFret"];
        
        
        tabSettings.CostScalars.SpanCost = parsedConstants["SpanCost"];
        tabSettings.CostScalars.NeckPositionCost = parsedConstants["NeckPositionCost"];
        tabSettings.CostScalars.NeckDiffCost = parsedConstants["NeckDiffCost"];
        tabSettings.CostScalars.SuppressedSustainCost = parsedConstants["SuppressedSustainCost"];
        
        tabSettings.Formatting.NumberOfLinesPerTabRow = parsedConstants["NumberOfLinesPerTabRow"];
    }
}

vector<Bar*> ParseIntermediateFile(std::string infile, int pitchOffset,int align) 
{
    std::ifstream file( infile );

	uint32_t ticksAccumulatedForCurrentMeasure = 0;
	uint32_t ticksPerMeasure = 36;
	vector<Bar*> scoreTree;
	
    scoreTree.push_back(new Bar());
    
	std::string line;
    
    const int shift = 24 - pitchOffset;

    while( std::getline( file, line ) ) 
    {
       	const bool currentMeasureIsFull = (ticksAccumulatedForCurrentMeasure >= ticksPerMeasure); 
		std::istringstream pitchDeltaInputStringStream( line );
        std::string pitchString,deltaString,trackNumberString;
        
        //Case 0: Time signature event
		if(line == "SIGEVENT")
		{
			std::getline(file, line);
			std::istringstream pitchDeltaInputStringStream( line );
			std::string beatsPerBarString, beatUnitString; 

			if( std::getline( pitchDeltaInputStringStream, beatsPerBarString, ',') && 
				std::getline( pitchDeltaInputStringStream, beatUnitString )) 
			{
				const float beatsPerBar = std::stof(beatsPerBarString);
				const float beatUnit = std::stof(beatUnitString);
				const float meterRatio = (beatsPerBar/beatUnit);

				ticksPerMeasure = meterRatio * beatUnit * 8;
			}
		}

        //Case 1: pitch event
		if( std::getline( pitchDeltaInputStringStream, pitchString , ',') && 
			std::getline( pitchDeltaInputStringStream, deltaString, ',' ) && 
			std::getline( pitchDeltaInputStringStream, trackNumberString) ) 
		{
			const uint32_t currentTrackNumber = stoi(trackNumberString);
			const uint32_t delta = stoi(deltaString)*pow(2.0,align);
			const uint32_t pitch = stoi(pitchString)- shift;
            
            Note * const currentNote = new Note(pitch,delta,currentTrackNumber);
            
			//The bar is full, create a new measure with an empty initial chunk	
			if(currentMeasureIsFull && (delta > 0))
			{
				Bar* const nextMeasure = new Bar();
                
				scoreTree.push_back(nextMeasure);

				//Find the delta remainder and carry it over to the next bar. 
                //Omit rests across all voices that last more than one bar.  
				ticksAccumulatedForCurrentMeasure %= ticksPerMeasure;
			}
			
			//This note must be added to the current chunk in the current bar
			if(delta == 0)
			{
				Bar* const currentMeasure = scoreTree.back(); 
                Chunk* const currentChunk = currentMeasure->GetLastElement();

				currentChunk->PushElement(currentNote);
			}
			
			//A new chunk in the current bar is needed
			else
			{
			 	Bar* const currentMeasure = scoreTree.back();
                
                //Initialize the current optimal note positions to their default
                //placements
                if(currentMeasure->GetNumberOfElements() > 0)
                {
                    Chunk* const currentChunk = currentMeasure->GetLastElement();
                    
                    //currentChunk->CleanChunk();
                    
                    vector<NotePositionEntry> currentNotePositionEntires = 
                      currentChunk->GetCurrentNotePositionEntries();

                    currentChunk->SetOptimalNotePositions(currentNotePositionEntires);
                }     
                
				Chunk* const nextChunk = new Chunk(delta);

				nextChunk->PushElement(currentNote);
 				currentMeasure->PushBackElement(nextChunk);
			}	

			ticksAccumulatedForCurrentMeasure += abs(delta);
		}
	}
    
	return scoreTree;
}
