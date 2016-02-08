#include "RotateVisitor.h"


#define ALLOWABLE  3

//std::mutex mtx_compstack, mtx_optima, mtx_cache;

//This will recursively check and reconfigure all of the elements of the tree

//This algorithm is terribly inefficient right now, it does a brute force check through permutations
//If chunk configurations were configured in terms of adjacent chunks, then analysis would be simpler. 

//Possibly...
// - separate into a "stringopen" checker and a "fingerable" checker
// - both will use the same algorithm to check iterative from a root, to test all permutations of configurations


void RotateVisitor::visitBar(Bar* b) 
{

  //Go through all of the chunks in a given bar and descend down into them
  //Threads should be used here so that each bar can be analyzed with a thread
  //All of the chunks are already allocated, they are just reconfigured here in place
  
  std::cout << "?";
  for(int i=0; i < b->get_children_size(); i++)
  {
  	//recursion_lock=0;
  	clear_cache();
	std::cout << i;
	
    b->get_child(i)->accept(this);
    cout << i << "!" << endl;
    _chunk_count++;
    }
  std::cout << "666" << std::endl;

}
/*
Algorithm idea: 
Increment through vector of notes. If a candidate note compares nicely with the current stack of accepted notes,
add it to the stack and continue to the next candidate. If it does not work with the rest of the note coordinates,
go back to the previous candidate and increment its index. The current best candidate is put on the stack, and all 
future candidates are compared with this candidate note.

the items are essentially being moved from the vector tree into a stack for comparison purpose, element by element.
the counter_index allows the iterative testing of permutations. If the maximum allowable failure count is reached,
the current existing portion that was assumed to be correct must be wrong. Reversing the counter and incrementing
the note index at that point by one, and then checking all the future notes after that should fix this.


Two levels of "failure":
1: fail count keeps track of the number of string/fret positions that the candidate note has tried to use.
		it is reset when candidate notes are accepted
2: super fail is used to prevent re-checking the same set infinitely once all rotations have been attempted
*/
void RotateVisitor::visitChunk(Chunk* c) 
{
	//Visit a chunk, and re-arrange all of its notes until they meet the bare requirements specified in the  "compare_with_stack" function
  //c->empty_stack();
  int counter_index=0,fail_count=0,super_fail=0;
	  while(counter_index < c->get_children_size())
	  {
	  	cout << "ci:" << counter_index << "cc:" << _chunk_count << "fc:" << fail_count << "sfc:" << super_fail << "rc:" << c->get_recursion_lock() << endl;
	  	if(super_fail > pow(3,(c->get_children_size() ))  ) 
	  	{
			break;
	  	}
	  	if(fail_count == c->get_note_at(counter_index)->get_children_size()){
	  	//A note has only so many fret/string positions, indicated by "get_children_size()". after exhausting all tries, go back one step on the stack
	  		c->pop_stack();
	  		counter_index--;
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count=0;
	  		super_fail++;
	  	}
	  	else if(int test = c->compare_with_stack(c->get_note_at(counter_index)))
	  	{
	  		switch(test){
				case DISCARD: //discard note: duplicate
					c->remove_note(c->get_note_at(counter_index));
					break;
				case GOOD: //if the note to be added is compatible with the rest of the stack, continue on to the next note
					c->push_stack(c->get_note_at(counter_index)); 
					counter_index++;
					cout << "RESET" << endl;
					fail_count=0;
					break;
				case BAD:	
					break;	  		
		  	};
		}
		//should only be these three cases
	  	else{ 
	  		//if the note is incompatible, begin reconfiguring the candidate note
	  		c->get_note_at(counter_index)->accept(this);
	  		fail_count++;
	  	}
	  }
	  cout << "break?" << endl;
	  c->inc_recursion_lock();
	  if (c->get_recursion_lock() < pow(3,(c->get_children_size() ))) 
	  {
	 	 compare_chunks(c->get_note_indices());
			/*
			cout << "current optima["<< _optima.size() << "] <";

			for(int i=0;i<_optima.size();i++)
				cout << Note::get_fret_at(_optima[i].first, _optima[i].second) << ", ";
			cout << ">" << endl;
			*/
			
			for(int i=0;i< (c->get_children_size());i++){
				for(int i2=0;i2<3;i2++){
					
					if(!in_cache( c->get_note_indices() ) ){ break; }
					c->get_note_at(i)->accept(this);
				}
			}//could this be done with a callback
			visitChunk(c);
	  }
	  else{
	  	 force_chunk_note_indices(_optima,c);
	  	 clear_cache();
	  	 c->empty_stack();
	  }
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
	cout << "forcing it in" << endl;
	if (indices.size() == c->get_children_size()){
		for(int i=0; i<c->get_children_size(); i++)
			c->get_note_at(i)->set_note_index(indices[i].first);
	}
	cout << "PENETRATE" << endl;
}



void RotateVisitor::visitNote(Note* n) 
{
  n->increment_note_index();
}
