
//#include "Reader.h"
//#include "MidiFile.h"
//#include "Options.h"
#include "midi2melody.h"
#include "Visitor.h"
#include "Printvisitor.h"
#include "RotateVisitor.h"

#include <thread>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>


/*

things to add still:

intra-chunk processing. 
	- the chunk rotating visitor could take into account the state of adjacent chunks, but this may sometimes require 
	arbitrary "recursive" stack permuting - a chunk may have two suitable states, but one would work better for the next chunk processed.
	The previous chunk with two states would not take this into account during its initial reconfiguration
	
	Melodies, defined by their track or (Specifically for baroque music) their distance from previous notes could be monitored so that
	configurations that break melodic coherency (ie during a subject entry in a fugue) would have a lower priority than "good" configurations
	with continuous melodic lines
	
	
*/


/*
void set_tuning(char setting)
{
	std::cout << tuning.size() << std::endl;

	if (setting == 'd')
	{
		tuning = {26, 33, 38, 43, 47, 52};
		ptuning = {'d','a','d','g','b','e'};
		SIZEOF_TUNING = 6;
	}
	else if (setting == 'b')
	{
		//BASS TAB
		tuning = {16, 21, 26, 31};
		ptuning = {'e','a','d','g'};
		SIZEOF_TUNING = 4;
	}
	else if (setting == '8')
	{
		//8STRING TAB
		tuning = {76, 81, 86, 91, 95, 100};
		ptuning = {'F','b','e','a','d','g','b','e'};
		SIZEOF_TUNING = 8;
	}
	else if (setting == 'g')
	{
		//goldberg variations tuning
		tuning = {26, 31, 38, 43, 47, 52};
		ptuning = {'d','g','d','g','b','e'};
		SIZEOF_TUNING = 6;
	}
}
*/

using namespace std;


void score_dispatch(RotateVisitor* thefixer, std::vector< Bar* > score, int start, int end)
{
 	int INDEX =0 ;
	for (std::vector< Bar* >::iterator it = score.begin() ; it < score.end(); it++,INDEX++)
	{
		if((start <= INDEX) && (INDEX <= end))
		{
			//cout << "bananas" << endl;
			(*it)->accept(thefixer);
		}
		//std::thread th(&Bar::accept,(*it), thefixer);
		//threads.push_back( std::thread(&Bar::accept,(*it), thefixer) );
	}	
}
 
 
int main(int argc, char* argv[]) {
/*

 This project transforms midi files into guitar tabs.
input:	./a <note_deltas.txt> <noteoffset=24> <bar_formatting=1> <lowerbound=0> <upperbound=-1>
argv: 	 0 	      1					2				  3				   4			  5

*/
	if(argc != 6){
		cout << "Invalid entry. use the following format:\n>> ./a note_deltas.txt, #noteoffset=20, #bar_formatting=1, #align=0" << endl;
		cout << "Note deltas is a text file with a simple format. One row is for note pitches, and one is for time deltas. See midi_writer.py for more info." << endl;
		cout <<"Note offset can be used to change the key of a song. It just offsets all of the notes by a set amount." << endl;
		cout <<"Bar formatting sets how many bars will show up per 'row' on the output text file with the tabs."<<endl;
		cout <<"Align sets how many eighth notes should appear before the first note." <<endl;
		return 0;
	}
	
	int INDEX = 0;
	string note_deltas = argv[1]; //name of input file
	int note_offset=24-atoi(argv[2]); //pitch shifts of 24 are so common 
	int barset = atoi(argv[3]);
	int format_count = barset;
	int inform_count = 0;
	int LOWERBOUND=atoi(argv[4]);
	unsigned int UPPERBOUND=atoi(argv[5]);
	
	
	//set_tuning(tuning);
	vector<Bar*> score = score_maker(note_deltas,note_offset,0);
	if (UPPERBOUND < LOWERBOUND)
	{
		//swap in place
		UPPERBOUND ^= LOWERBOUND;
		LOWERBOUND ^= UPPERBOUND;
		UPPERBOUND ^= LOWERBOUND;
	}
	if( -1 == UPPERBOUND)	UPPERBOUND = score.size();
	
	RotateVisitor* thefixer = new RotateVisitor();
	PrintVisitor* theprinter = new PrintVisitor();
	cout << "tabbing " << (UPPERBOUND - LOWERBOUND) <<  " measures from " << LOWERBOUND << " to " << UPPERBOUND << "..." << endl;
	
	
     vector<std::thread> threads;
     //Partition the score into four chunks (one per core), later update this to take into account the distribution of chunk sizes \
     across the entire score so that each thread has a separate area that contains a relatively even distribution of chunk sizes
     
     int score_sublength = score.size()/4; //round down
     vector<int> score_indices = {0, score_sublength, 2*score_sublength, 3*score_sublength, 4*score_sublength};
     for (auto e: score_indices)
     {
     	cout << e << endl;
     }
    // threads.push_back( std::thread(score_dispatch,thefixer,score, score_indices[0],score_indices[1]) );   
    // threads.push_back( std::thread(score_dispatch,thefixer,score, score_indices[4],score_indices[2]) );   

     /*  
     threads.push_back( std::thread(score_dispatch,thefixer,score, score_indices[1],score_indices[2]) );     
     threads.push_back( std::thread(score_dispatch,thefixer,score, score_indices[2],score_indices[3]) );     
     threads.push_back( std::thread(score_dispatch,thefixer,score, score_indices[3],score_indices[4]) );  
     */   
//     score_dispatch(thefixer,score, 0, score_indices[4]);
     
     
     
     std::cout << "Threads dispatched\n";
     for (auto& th : threads) th.join();
     std::cout << "Threads complete\n";

	INDEX=0;	
	for (std::vector< Bar* >::iterator it = score.begin() ; it < score.end(); it++,INDEX++)
	{
		
		if(format_count == barset){
		   theprinter->newlines();
		   format_count = 0;
		}
		if((LOWERBOUND <= INDEX) && (INDEX <= UPPERBOUND))
		{
			//cout << INDEX  << endl;
			(*it)->accept(thefixer);
			(*it)->accept(theprinter);
			format_count++;
		}
	}
	
	//cout << "printing ... tampered with " <<score[0]->get_child()->get_note_at(0)->get_noteslost() << " notes (octaved)." << endl;
	theprinter->print_out();
	cout << "done. " ;//<< //child(0)->get_note_at(0)->get_noteslost() << endl;
  
	delete thefixer;
	delete theprinter; 
	for (std::vector< Bar* >::iterator it = score.begin() ; it != score.end(); ++it)
		delete (*it);
  	
	return 0;
  
}


