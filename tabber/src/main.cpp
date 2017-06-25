
//#include "Reader.h"
//#include "MidiFile.h"
//#include "Options.h"
#include "midi2melody.h"
#include "visitor.h"
#include "print_visitor.h"
#include "rotate_visitor.h"

#include <thread>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>


 
 
int main(int argc, char* argv[]) 
{

	if(argc != 8)
	{
		cout << "Invalid entry. use the following format:\n";
		cout << ">> ./gen <inputFile> <outputFile> <pitchShift#>,";
		cout << " <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
		return 0;
	}
	
	int measureIndex = 0;

	const string inputFile = argv[1]; //name of input file
	const string outputFile = argv[2];

	int noteOffset=24-atoi(argv[3]); //pitch shifts of 24 are common for the guitar
	const int format_count_initial = atoi(argv[4]);
	int format_count = format_count_initial;
	
	int lowerBound=atoi(argv[5]);
	unsigned int upperBound=atoi(argv[6]);
	const unsigned int align = atoi(argv[7]);
	std::cout << "scanning " << inputFile << "...";
	vector<Bar*> score = score_maker(inputFile,noteOffset,align);
	
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
		//swap
		int tempBound = lowerBound;
		lowerBound = upperBound;
		upperBound = tempBound;
	}
	if(( -1 == upperBound) || upperBound > score.size())	
		upperBound = score.size();
	
	std::cout << "done." << std::endl;
	RotateVisitor* thefixer = new RotateVisitor();
	PrintVisitor* theprinter = new PrintVisitor(outputFile,80);
	cout << "tabbing " << (upperBound - lowerBound) <<  " measures";
	cout << " from " << lowerBound << " to " << upperBound << "..." << endl;
	

	//Iterate through each bar, recursively apply the visitor pattern to fix note positions
	for (std::vector< Bar* >::iterator it = score.begin() ; it < score.end(); it++,measureIndex++)
	{	
		if(format_count == format_count_initial){
		   theprinter->newlines((it==score.begin()));
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


