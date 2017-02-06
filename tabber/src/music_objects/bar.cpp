#include "base.h"


Bar::~Bar()
{
	for (std::vector< Chunk* >::iterator it = _bar_chunks.begin() ; it != _bar_chunks.end(); ++it)
		delete (*it);
	_bar_chunks.clear();
}

/* 
 *	Push reference to a chunk into the current bar
 */
void Bar::add_chunk(Chunk* c) 
{
	_bar_chunks.push_back(c);
}

/* 
 *	Remove chunk from collection
 */
void Bar::remove_chunk(Chunk* c) 
{ 
	_bar_chunks.erase(std::find(_bar_chunks.begin(),_bar_chunks.end(),c)); 
}

/* 
 *	Get the ith chunk in the bar
 */
Chunk* Bar::get_child(int i) 
{
	return _bar_chunks[i];
}


/* 
 *	Get the last chunk by default
 */
Chunk* Bar::get_child() 
{
	return _bar_chunks.back();
}

/* 
 *	Dispatch visitor code
 */
void Bar::accept(Visitor*v) 
{
	v->visitBar(this);
}

/* 
 *	Get number of chunks in the current bar
 */
int Bar::get_children_size() const 
{
	return _bar_chunks.size();
}