#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__
#include "Visitor.h"
#include "base.h"

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
      for(int i=6; i>=0; i--)
      {
        cout << string_buffer[i] << endl;
      }
    }
};

#endif

