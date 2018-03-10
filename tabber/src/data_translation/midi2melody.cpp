//Translate an intermediate file into a c++ representation of the pitches and deltas

#include "midi2melody.h"
#include <iomanip>

#include <stdlib.h>
#include <cmath>
#include <assert.h>

using namespace std;

typedef unsigned char uchar;


vector<Bar*> ParseIntermediateFile(std::string infile, int shift,int align) 
{
    std::ifstream file( infile );

	uint32_t ticksAccumulatedForCurrentMeasure = 0;
	uint32_t ticksPerMeasure = 36;
	vector<Bar*> scoreTree;
	
    scoreTree.push_back(new Bar());
    
	std::string line;
    
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
            
			//The bar is full, create a new measure with an empty initial chunk	
			if(currentMeasureIsFull && (delta > 0))
			{
				Bar* const nextMeasure = new Bar();
				//Chunk* initialChunk = new Chunk();

				//nextMeasure->add_chunk(initialChunk);
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

				currentChunk->PushElement(new Note(pitch,delta,currentTrackNumber));
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
                    
                    vector<NotePositionEntry> currentNotePositionEntires = 
                      currentChunk->GetCurrentNotePositionEntries();

                    currentChunk->SetOptimalNotePositions(currentNotePositionEntires);
                }     
                
				Chunk* const nextChunk = new Chunk(delta);

				nextChunk->PushElement(new Note(pitch,delta,currentTrackNumber));
 				currentMeasure->PushBackElement(nextChunk);
			}	

			ticksAccumulatedForCurrentMeasure += abs(delta);
		}
	}
    
	return scoreTree;
}
