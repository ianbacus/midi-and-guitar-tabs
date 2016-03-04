#ifndef __VISITOR__
#define __VISITOR__

#include <string>
#include <vector>
#include <map>
#include <stack>
class Note;
class Bar;
class Chunk;
using namespace std;

class Visitor
{

  public:
    virtual void visitNote(Note*)=0;
    virtual void visitBar(Bar*)=0;
    virtual void visitChunk(Chunk*)=0;
    
    
};


#endif
