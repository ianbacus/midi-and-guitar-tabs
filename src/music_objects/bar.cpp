#include "base.h"


Bar::~Bar()
{
	for (std::vector< Chunk* >::iterator it = _bar_chunks.begin() ; it != _bar_chunks.end(); ++it)
		delete (*it);
	_bar_chunks.clear();
}

void Bar::add_chunk(Chunk* c) 
{
	_bar_chunks.push_back(c);
}
void Bar::remove_chunk(Chunk* c) 
{ 
	_bar_chunks.erase(std::find(_bar_chunks.begin(),_bar_chunks.end(),c)); 
}

Chunk* Bar::get_child(int i) 
{
	return _bar_chunks[i];
}

Chunk* Bar::get_child() 
{
	return _bar_chunks.back();
}

void Bar::accept(Visitor*v) 
{
	v->visitBar(this);
}

int Bar::get_children_size() const 
{
	return _bar_chunks.size();
}