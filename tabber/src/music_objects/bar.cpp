#include "base.h"


Bar::~Bar()
{
	for (std::vector< Chunk* >::iterator it = Chunks.begin() ; it != Chunks.end(); ++it)
		delete (*it);
	Chunks.clear();
}

/* 
 *	Push reference to a chunk into the current bar
 */
void Bar::PushBackElement(Chunk* c) 
{
	Chunks.push_back(c);
}

/* 
 *	Remove chunk from collection
 */
void Bar::RemoveElement(Chunk* c) 
{ 
	Chunks.erase(std::find(Chunks.begin(),Chunks.end(),c)); 
}

/* 
 *	Get the ith chunk in the bar
 */
Chunk* Bar::GetElementWithIndex(uint32_t i) const
{
	return Chunks[i];
}


/* 
 *	Get the last chunk by default
 */
Chunk* Bar::GetLastElement() const
{
	return Chunks.back();
}

vector<Chunk*> Bar::GetElements(void) const
{
    return Chunks;
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
uint32_t Bar::GetNumberOfElements() const 
{
	return Chunks.size();
}
