#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"

class RotateVisitor : public Visitor
{

  private:
    stack<Note*> _comparison_stack;
    int _chunk_count;
    //int strings_occupied[6];
  public:
  	virtual ~RotateVisitor() {
  		empty_stack();
  		}
  	void print_chunk_count() { cout << _chunk_count; }
    void empty_stack();
    void pop_stack() { _comparison_stack.pop();}
    void push_stack(Note* n) {_comparison_stack.push(n); }
    int compare_with_stack(Note*);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    virtual void visitChunk(Chunk*);

};

#endif
