#include "tabber.h"


void Bar::reconfigure(Base*)
{
	//possibly implement using visitor pattern, recursively search nonterminal nodes.. run a comparison on every set of 3 Chunks
	//every Note will have multiple locations. the goal of this algorithm is to make sure that no two Notes are more than 5 frets away, not counting 0 fret Notes
	
	//iterative solution for now
	for (auto Chunk_it = Bar_Chunks.begin() ; Chunk_it != Bar_Chunks.end()-2; ++Chunk_it)
	{
		if(!((*Chunk_it)->compare_chunks(*(Chunk_it+1) )) ) //return false for bad comparison
		{
//			rotate();
		}
		
	}
}
