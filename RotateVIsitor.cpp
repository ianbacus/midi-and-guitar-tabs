#include "RotateVisitor.h"

//This will recursively check and reconfigure all of the elements of the tree

//This algorithm is terribly inefficient right now, it does a brute force check through permutations
//If chunk configurations were configured in terms of adjacent chunks, then analysis would be simpler. 

//Possibly...
// - separate into a "stringopen" checker and a "fingerable" checker
// - both will use the same algorithm to check iterative from a root, to test all permutations of configurations


void RotateVisitor::visitBar(Bar* b) 
{
//clear_cache();
  //Go through all of the chunks in a given bar and descend down into them
  for(int i=0; i < b->get_children_size(); i++)
  {
    b->get_child(i)->accept(this); 
  }

}
  //Algorithm idea: 
  // Increment through vector of notes. If a candidate note compares nicely with the current stack of accepted notes,
  // add it to the stack and continue to the next candidate. If it does not work with the rest of the note coordinates,
  // go back to the previous candidate and increment its index. The current best candidate is put on the stack, and all 
  // future candidates are compared with this candidate note.

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
	//Visit a chunk, and re-arrange all of its notes until they meet the bare requirements specified in the  "c->compare_with_stack()" function
  c->empty_stack();
  int counter_index=0,fail_count=0,super_fail=0;
	  while(counter_index < c->get_children_size()){//cout << counter_index << "<" << c->get_children_size() << ", failcount = " << fail_count <<  endl;
	  	if(super_fail > pow(3,(c->get_children_size() ))  ) //cout << "rotation error: max rotation limit" << endl;
			break;
	  	if(fail_count == c->get_note_at(counter_index)->get_children_size()){
	  	//A note has only so many fret/string positions, indicated by "get_children_size()". after exhausting all tries, go back one step on the stack
	  		c->pop_stack();
	  		counter_index--;
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count=0;
	  		super_fail++;
	  	}
	  	else if(int test = c->compare_with_stack((c->get_note_at(counter_index))))
	  	{
	  		switch(test){
				case DISCARD: //discard note: duplicate
					c->remove_note(c->get_note_at(counter_index));
					break;
				case GOOD: //if the note to be added is compatible with the rest of the stack, continue on to the next note
					c->push_stack(c->get_note_at(counter_index));
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

	  c->inc_lock();
	  
	  if (c->get_lock_val() < pow(3,(c->get_children_size() ))) {//c->get_children_size()){
	 	 compare_chunks(c,c->get_note_indices());	
			for(int i=0;i< (c->get_children_size());i++){
				for(int i2=0;i2<3;i2++){
					if(!in_cache( c->get_note_indices() ) )
						break;
					c->get_note_at(i)->accept(this);
				}
			}
			visitChunk(c);
	  }
	  else{
	  	 c->force_chunk_note_indices();
	  	 //clear_cache();
	  	 c->empty_stack();
	  }
}
void RotateVisitor::compare_chunks(Chunk *c, vector<pair <int, int> > current_indices) {
	//TODO: make this less sloppy and hacky
	//Test max fret spacing, determine which chunk is better between valid configurations
	//the Optima is specific to the chunk

	if (!in_cache(current_indices)){
		_cache.push_back(current_indices);
		if (c->get_optima_size() == current_indices.size()){
			int optima_spacing=0, candidate_spacing=0;
			int fret =0;
			int fmin =24, fmax_optima=0;
			//Inspect the current optima and obtain its fret range
			for(auto i : c->_optima){
				fret = Note::get_fret_at(i.first, i.second);
				//cout << fret << " ";
				if (fret > fmax_optima)
					fmax_optima = fret;
				if (fret < fmin)
					fmin = fret;		
			}
			optima_spacing = fmax_optima-fmin;
			fmin =24;
			int fmax_candidate=0;
			//Inspect the candidate chunk and determine its fret range
			for(auto i : current_indices){
				fret = Note::get_fret_at(i.first, i.second);
				//cout << fret << " ";
				if (fret > fmax_candidate)
					fmax_candidate = fret;
				if (fret < fmin)
					fmin = fret;	
			}
			candidate_spacing=fmax_candidate-fmin;
			//cout << "compared s1, s2: " << optima_spacing << " " << candidate_spacing << endl;
			
			//If candidate spacing is wider than the current spacing by 2 frets
			if (((optima_spacing < candidate_spacing) && ((candidate_spacing - optima_spacing) > 3)) \
			| (fmax_optima < fmax_candidate) )
				return;
		}
		c->set_optima(current_indices);
	}
	
}




void RotateVisitor::visitNote(Note* n) 
{
//  cout << "incrementing note: " << n->get_string() << " " << n->get_fret() <<"to";
  n->increment_note_index();
//  cout <<  n->get_string() << " " << n->get_fret() << endl;
}



