#ifndef __BAR__
#define __BAR__
#include "base.h"
#include "chunk.h"

class Bar : public Base
{
	private:
		vector<Chunk*> _bar_chunks;

	public:
		virtual ~Bar();
		
		void add_chunk(Chunk* c);
		void remove_chunk(Chunk* c);
		Chunk* get_child(int i);
		Chunk* get_child();
		
		virtual void accept(Visitor*v);
		virtual int get_children_size() const;
};

#endif
