#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "Visitor.h"
#include "base.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

class PrintVisitor : public Visitor
{
  private:
    string string_buffer[7];
    int string_print_index;
  public:
  
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
    void print_out()
    {
      
	    ofstream ofile; //move to a constructor, make part of class attributes
	    ofile.open("testoutput.txt");
	    stringstream ss;
      for(int i=6; i>=0; i--)
      {
//        cout << string_buffer[i] << endl;
        ss << string_buffer[i];
      }
      ofile << ss.rdbuf();
		    ofile.close();
		    
    }
};

#endif

