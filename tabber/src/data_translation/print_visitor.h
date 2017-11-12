#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "visitor.h"

#include "note.h"
#include "bar.h"
#include "chunk.h"

#include "tuning.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

class PrintVisitor : public Visitor
{
    private:
        vector< vector<string> > string_buffer;
        int string_print_index;
        int bar_ticks;
        int columnSet;
        int columnIndex;
        bool tripled;
        bool strings_closed;
        std::string outfile;
        void addSpaces(int &delta);

    public:

        PrintVisitor(string,int);
        virtual ~PrintVisitor(void) {}
        virtual void VisitNote(Note*);
        virtual void VisitBar(Bar*);
        virtual void VisitChunk(Chunk*);
        int get_columnSet(void);
        void overflowLines(void);
        void bar_ticks_reset();
        void bar_ticks_increment(int d);
        void newlines(bool) ;
        void print_out(void);
        void set_outfile(string of) { outfile = of; } 
};

#endif

