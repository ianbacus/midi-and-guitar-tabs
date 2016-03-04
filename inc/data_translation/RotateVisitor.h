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
    
    std::mutex mtx_optima, mtx_cache;
    int _chunk_count;
  public:
  	virtual ~RotateVisitor() {
  		}
  	void print_chunk_count() { cout << _chunk_count; }
    
    void compare_chunks(Chunk *c,vector<pair<int, int> >);
    virtual void visitNote(Note*);
    virtual void visitBar(Bar*);
    
    virtual void visitChunk(Chunk*);
    bool in_cache(vector<pair<int, int> > input)
    {
    	//Print cache:
		//cout << "<";for(auto e : input)cout << Note::get_fret_at(e.first, e.second) << " ";cout << ">" << endl;
    	bool ret = false;
    	cout << "";
    	mtx_cache.lock();
    	for(auto entry : _cache){
    	// cout << "~<"; for(auto en : entry) cout<< Note::get_fret_at(en.first, en.second) << " "; cout << ">" << endl;
    		if(entry == input) 
    		{
    			ret = true;
    		}
    	}
    	mtx_cache.unlock();
    	return ret;
    }
    void clear_cache(){
    	//cout << "clear cache" << endl;
    	cout << "";
    	mtx_cache.lock();
    	_cache.clear();
    	mtx_cache.unlock();
    }

};

#endif
