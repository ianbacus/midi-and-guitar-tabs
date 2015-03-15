#include "RotateVisitor.h"
#define MAX  8
//This will recursively check and reconfigure all of the elements of the tree

//Possibly...
// - separate into a "stringopen" checker and a "fingerable" checker
// - both will use the same algorithm to check iterative from a root, to test all permutations of configurations


void RotateVisitor::visitBar(Bar* b) 
{
cout << "Bar" << endl;
  for(int i=0; i < b->get_children_size(); i++)
  {
    b->get_child(i)->accept(this);
  }
}
/*
void Chunk::rotate()
{

	//for each note in a chunk, hold a root "string." Determine if the other notes in the chunk can be tabbed 
	// without overlapping with this string. If not, go through the children nodes and fix them
	
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
void RotateVisitor::visitChunk(Chunk* c) 
{

  //hold a "locus" note in the chunk and compares it against all other notes. If no configuration works, tries next note as locus
  for(int i=0; i < c->get_children_vector_size()-1; i++)
  {
//    tabstrings_reset(); //mark all strings as open except the current locus
//    tabstrings_close(c->get_child(i)->get_string());
    
    for(int j=0; j < c->get_children_vector_size()-1; j++)
    {if(j != i){ //Loop over all of the non-i elements for comparison purposes. If they are valid, don't visit
    // but... call a function from them to recurse back to. (?)
        if(tabstrings_close(c->get_child(j)) && c->get_child(i)->compare(c->get_child(j))) 
        {
        	push_stack(c);
        	c->get_child(i)->accept(this);
        }
    }}  
  }
  */
  //Algorithm idea: 
  // Increment through vector of notes. If a candidate compares nicely with the current stack of accepted notes,
  // add it to the stack and continue to the next candidate. If it does not work with the rest of the note coordinates,
  // go back to the previous candidate and increment its index
 
 //the items are essentially being moved from the vector tree into a stack for comparison purpose, element by element
void RotateVisitor::visitChunk(Chunk* c) 
{
	cout << "chunk" << endl;
  int counter_index=0,fail_count=0;
//  push_stack(c->get_note_at(++j)); //only if this increments after evaluating

	  while(counter_index < c->get_children_size())
	  {
	  	if(fail_count == c->get_note_at(counter_index)->get_children_size())
	  	{
	  		//after exhausting all tries, go back one step on the stack
	  		pop_stack();
	  		counter_index--;
	  		c->get_note_at(counter_index)->accept(this);
	  	}
	  	else if(compare_with_stack(c->get_note_at(counter_index)) )
	  	{
	  		//if the note to be added is compatible with the rest of the stack, continue on to the next note
	  		push_stack(c->get_note_at(counter_index));
	  		counter_index++;
	  		fail_count=0;
	  		
	  	}
	  	else //should only be these three cases
	  	{
	  		//if the note is incompatible, begin reconfiguring the candidate note
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count++;
	  	}
	  }
}

bool RotateVisitor::compare_with_stack(Note* n)
{
	stack<Note*> stack_copy = _comparison_stack;

	while(!stack_copy.empty())
	{
	  Note* current = stack_copy.top();
	  //check if the string is available
//	  cout << "COPIED TOP ITEM" << endl;
	  cout << "candidate fret:" << n->get_fret();
	cout <<  " candidate string:" << n->get_string() << endl;
	cout << "stack fret:" << current->get_fret();
	cout << " stack string:" << current->get_string() << endl;
	  	//continue;
	  if(n->get_string() == current->get_string()){
	  	cout << "OVERLAP" << endl;
	  	return false;
	  }
	  else if(n->get_fret() == 0 || current->get_fret() == 0) {
	  	cout << "FRETTED ZERO, NONOVERLAPPING" << endl;
	  	stack_copy.pop();
	  }
	  else if(abs((n->get_fret() - current->get_fret())) > 10) {
	  	cout << "SPACED " << MAX << " ITEMS AWAY" << endl;
	  	return false;
	  }
	  else{
	  	cout << "ELSE: VALID" << endl;
	  	stack_copy.pop();
	  }
	  
	}
	//if the candidate does not violate string overlap or fret over-extension, it is accepted
	return true;
	
}

void RotateVisitor::visitNote(Note* n) 
{
cout << "incrementing note" << endl;
  n->increment_note_index();
}
