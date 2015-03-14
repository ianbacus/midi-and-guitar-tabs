#include "Visitor.h"


class printVisitor : public Visitor
{
  
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
}


