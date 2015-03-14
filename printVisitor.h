#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "Visitor.h"


class printVisitor : public Visitor
{
  private:
    string string_buffer[7];
    int string_print_index;
  public:
  
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
};

#endif

