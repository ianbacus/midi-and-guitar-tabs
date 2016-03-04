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

#define OUTFILE "data/outputTab.txt"

class PrintVisitor : public Visitor
{
  private:
    vector< vector<string> > string_buffer;
    int string_print_index;
    int bar_ticks;
    bool tripled;
    bool strings_closed; //this is used to 
  public:
  	
  	PrintVisitor(void);
  	virtual ~PrintVisitor(void) {}
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
    void bar_ticks_reset();
    void bar_ticks_increment(int d);
    void newlines(void) ;
    void print_out(void);
};

#endif
