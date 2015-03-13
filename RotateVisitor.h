#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"


class RotateVisitor : public Visitor
{

  public:
    virtual void visitNote();
    virtual void visitBar();
    virtual void visitChunk();

};

#endif
