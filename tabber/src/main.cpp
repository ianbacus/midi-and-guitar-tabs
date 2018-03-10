#include "midi2melody.h"
#include "visitor.h"
#include "print_visitor.h"
#include "rotate_visitor.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>


class test1
{
public:
    vector<pair<int,int> > X;
    
    void setX(vector<pair<int,int> >  x)
    {
        X = x;
    }
    vector<pair<int,int> > getX(void)
    {
        return X;
    }
};

void test2(test1* qqq)
{
    vector<pair<int,int> > a = {{1,2},{1,3}};
    qqq->setX(a);
}

vector<pair<int,int> > test3(test1* qqq)
{
    return qqq->getX();
}

//    vector<pair<int,int> > a = {{1,2},{1,3}};
//    test1 mmm;
//    test2(&mmm);
//    vector<pair<int,int> > b = test3(&mmm);
//    
//    cout << endl;
//    for(auto i:a)
//        cout << "(" << i.first << "," << i.second << "),";
//     
//    cout << endl;
//    for(auto i:b)
//        cout << "(" << i.first << "," << i.second << "),";
//    
//    cout << endl;



void GenerateTab(
        string outputFile, vector<Bar*> score, 
        int upperBound, int lowerBound, 
        int format_count_initial)
{
    int measureIndex = 0;
    int format_count = format_count_initial;

    RotateVisitor* const TablatureRearranger = new RotateVisitor();
    PrintVisitor* const TablaturePrinter = new PrintVisitor(outputFile,80);

    std::cout << "done.\r\n tabbing " 
              << (upperBound - lowerBound)
              << " measures  from " 
              << lowerBound << " to " << upperBound << "...";

    //Iterate through each bar, recursively apply the visitor pattern to fix note positions
    for (std::vector< Bar* >::iterator it = score.begin() ; it < score.end(); it++,measureIndex++)
    {	
        if(format_count == format_count_initial)
        {
           TablaturePrinter->newlines((it==score.begin()));
           format_count = 0;
        }

        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
                (*it)->DispatchVisitor(TablatureRearranger);
                (*it)->DispatchVisitor(TablaturePrinter);
                format_count++;
        }
    }

    TablaturePrinter->print_out();
    TablaturePrinter->set_outfile("data/outTab.txt");
    TablaturePrinter->print_out();

    std::cout << "done. " << std::endl;

    delete TablatureRearranger;
    delete TablaturePrinter; 

}


int ParseFileIntoTab(const string inputFile, const string outputFile,
        int noteOffset, const int format_count_initial, int lowerBound,
        unsigned int upperBound, const unsigned int align) 
{
	std::cout << "scanning " << inputFile << "...";
	vector<Bar*> score = ParseIntermediateFile(inputFile,noteOffset,align);
	
	//Limit transpositions to +/- 127 pitches 
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

	//Swap bounds if they are incorrect
	if (upperBound < lowerBound)
	{
		const int tempBound = lowerBound;
		lowerBound = upperBound;
		upperBound = tempBound;
	}
	
	//Set the default upper bound to the maximum value
	if(upperBound > score.size())	
	{
		upperBound = score.size();
	}

	GenerateTab(outputFile, score, upperBound, lowerBound, format_count_initial);
  
	for (std::vector< Bar* >::iterator it = score.begin() ; it != score.end(); ++it)
	{
		delete (*it);
  	}

	return 0;
  
}

int main(int argc, char* argv[])
{
    bool Debug = false;
            
    if(Debug)
    {
        return ParseFileIntoTab("data/intermediates/test_for_tabber.txt", 
                                "data/tabs/outTab.txt", 12, 4, 0, -1, -1);
    }
    
    else
    {
        if(argc != 8)
        {
            cout << "Invalid entry. use the following format:\n";
            cout << ">> ./gen <inputFile> <outputFile> <pitchShift#>,";
            cout << " <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
            return 0;
        }

        const int pitchOffset = 24;

        const string inputFile = argv[1]; //name of input file
        const string outputFile = argv[2];

        int noteOffset=pitchOffset - atoi(argv[3]); 
        const int format_count_initial = atoi(argv[4]);

        int lowerBound=atoi(argv[5]);
        unsigned int upperBound=atoi(argv[6]);

        const unsigned int align = atoi(argv[7]);

        return ParseFileIntoTab(inputFile, outputFile, noteOffset, format_count_initial, lowerBound, upperBound, align);
    }
    
}
