#include "base.h"

///////////////////
// Compare function used by visitor. Shouldn't move anything around...
// just inspect the notes separation? This function might be unneeded/unwanted here

/*
bool Chunk::compare_chunks(Chunk* chunk2)
{
//only passed other chunks, polymorphism here used lazily
	vector<Note*> * chunk2_vector = chunk2->get_chunk_notes_vector();
	for(auto itx=Chunk_Notes.begin();itx!=Chunk_Notes.end();itx++)
	{
		for(auto ity=chunk2_vector->begin();ity!=chunk2_vector->end();ity++)
		{
			(*itx)->compare(*ity);
		}
	}

}
*/
