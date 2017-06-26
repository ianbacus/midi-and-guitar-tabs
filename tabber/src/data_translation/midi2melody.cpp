//Translate an intermediate file into a c++ representation of the pitches and deltas

#include "midi2melody.h"
#include <iomanip>

#include <stdlib.h>
#include <cmath>
using namespace std;



typedef unsigned char uchar;


vector<Bar*> ParseIntermediateFile(std::string infile, int shift,int align) 
{
    std::ifstream file( infile );

	uint32_t ticksAccumulatedForCurrentMeasure = 0;
	uint32_t ticksPerMeasure = 36;
	vector<Bar*> parsedResult;
	
    parsedResult.push_back(new Bar());
    parsedResult.back()->add_chunk(new Chunk());
    
	std::string line;
    
    while( std::getline( file, line ) ) 
    {
       	const bool currentMeasureIsFull = (ticksAccumulatedForCurrentMeasure >= ticksPerMeasure); 
		std::istringstream pitchDeltaInputStringStream( line );

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

        std::string pitchString,deltaString,trackNumberString;

		if( std::getline( pitchDeltaInputStringStream, pitchString , ',') && 
			std::getline( pitchDeltaInputStringStream, deltaString, ',' ) && 
			std::getline( pitchDeltaInputStringStream, trackNumberString) ) 
		{
			const uint32_t currentTrackNumber = stoi(trackNumberString);
			const uint32_t delta = stoi(deltaString)*pow(2.0,align);
			int32_t pitch = stoi(pitchString);
			
			if(pitch<0)
			{
				std::cout << "negative pitch m2m" << std::endl;
				pitch -= shift;
			}	
			
			
			//The bar is full, create a new measure with an empty initial chunk
			//if((ticksAccumulatedForCurrentMeasure >= ticksPerMeasure) && (delta != 0))	
			if(currentMeasureIsFull && (delta > 0))
			{
				Bar *nextMeasure = new Bar();
				Chunk* initialChunk = new Chunk();

				nextMeasure->add_chunk(initialChunk);
				parsedResult.push_back(nextMeasure);

				//Find the remainder and carry it over to the next bar. Omit rests across
				// all voices that last more than one bar.  
				ticksAccumulatedForCurrentMeasure %= ticksPerMeasure;
			}
			
			//This note must be added to the current chunk in the current bar
			if(delta == 0)
			{
			 
				Bar* currentMeasure = parsedResult.back(); 
				uint32_t indexOfLastChunkInScore = currentMeasure->get_children_size() - 1;
				Chunk* currentChunk = currentMeasure->get_child(indexOfLastChunkInScore);

				currentChunk->add_note(new Note(pitch,delta,currentTrackNumber));

			}
			
			
			//A new chunk in the current bar is needed
			else
			{
			 	Bar* currentMeasure = parsedResult.back();
				Chunk* nextChunk = new Chunk(delta);

				nextChunk->add_note(new Note(pitch,delta,currentTrackNumber));
 				currentMeasure->add_chunk(nextChunk);
			}	

			ticksAccumulatedForCurrentMeasure += abs(delta);
			
		}
	}
	return parsedResult;

}
