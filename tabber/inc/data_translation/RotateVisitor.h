#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>



class RotateVisitor : public Visitor
{

  private:

    
    vector< vector<pair<int, int> > > _cache;
    vector<int> locked_strings;
    std::mutex mtx_optima, mtx_cache;
    int _chunk_count;
  public:
  	virtual ~RotateVisitor() {}
  	void print_chunk_count();
    
    void compare_chunks(Chunk *c,vector<pair<int, int> >);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    
    virtual void visitChunk(Chunk*);
    bool in_cache(vector<pair<int, int> > input);
    void clear_cache(void);

};

#endif