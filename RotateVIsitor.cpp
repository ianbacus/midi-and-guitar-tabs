#include "RotateVisitor.h"
#define ALLOWABLE  4

//This will recursively check and reconfigure all of the elements of the tree

//This algorithm is terribly inefficient right now, it does a brute force check through permutations. I kept tweaking it until it worked.
//If chunk configurations were configured in terms of adjacent chunks, then analysis would be simpler. 

//Possibly...
// - separate into a "stringopen" checker and a "fingerable" checker
// - both will use the same algorithm to check iterative from a root, to test all permutations of configurations


void RotateVisitor::visitBar(Bar* b) 
{
//cout << "Bar" << endl;
  for(int i=0; i < b->get_children_size(); i++)
  {
  	recursion_lock=0;
  	clear_cache();

    b->get_child(i)->accept(this);
    _chunk_count++;
  }

}
  //Algorithm idea: 
  // Increment through vector of notes. If a candidate compares nicely with the current stack of accepted notes,
  // add it to the stack and continue to the next candidate. If it does not work with the rest of the note coordinates,
  // go back to the previous candidate and increment its index. The current best candidate is cached, and all future candidates
  // are compared with this candidate.

 //the items are essentially being moved from the vector tree into a stack for comparison purpose, element by element.
 // the counter_index allows the iterative testing of permutations. If the maximum allowable failure count is reached,
 // the current existing portion that was assumed to be correct must be wrong. Reversing the counter and incrementing
 // the note index at that point by one, and then checking all the future notes after that should fix this.


//Two levels of "failure":
// - fail count keeps track of the number of string/fret positions that the candidate note has tried to use.
//		it is reset when candidate notes are accepted
// - super fail is used to prevent re-checking the same set infinitely once all rotations have been attempted

void RotateVisitor::visitChunk(Chunk* c) 
{
  empty_stack();
  int counter_index=0,fail_count=0,super_fail=0;
//  push_stack(c->get_note_at(++j)); //only if this increments after evaluating

	  while(counter_index < c->get_children_size()){//cout << counter_index << "<" << c->get_children_size() << ", failcount = " << fail_count <<  endl;
	  	if(super_fail > pow(3,(c->get_children_size() ))  ) //cout << "rotation error: max rotation limit" << endl;
			break;
	  	if(fail_count == c->get_note_at(counter_index)->get_children_size()){
	  	//A note has only so many fret/string positions, indicated by "get_children_size()". after exhausting all tries, go back one step on the stack
	  		pop_stack();
	  		counter_index--;
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count=0;
	  		super_fail++;
	  	}
	  	else if(int test = compare_with_stack(c->get_note_at(counter_index))){
	  		switch(test){
				case DISCARD: //discard note: duplicate
					c->remove_note(c->get_note_at(counter_index));
					break;
				case GOOD: //if the note to be added is compatible with the rest of the stack, continue on to the next note
					push_stack(c->get_note_at(counter_index));
//cout << "good'ed:";print_stack();
					counter_index++;
					fail_count=0;
					break;
				case BAD:
					cout << "THIS WILL NEVER HAPPEN " << endl;		
					break;	  		
		  	};
		}
	  	else{ //should only be these three cases
	  		//if the note is incompatible, begin reconfiguring the candidate note
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count++;
	  	}
	  }

	  recursion_lock++;
	  
	  //cout << "break, r_lock = " << recursion_lock <<  endl;
	  if (recursion_lock < pow(3,(c->get_children_size() ))) {//c->get_children_size()){
	 	 compare_chunks(c->get_note_indices());
	 	 
			/*
			cout << "current optima["<< _optima.size() << "] <";

			for(int i=0;i<_optima.size();i++)
				cout << Note::get_fret_at(_optima[i].first, _optima[i].second) << ", ";
			cout << ">" << endl;
			*/
			
			for(int i=0;i< (c->get_children_size());i++){
				for(int i2=0;i2<3;i2++){
					if(!in_cache( c->get_note_indices() ) )
						break;
					c->get_note_at(i)->accept(this);
				}
			}//could this be done with a callback
			visitChunk(c);
	  }
	  else{
	  	 force_chunk_note_indices(_optima,c);
	  	 clear_cache();
	  	 empty_stack();
	  }
}
void RotateVisitor::empty_stack()
{
	while(!_comparison_stack.empty()){
		pop_stack();
	}
}

void RotateVisitor::print_stack() {
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

void RotateVisitor::compare_chunks(vector<pair <int, int> > current_indices) {
	//TODO: make this less sloppy and hacky
	//Test max fret spacing

	if (!in_cache(current_indices)){
		_cache.push_back(current_indices);
		if (_optima.size() == current_indices.size()){
			int spacing1=0, spacing2=0;
			int fret =0;
			int fmin =24, fmax1=0;
			//cout << "s1: " ;
			for(auto i : _optima){
				fret = Note::get_fret_at(i.first, i.second);
				//cout << fret << " ";
				if (fret > fmax1)
					fmax1 = fret;
				if (fret < fmin)
					fmin = fret;		
			}
			//cout << endl;
			spacing1 = fmax1-fmin;
			fmin =24;
			int fmax2=0;
			//cout << "s2: " ;
			for(auto i : current_indices){
				fret = Note::get_fret_at(i.first, i.second);
				//cout << fret << " ";
				if (fret > fmax2)
					fmax2 = fret;
				if (fret < fmin)
					fmin = fret;	
			}
			//cout << endl;
			spacing2=fmax2-fmin;
			//cout << "compared s1, s2: " << spacing1 << " " << spacing2 << endl;
			if (spacing1 < spacing2 && (spacing2 - spacing1) > 3 )//current optima is still better
				return;
			else if (fmax1 < fmax2)
				return;
		}
	
		_optima = current_indices;
	}
	
}


void RotateVisitor::force_chunk_note_indices( vector<pair<int, int> > indices, Chunk* c ){
	if (indices.size() == c->get_children_size()){
		for(int i=0; i<c->get_children_size(); i++)
			c->get_note_at(i)->set_note_index(indices[i].first);
	}
}


int RotateVisitor::compare_with_stack(Note* n){
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
	  	return DISCARD;

	  if(n->get_string() == current->get_string())//string overlaps
	  	return BAD;
	  
	  
	  /*
	  if((n->get_pitch() - current->get_pitch()) > 28) {
	  	if( !(n->get_fret() == 0 || current->get_fret() == 0))
		    current->decrement_octave();
		    cout << "niggers" << endl;
	  }*/
	  
	  if(n->get_fret() == 0 || current->get_fret() == 0)//open frets are fine
	  	stack_copy.pop();//GOOD

	  else if(abs((n->get_fret() - current->get_fret())) > ALLOWABLE){
	  // check if the fret position for the candidate note would fit with the current portion of the note stack
	  	return BAD;
	  }
	  
	  else
	  	stack_copy.pop();//GOOD
	  
	}
	return GOOD;
	
}

void RotateVisitor::visitNote(Note* n) 
{
//  cout << "incrementing note: " << n->get_string() << " " << n->get_fret() <<"to";
  n->increment_note_index();
//  cout <<  n->get_string() << " " << n->get_fret() << endl;
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
