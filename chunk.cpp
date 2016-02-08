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


void Chunk::empty_stack()
{
	cout << "emptying stack?" << endl;
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
		//cout << "comparing my stack" << endl;
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
	  	//open frets will not cause conflicts, so their frets do not need to be checked against other notes
	  	stack_copy.pop();//GOOD
	  }

	  else if(abs((n->get_fret() - current->get_fret())) <= 3)
	  {
	  	// check if the fret position for the candidate note would fit with the current portion of the note stack
	  	return GOOD;
	  }
	  else if(abs((n->get_fret() - current->get_fret())) > 3)
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
