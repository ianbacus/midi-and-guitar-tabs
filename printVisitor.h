#include "Visitor.h"


class printVisitor : public Visitor
{
  private:
    vector<char> string_buffer[7];
  public:
  
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);
}


