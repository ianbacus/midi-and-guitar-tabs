#include "Visitor.h"

//This will recursively check and reconfigure all of the elements of the tree

//Possibly...
// - separate into a "stringopen" checker and a "fingerable" checker
// - both will use the same algorithm to check iterative from a root, to test all permutations of configurations


void RotateVisitor::visitBar(Bar* b) 
{
  for(int i=0; i < b->get_children_vector_size(); i++)
  {
    b->get_child(i)->accept(this);
  }
}

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
    tabstrings_reset(); //mark all strings as open except the current locus
    tabstrings_close(c->get_child(i)->get_string());
    
    for(int j=0; j < c->get_children_vector_size()-1; j++)
    {if(j != i){ //Loop over all of the non-i elements for comparison purposes. If they are valid, don't visit
        if(tabstrings_close(c->get_child(j)) && c->get_child(i)->compare(c->get_child(j))) 
        {
        	c->get_child(i)->accept(this);
        }
    }}  
  }
  
}


void RotateVisitor::visitNote() 
{
  
}
