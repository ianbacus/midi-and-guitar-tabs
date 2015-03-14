#ifndef __VISITOR__
#define __VISITOR__

#include <string>
#include <vector>
#include <map>
#include "Note.h"
#include "Bar.h"
#include "Chunk.h"
class Visitor
{

  public:
    virtual void visitNote(Note*)=0;
    virtual void visitBar(Bar*)=0;
    virtual void visitChunk(Chunk*)=0;
    
    
};


#endif
