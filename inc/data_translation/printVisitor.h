#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "Visitor.h"
#include "base.h"
#include "tuning.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
using namespace std;




class PrintVisitor : public Visitor
{
  private:
    vector< vector<string> > string_buffer;
    int string_print_index;
    int bar_ticks;
    bool tripled;
    bool strings_closed; //this is used to 
  public:
  	
  	PrintVisitor(){
		string_buffer.push_back(vector<string>(SIZEOF_TUNING+2));
		string_print_index=0;
		strings_closed=false;
		tripled = false;
  	}
  	virtual ~PrintVisitor() {
  		//delete[] string_buffer;
  	}
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
    void bar_ticks_reset() {bar_ticks = 0;}
    void bar_ticks_increment(int d) {bar_ticks+=d;}
    void newlines() { 
    	for(string_print_index=SIZEOF_TUNING+1; string_print_index>= 0; string_print_index--){
			if(string_print_index>=SIZEOF_TUNING) string_buffer.back()[string_print_index] += " ";
			else
				string_buffer.back()[string_print_index] += "|";
		}
    	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2) );
        
	    for(string_print_index=SIZEOF_TUNING+1; string_print_index>= 0; string_print_index--){
			if(string_print_index>=SIZEOF_TUNING) string_buffer.back()[string_print_index] += " ";
			else{
				string_buffer.back()[string_print_index] += "|";
				string_buffer.back()[string_print_index].push_back(ptuning[(string_print_index)]);
			}
		}
	}
    void print_out(){
      
		ofstream ofile; //move to a constructor, make part of class attributes
		ofile.open("testoutput.txt");
		stringstream ss;
		for(std::vector< vector<string> >::iterator it = string_buffer.begin() ; it != string_buffer.end(); ++it) {
			for(int i=SIZEOF_TUNING; i>=0; i--){
			//  cout << string_buffer[i] << endl;
				ss << (*it)[i] << '\n';
			}
		}
		ofile << ss.rdbuf();
		ofile.close();

		}
};

#endif

