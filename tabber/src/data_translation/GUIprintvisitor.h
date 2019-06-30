

#ifndef __GUIPRINTVISITOR__
#define __GUIPRINTVISITOR__


#include "visitor.h"
#include "base.h"
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

  public:

    GUIPrintVisitor(void);
    virtual ~GUIPrintVisitor(void) {}
    void bar_ticks_reset();
    void bar_ticks_increment(int d);
    void newlines(void) ;


    virtual void VisitNote(Note*) override;
    virtual void VisitBar(Bar*) override;
    virtual void VisitChunk(Chunk*) override;
    void WriteTablatureToOutputFile(string fileName);

    void WriteTablatureToOutputString(string& outputString);
};

#endif
