#ifndef __VISITOR__
#define __VISITOR__

#include <string>
#include <vector>
#include <map>


class Visitor
{

  public:
    virtual void visitNote()=0;
    virtual void visitBar()=0;
    virtual void visitChunk()=0;
    
    
};


#endif
