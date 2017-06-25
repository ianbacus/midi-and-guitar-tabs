#include "base.h"
#include "chunk.h"


#ifndef __BAR__
#define __BAR__

class Bar : public Base
{
	private:
		vector<Chunk*> _bar_chunks;

	public:
		virtual ~Bar();
		
		void add_chunk(Chunk* c);
		void remove_chunk(Chunk* c);

		//retrieve a chunk from the bar by its index
		Chunk* get_child(int i);
		Chunk* get_child();
		
		virtual void accept(Visitor*v);
		virtual int get_children_size() const;
};

#endif
