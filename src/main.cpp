
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

	if(argc != 7)
	{
		cout << "Invalid entry. use the following format:\n>> ./gen <inputFile> <outputFile> <pitchShift#>, <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
		return 0;
	}
	
	int measureIndex = 0;
	string inputFile = argv[1]; //name of input file
	string outputFile = argv[2];
	int noteOffset=24-atoi(argv[3]); //pitch shifts of 24 are common for the guitar
	int barset = atoi(argv[4]);
	int format_count = barset;
	int lowerBound=atoi(argv[5]);
	unsigned int upperBound=atoi(argv[6]);
	
	vector<Bar*> score = score_maker(inputFile,noteOffset,0);
	
	//Fix inputs
	if (noteOffset < -127) 
	{
		noteOffset = -127;
		std::cout << "Note shift exceeds bounds: set to -127" << std::endl;
	}
	if (noteOffset > 127)
	{
		noteOffset = 127;
		std::cout << "Note shift exceeds bounds: set to 127" << std::endl;
	}
	if (upperBound < lowerBound)
	{
		//swap in place
		upperBound ^= lowerBound;
		lowerBound ^= upperBound;
		upperBound ^= lowerBound;
	}
	if(( -1 == upperBound) || upperBound > score.size())	upperBound = score.size();
	
	std::cout << outputFile << std::endl;
	RotateVisitor* thefixer = new RotateVisitor();
	PrintVisitor* theprinter = new PrintVisitor(outputFile);
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
	theprinter->set_outfile("data/outTab.txt");
	theprinter->print_out();
	std::cout << "done. " << std::endl;
  
	delete thefixer;
	delete theprinter; 
	for (std::vector< Bar* >::iterator it = score.begin() ; it != score.end(); ++it)
		delete (*it);
  	
	return 0;
  
}


