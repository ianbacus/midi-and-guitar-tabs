#include "base.h"


Bar::~Bar()
{
	for (std::vector< Chunk* >::iterator it = ChunkElements.begin() ; it != ChunkElements.end(); ++it)
		delete (*it);
	ChunkElements.clear();
}

/* 
 *	Push reference to a chunk into the current bar
 */
void Bar::PushBackElement(Chunk* c) 
{
	ChunkElements.push_back(c);
}

/* 
 *	Remove chunk from collection
 */
void Bar::RemoveElement(Chunk* c) 
{ 
	ChunkElements.erase(std::find(ChunkElements.begin(),ChunkElements.end(),c)); 
}

/* 
 *	Get the ith chunk in the bar
 */
Chunk* Bar::GetElementWithIndex(int i) 
{
	return ChunkElements[i];
}


/* 
 *	Get the last chunk by default
 */
Chunk* Bar::GetLastElement() 
{
	return ChunkElements.back();
}

/* 
 *	Dispatch Visitor code
 */
void Bar::DispatchVisitor(Visitor*v) 
{
	v->VisitBar(this);
}

/* 
 *	Get number of chunks in the current bar
 */
int Bar::GetNumberOfElements() const 
{
	return ChunkElements.size();
}
