#ifndef __BAR__
#define __BAR__
#include "base.h"
#include "Chunk.h"

class Bar : public Base
//linked list implementation might make most sense here
{
	//not sure if this class will be used. 
	// - absorb this class into Chunk, treat that like a nonterminal node
	
	public:
		vector<Chunk*> _bar_chunks; //Chunks are children of Bar	

		void add_Chunk(Chunk*);
		//operator [] for accessing Chunks
		virtual void reconfigure();
		
		virtual void accept(Visitor*v) {v->visitBar(this);}
		virtual int get_children_size() {return _bar_chunks.size();}
};

#endif
