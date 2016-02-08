#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__

#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>

class RotateVisitor : public Visitor
{

  private:
    
    vector<pair<int, int> > _optima;
    vector< vector<pair<int, int> > > _cache;

    int _chunk_count;
    //int recursion_lock;
    
    std::mutex mtx_compstack, mtx_optima, mtx_cache;
    //int strings_occupied[6];
  public:
  	virtual ~RotateVisitor() {
  		}
  	void print_chunk_count() { cout << _chunk_count; }
    
    void compare_chunks(vector<pair<int, int> >);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    void force_chunk_note_indices( vector<pair<int, int> > , Chunk*  );
    virtual void visitChunk(Chunk*);
    bool in_cache(vector<pair<int, int> > input){
    	/*
		cout << "<";
		for(auto e : input)
			cout << Note::get_fret_at(e.first, e.second) << " ";
		cout << ">" << endl;
		*/
    	for(auto entry : _cache){
    		/*
    		cout << "~<";
    		for(auto en : entry)
				cout<< Note::get_fret_at(en.first, en.second) << " ";
			cout << ">" << endl;
			*/
    		if(entry == input) {
    			//cout << "false" << endl;
    			return true;
    		}
    	}
    //	cout << "true" << endl;
    	return false;
    }
    void clear_cache(){
    	_cache.clear();
    }

};
#endif
