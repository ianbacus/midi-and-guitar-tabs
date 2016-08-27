#include "midi2melody.h"
#include <iomanip>

#include <stdlib.h>
#include <cmath>
using namespace std;


//See midiTranslate for the order of initial translations

typedef unsigned char uchar;


vector<Bar*> score_maker(std::string infile, int shift,int align) {
	/* 
	  A beat overflow is calculated by multiplying the numerator of the time signature with
	  the mapped value of the denominator. This the denominator is mapped as 32/n, where n is the denominator.
	  
	  The time signature ratio is multiplied by 32 to count how many 32nd notes would fit in a bar.
	  
	  4/4 time: 32 32nd notes .... 128 128th notes
	     four quarter notes : 6 quarter triplets
	     sixteen sixteenth notes : 16*3/2 = 24 sixteenth triplets
	  6/8 time: 24 32nd notes
	  
	  This is because 32nd notes are used as the most precise bits (highest resolution).
	  
	  Triplets are 2/3 of their normal note value, so in the time that 2 eighth notes pass 3 triple-eighth notes pass
	  There are no reasonably low numbers with common multiples that are compatible with triplets
	  
	  all that matters is that the beginning and end notes are aligned properly... 
	  
	  
	  handles input in this form
  	 	 line 0: 72,100
		 line 1: 72,0
	*/
    std::ifstream file( infile );
    int last;
    
    map<int,int> beat_value = {{1,32},{2,16},{4,8},{8,4},{16,2},{32,1}};
    float bartime = 0;
    int counter = 0;
    float beat_overflow=36;
	vector<Bar*> score;
	
    score.push_back(new Bar());
    score.back()->add_chunk(new Chunk());
    std::string line;
    
    
	//align = align*(beat_overflow/(2*(beat_per_measure)));
	//bartime += align;
    while( std::getline( file, line ) ) 
    {
        std::istringstream iss( line );
		if(line == "SIGEVENT"){
			std::getline(file, line);
			std::istringstream iss( line );
			std::string num,denom;
			if( std::getline( iss, num , ',') && std::getline( iss, denom )) 
			{
				std::cout << "TS: " << num << "/" << denom << std::endl;
				//beat_overflow = (std::stof(num)*(beat_value[std::stoi(denom)]) );
				beat_overflow = (std::stof(num)/std::stof(denom)) * std::stof(denom) * 8; //(beat_value[std::stoi(denom)]) );
			}
		}	
        std::string p,d,tn;
		if( std::getline( iss, p , ',') && std::getline( iss, d, ',' ) && std::getline( iss, tn) ) 
		{
			//delimiting character inside each line
			counter++;
			int track_num = stoi(tn);
			float delta = stof(d)*pow(2.0,align); //multiply by a factor of 2: lengthen notes 
			int pitch = stoi(p);
			
			if(bartime >= beat_overflow && delta != 0)
			{
				//Case 1: the bar is full, create a new one with an empty initial chunk
				score.push_back(new Bar());	
			    score.back()->add_chunk(new Chunk());
				while(bartime >= beat_overflow) bartime -= beat_overflow;
				
			}
			bartime += abs(delta);
			pitch = (pitch<0) ? pitch : pitch-shift;
			
			if(delta == 0){
			  //Case 2: this note must be added to the current chunk in the current bar
			  //score.back() returns the last bar
			  //score.back()->get_children_size() returns the number of chunks, used to index the last chunk
			  //since the member vectors are inaccessible, using the "back()" function on the vector is not viable
			  
			  last = score.back()->get_children_size() - 1;
			  score.back()->get_child(last)->add_note(new Note(pitch,delta,track_num));
			}
			else{
			  //Case 3: a new chunk in the current bar is needed
			  
			  score.back()->add_chunk(new Chunk(delta));
			  last =  score.back()->get_children_size() - 1;
			  score.back()->get_child(last)->add_note(new Note(pitch,delta,track_num));
			}	

		}
	}
	return score;

}
