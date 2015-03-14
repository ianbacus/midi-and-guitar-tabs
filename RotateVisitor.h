#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"


class RotateVisitor : public Visitor
{

  private:
    Stack<Note*> _comparison_stack;
    int strings_occupied[6];
  public:
  
    void pop_stack() {_comparison_stack.pop();}
    void push_stack(Note* n) {_comparison_stack.push_back(n);}
    bool compare_with_stack(Note*);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);

};

#endif
