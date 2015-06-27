#ifndef __BAR__
#define __BAR__
#include "base.h"
#include "Chunk.h"

class Bar : public Base
//linked list implementation might make most sense here
{
	//not sure if this class will be used. 
	// - absorb this class into Chunk, treat that like a nonterminal node
	private:
		vector<Chunk*> _bar_chunks; //Chunks are children of Bar	

	public:
		virtual ~Bar() {
			for (std::vector< Chunk* >::iterator it = _bar_chunks.begin() ; it != _bar_chunks.end(); ++it)
  				delete (*it);
  			_bar_chunks.clear();
		}
		
		void add_chunk(Chunk* c) {_bar_chunks.push_back(c);}
		void remove_chunk(Chunk* c) { _bar_chunks.erase(std::find(_bar_chunks.begin(),_bar_chunks.end(),c)); }
		//operator [] for accessing Chunks
		//virtual void reconfigure();
		Chunk* get_child(int i) {return _bar_chunks[i];}
		
		virtual void accept(Visitor*v) {v->visitBar(this);}
		virtual int get_children_size() const {return _bar_chunks.size();}
};

#endif
