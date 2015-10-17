#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include "pthread.h"
#include <math.h>

class Comparison_Stack
{

private:
	stack<Note*> _comparison_stack;
public:
	void empty_stack();
    void pop_stack() { _comparison_stack.pop();}
    void push_stack(Note* n) {_comparison_stack.push(n); }
    void print_stack();
    int compare_with_stack(Note*);

};

class RotateVisitor : public Visitor
{

  private:
    //stack<Note*> _comparison_stack;
    vector<pair<int, int> > _optima;
    vector< vector<pair<int, int> > > _cache;
    
    int _chunk_count;
    int recursion_lock;
    //int strings_occupied[6];
  public:
  	virtual ~RotateVisitor() {
  		//empty_stack();
  		}
  	void print_chunk_count() { cout << _chunk_count; }
   
   /*
    void empty_stack();
    void pop_stack() { _comparison_stack.pop();}
    void push_stack(Note* n) {_comparison_stack.push(n); }
    void print_stack();
    int compare_with_stack(Note*);
   */
    void compare_chunks(vector<pair<int, int> >);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    void force_chunk_note_indices( vector<pair<int, int> > , Chunk*  );
    virtual void visitChunk(Chunk*);
    bool in_cache(vector<pair<int, int> > input){
    	for(auto entry : _cache){
    		//cout << "~<";for(auto en : entry)cout<< Note::get_fret_at(en.first, en.second) << " ";cout << ">" << endl;
    		if(entry == input) {
    			return true;
    		}
    	} return false;
    }
    void clear_cache(){
    	_cache.clear();
    }

};
enum {
BAD, DISCARD, GOOD,
};

#endif
