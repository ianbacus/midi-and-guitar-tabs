#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "Visitor.h"
#include "base.h"
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
    bool strings_closed;
  public:
  	
  	PrintVisitor(){
		string_buffer.push_back(vector<string>(7));
		string_print_index=0;
		strings_closed=false;
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
    	for(string_print_index=6; string_print_index>= 0; string_print_index--){
			if(string_print_index==6) string_buffer.back()[string_print_index] += " ";
			else
				string_buffer.back()[string_print_index] += "|";
		}
    	string_buffer.push_back(vector<string>(7) );
        char tuning[] = {'d','a','d','g','b','?'};
	    for(string_print_index=6; string_print_index>= 0; string_print_index--){
			if(string_print_index==6) string_buffer.back()[string_print_index] += " ";
			else{
				string_buffer.back()[string_print_index] += "|";
				string_buffer.back()[string_print_index].push_back(tuning[(string_print_index)]);
			}
		}
	}
    void print_out(){
      
		ofstream ofile; //move to a constructor, make part of class attributes
		ofile.open("testoutput.txt");
		stringstream ss;
		for(std::vector< vector<string> >::iterator it = string_buffer.begin() ; it != string_buffer.end(); ++it) {
			for(int i=6; i>=0; i--){
			//  cout << string_buffer[i] << endl;
				ss << (*it)[i] << '\n';
			}
		}
		ofile << ss.rdbuf();
		ofile.close();

		}
};

#endif

