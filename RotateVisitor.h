#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"


class RotateVisitor : public Visitor
{

  private:
    Stack<Note*> comparison_stack;
  public:
    virtual void visitNote();
    virtual void visitBar();
    virtual void visitChunk();

};

#endif
