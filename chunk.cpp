#include "base.h"
#define ALLOWABLE  3



void Chunk::force_chunk_note_indices(  ){
	if (_optima.size() == get_children_size()){
		for(int i=0; i<get_children_size(); i++)
			get_note_at(i)->set_note_index(_optima[i].first);
	}
}

void Chunk::empty_stack()
{
	while(!_comparison_stack.empty()){
		pop_stack();
	}
}

void Chunk::print_stack() {
//TODO: add more templates
	stack<Note*> stack_copy = _comparison_stack; 
	cout << "<";
	while(!stack_copy.empty()){
		int fretn = stack_copy.top()->get_fret();
		cout << fretn << ",";
		stack_copy.pop();
	}
	cout << ">" << endl;
}



int Chunk::compare_with_stack(Note* n){
// Return true if the note is addable. This method will copy the stack of notes 
    //return false if the note should perform a rotation to a new fret/string
	//bad(0) message: not compatible with chunk
    //discard(1) message: permanently remove note
    //good(2) message: this note works with the current chunk

	stack<Note*> stack_copy = _comparison_stack;

	while(!stack_copy.empty())
	{
	  Note* current = stack_copy.top();
	  
	  if(n->get_pitch() == current->get_pitch())
	  {
	  	//erase duplicate notes. This can happen after an octave shift
	  	return DISCARD;
	  }
	  if(n->get_string() == current->get_string())
	  {
	  	//Reshuffle for string overlaps
	  	return BAD;
	  }
	  /*
	  if((n->get_pitch() - current->get_pitch()) > 28) {
	  	if( !(n->get_fret() == 0 || current->get_fret() == 0))
		    current->decrement_octave();
	  }*/
	  
	  if(n->get_fret() == 0 || current->get_fret() == 0)
	  {
	  	//open frets will not cause conflicts
	  	stack_copy.pop();//GOOD
	  }

	  else if(abs((n->get_fret() - current->get_fret())) <= ALLOWABLE)
	  {
	  	// check if the fret position for the candidate note would fit with the current portion of the note stack
	  	//don't bother evaluating this for open strings
	  	return GOOD;
	  }
	  else if(abs((n->get_fret() - current->get_fret())) > ALLOWABLE)
	  {
	  	// check if the fret position for the candidate note would fit with the current portion of the note stack
	  	//don't bother evaluating this for open strings
	  	return BAD;
	  }
	  
	  
	  else
	  	stack_copy.pop();//GOOD
	  
	}
	return GOOD;
	
}





//The iterators point to pointers, so they must be dereferenced twice 
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
