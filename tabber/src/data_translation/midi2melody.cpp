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

vector<Chunk*> ParseIntermediateFile(std::string infile, int pitchOffset,int align) 
{
    ifstream file( infile );

    uint32_t measureIndex = 0;
    uint32_t chunkIndex = 0;
    
	uint32_t ticksAccumulatedForCurrentMeasure = 0;
	uint32_t ticksPerMeasure = 36;
	vector<Chunk*> scoreTree;
	
    
	string line;
    
    const int shift = 24 - pitchOffset;

    while( std::getline( file, line ) ) 
    {
       	const bool currentMeasureIsFull = (ticksAccumulatedForCurrentMeasure >= ticksPerMeasure); 
		istringstream pitchDeltaInputStringStream( line );
        string pitchString;
        string deltaString;
        string trackNumberString;
        string noteDurationString;
        
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
			std::getline( pitchDeltaInputStringStream, trackNumberString, ',' ) && 
			std::getline( pitchDeltaInputStringStream, noteDurationString) ) 
		{
			const uint32_t currentTrackNumber = stoi(trackNumberString);
			const uint32_t delta = stoi(deltaString)*pow(2.0,align);
			const uint32_t noteDuration = stoi(noteDurationString)*pow(2.0,align);
			const uint32_t pitch = stoi(pitchString)- shift;
            
            Note * const currentNote = new Note(pitch,noteDuration,currentTrackNumber);
            
            Chunk* const currentChunk = (scoreTree.size() > 0 ) ? scoreTree.back() : nullptr;
            
			
			//This note must be added to the current chunk in the current bar
			if((delta == 0) && (currentChunk != nullptr))
			{
				currentChunk->PushElement(currentNote);
			}
			
			//A new chunk in the current bar is needed
			else
			{                
				//Chunk* const nextChunk = new Chunk(delta,measureIndex);
				Chunk* const nextChunk = new Chunk(delta,chunkIndex);
                
                chunkIndex++;
                
				nextChunk->PushElement(currentNote);
 				scoreTree.push_back(nextChunk);
                
                if(currentChunk != nullptr)
                {
                    //Initialize the current optimal note positions to their default
                    //placements
                    vector<NotePositionEntry> currentNotePositionEntires = 
                      currentChunk->GetCurrentNotePositionEntries();
                    
                    currentChunk->SetOptimalNotePositions(currentNotePositionEntires);

                    currentChunk->SetNextChunk(nextChunk);
                    nextChunk->SetPreviousChunk(currentChunk);
                    
                    if(currentMeasureIsFull)
                    {
                        currentChunk->SetIsMeasureEnd(true);
                    }
                }
                
                //The bar is full, create a new measure with an empty initial chunk	
                if(currentMeasureIsFull)
                {
                    //Find the delta remainder and carry it over to the next bar. 
                    //Omit rests across all voices that last more than one bar.  
                    measureIndex++;
                    ticksAccumulatedForCurrentMeasure %= ticksPerMeasure;
                }
                
			}	

			ticksAccumulatedForCurrentMeasure += abs(delta);
		}
	}
    
    scoreTree.back()->SetIsMeasureEnd(true);
    
	return scoreTree;
}
