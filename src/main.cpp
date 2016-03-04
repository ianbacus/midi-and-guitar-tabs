
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

 
 
int main(int argc, char* argv[]) 
{
	/*

	 This project transforms midi files into guitar tabs.
	input:	./a <note_deltas.txt> <noteoffset=24> <bar_formatting=1> <lowerbound=0> <upperbound=-1>
	argv: 	 0 	      1					2				  3				   4			  5

	*/
	if(argc != 6)
	{
		cout << "Invalid entry. use the following format:\n>> ./a note_deltas.txt, #noteoffset=20, #bar_formatting=1, #align=0" << endl;
		cout << "Note deltas is a text file with a simple format. One row is for note pitches, and one is for time deltas. See midi_writer.py for more info." << endl;
		cout <<"Note offset can be used to change the key of a song. It just offsets all of the notes by a set amount." << endl;
		cout <<"Bar formatting sets how many bars will show up per 'row' on the output text file with the tabs."<<endl;
		cout <<"Align sets how many eighth notes should appear before the first note." <<endl;
		return 0;
	}
	
	int measureIndex = 0;
	string note_deltas = argv[1]; //name of input file
	int note_offset=24-atoi(argv[2]); //pitch shifts of 24 are so common 
	int barset = atoi(argv[3]);
	int format_count = barset;
	int inform_count = 0;
	int lowerBound=atoi(argv[4]);
	unsigned int upperBound=atoi(argv[5]);
	
	vector<Bar*> score = score_maker(note_deltas,note_offset,0);
	if (upperBound < lowerBound)
	{
		//swap in place
		upperBound ^= lowerBound;
		lowerBound ^= upperBound;
		upperBound ^= lowerBound;
	}
	if( -1 == upperBound)	upperBound = score.size();
	
	RotateVisitor* thefixer = new RotateVisitor();
	PrintVisitor* theprinter = new PrintVisitor();
	cout << "tabbing " << (upperBound - lowerBound) <<  " measures from " << lowerBound << " to " << upperBound << "..." << endl;
	
	measureIndex=0;	
	for (std::vector< Bar* >::iterator it = score.begin() ; it < score.end(); it++,measureIndex++)
	{
		
		if(format_count == barset){
		   theprinter->newlines();
		   format_count = 0;
		}
		if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
		{
			(*it)->accept(thefixer);
			(*it)->accept(theprinter);
			format_count++;
		}
	}
	theprinter->print_out();
	std::cout << "done. " << std::endl;
  
	delete thefixer;
	delete theprinter; 
	for (std::vector< Bar* >::iterator it = score.begin() ; it != score.end(); ++it)
		delete (*it);
  	
	return 0;
  
}


