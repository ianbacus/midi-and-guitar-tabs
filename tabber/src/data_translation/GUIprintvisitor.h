

#ifndef __GUIPRINTVISITOR__
#define __GUIPRINTVISITOR__


#include "visitor.h"
#include "base.h"
#include "tuning.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
using namespace std;

class GUIPrintVisitor : public Visitor
{
  private:
    vector< vector<string> > string_buffer;
    int string_print_index;
    int bar_ticks;
    bool tripled;
    bool strings_closed;
    std::string outfile;
    
  public:
  	
  	PrintVisitor(string);
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

