#include "base.h"

//The iterators point to pointers, so they must be dereferenced twice essentially
/*
void Chunk::rotate()
{

	//for each note in a chunk, hold a root "string"
	
	//iterate through each note of the chunk.
	for(auto root_ix=Chunk_Notes.begin(); root_ix<Chunk_Notes.end(); root_ix++)
	{	
		//iterate through other notes, circular
		auto alt_ix = root_ix+1;
		while(alt_ix != root_ix)
		{
			alt_ix++;
			if(alt_ix == Chunk_Notes.end() ) //iterate circularly
				alt_ix = Chunk_Notes.begin();
			
			//for each possible fingering of this note, compare it to the root
			if((*root_ix)->compare(*alt_ix))
			{
				int pitch = (*root_ix)->get_pitch();
				for(int note_ix=0; note_ix < (*root_ix)->get_pitch_to_frets_entry_size(pitch); note_ix++)
				{
					(*alt_ix)->increment_note_index();
				}
			}
		}
	}
	//creates a different 
}
*/
///////////////////
// Compare function used by visitor. Shouldn't move anything around...
// just inspect the notes separation? This function might be unneeded/unwanted here


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
