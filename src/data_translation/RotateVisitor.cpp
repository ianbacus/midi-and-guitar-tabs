#include "RotateVisitor.h"

//Recursively check and reconfigure all of the elements of the tree

/* 
   Increment through vector of notes. If a candidate note compares nicely with the \
  current stack of accepted notes,
   add it to the stack and continue to the next candidate. If it does not work with \
  the rest of the note coordinates,
   go back to the previous candidate and increment its index. The current best candidate \
  is put on the stack, and all 
   future candidates are compared with this candidate note.

 the items are essentially being moved from the vector tree into a stack for comparison \
 purpose, element by element.
  the counter_index allows the iterative testing of permutations. If the maximum allowable\
  failure count is reached,
  the current existing portion that was assumed to be correct must be wrong. Reversing\
  the counter and incrementing
  the note index at that point by one, and then checking all the future notes after that \
 should fix this.


Two levels of "failure":
 - fail count keeps track of the number of string/fret positions that the candidate note\
 has tried to use.
		it is reset when candidate notes are accepted
 - super fail is used to prevent re-checking the same set infinitely once all rotations \
have been attempted
*/

#define PERMUTATION_MAX_BASE 3


void printNoteIndices(vector<pair <int, int> > current_indices)
{
	std::cout << "-" << std::endl;
	int notefret = 0;
	char notestring = '0';
	for(auto i: current_indices)
	{
		notefret = Note::get_fret_at(i.first, i.second);
		notestring = ptuning[Note::get_string_at(i.first, i.second)];
		std::cout << notefret << notestring << std::endl;
	}
	std::cout << "-" << std::endl;
}

void RotateVisitor::visitBar(Bar* b) 
{
  //Go through all of the chunks in a given bar and descend down into them

  for(int i=0; i < b->get_children_size(); i++)
  {
  	Chunk* c = b->get_child(i);
  	c->accept(this);
  	/*
  	if(!checkStore(c))
  	{
    	c->accept(this);
    	addToStore(c);
    }
    */
  }

}



/*

inter-chunk optimization:
1. match adjacent elements (outwards from) chunks that cannot be reconfigured:
	the most immobile note in a chunk will limit its mobility
	
2. come up with a set of different valid configurations for each chunk of notes (LUT style),
	treat each bar like a chunk and perform rotations through the positions of each chunk
	with a stack of candidate chunks
3. optimize chunks with more than n elements, configure the n-1 sized chunks to match these
4. find a "note center" for each chunk, formulate rules in terms of these to prioritize more
	closely spaced note centers for adjacent individual chunks than narrower chunk fret widths
5. 


*/


void RotateVisitor::recursivePermutationCompare(Chunk* c)
{
	c->inc_lock();

	if (c->get_lock_val() < pow(PERMUTATION_MAX_BASE,(c->get_children_size() ))) 
	{
		compare_chunks(c,c->get_note_indices());	
		for(int i=0;i< (c->get_children_size());i++)
		{
			for(int i2=0;i2<3;i2++){
				if(!in_cache( c->get_note_indices() ) )
					break;
				c->get_note_at(i)->accept(this);
			}
		}
		visitChunk(c);
	}
	else
	{
		c->force_chunk_note_indices();
		clear_cache();
		c->empty_stack();
	}
}

void RotateVisitor::conditionallyAddToStack(comparisonResult test, Chunk *c, 
											int &counter_index, int& fail_count)
{
	switch(test)
	{
		case DISCARD: //discard note from chunk
			//cout << "discard" << endl;
			c->remove_note(c->get_note_at(counter_index));
			break;
		case GOOD: //lock in compatible note for now
			c->push_stack(c->get_note_at(counter_index));
			counter_index++;
			fail_count=0;
			break;
		case BAD:	
			break;
	};
}

void RotateVisitor::visitChunk(Chunk* c) 
{
	//Visit a chunk, and re-arrange all of its notes until they are valid/playable

	c->empty_stack();
	const int max_permutation_limit = pow(PERMUTATION_MAX_BASE,(c->get_children_size() ));
	int counter_index=0,fail_count=0,max_permutation_counter=0;
	comparisonResult result;
	while(counter_index < c->get_children_size())
	{
		if(max_permutation_counter > max_permutation_limit)
		{
			//go with local minimum
			break; 
		}
		else if(fail_count == c->get_note_at(counter_index)->get_children_size()) 
		{
			//after fruitlessly exhausting all candidate positions, unwind stack by one step
			fail_count=0;
			max_permutation_counter++;
			counter_index--;
			c->pop_stack();
			c->get_note_at(counter_index)->accept(this);
			
		}
		else if( (result = c->compare_with_stack((c->get_note_at(counter_index))) ) )
		{
			conditionallyAddToStack(result, c, counter_index, fail_count);
		}
		else
		{
			//if the note is incompatible, begin reconfiguring the candidate note
			c->get_note_at(counter_index)->accept(this);
			fail_count++;
		}
	}
	recursivePermutationCompare(c);

	
}

int getFretMaxAndOptimaSpacing(int &fmax,vector<pair <int, int> > indices)
{
	/*
	Find the maximum fret in the vector of note grid positions, and the maximum spacing
	*/
	int fmin = 24,fret=0,spacing=0;
	for(auto i : indices)
	{
		fret = Note::get_fret_at(i.first, i.second);
		if(fret == 0) continue;
		if (fret > fmax)
			fmax = fret;
		if (fret < fmin)
			fmin = fret;		
	}
	spacing = fmax-fmin;
	if (spacing < 0) spacing = 0;
	return spacing;
}
void RotateVisitor::compare_chunks(Chunk *c, vector<pair <int, int> > current_indices) 
{
	//Test max fret spacing, determine which chunk is better between valid configurations
	/*
		If the spacings are equal, then choose the pair with the lower frets (determined by the sum of the frets)
		If the spacings are different, go with the one with the less total spacing
	*/
	if (!in_cache(current_indices)){
		_cache.push_back(current_indices);
		if (c->get_optima_size() == current_indices.size()){
			int fmax_optima = 0,fmax_candidate =0;
			int optima_spacing = getFretMaxAndOptimaSpacing(fmax_optima,c->_optima);
			int candidate_spacing = getFretMaxAndOptimaSpacing(fmax_candidate,current_indices);
			//If candidate spacing is wider than the current spacing by 2 frets, cancel candidate
			//cout << "------\noptima:" << "spacing=" << optima_spacing << "fmax=" << fmax_optima << endl;
			//printNoteIndices(c->_optima);
			//cout << "candidate:" << "spacing=" << candidate_spacing << "fmax=" << fmax_candidate << endl;
			//printNoteIndices(current_indices);
			#define SPACING(a,b,c) ((a < b) && ((b - a) > c))
			
			//candidate will be discarded unless it is better than the current optima
			bool candidateWins = false; 
			if((abs(optima_spacing-candidate_spacing) < 3))
			{
				if(fmax_candidate < fmax_optima) candidateWins = true;
				else candidateWins = false;
			}
			else if(SPACING(candidate_spacing,optima_spacing,0))
			{
				candidateWins = true;
			}
			if(candidateWins == false)
			{
				return;
			}
		}
		c->set_optima(current_indices);
	}	
}

void RotateVisitor::visitNote(Note* n) 
{
  n->increment_note_index();
}

void RotateVisitor::print_chunk_count(void) 
{
	std::cout << _chunk_count << std::endl; 
}

bool RotateVisitor::in_cache(vector<pair<int, int> > input)
{
	bool ret = false;
	//mtx_cache.lock();
	for(auto entry : _cache){
		if(entry == input) 
		{
			ret = true;
		}
	}
	//mtx_cache.unlock();
	return ret;
}

void RotateVisitor::clear_cache(void)
{
	//mtx_cache.lock();
	_cache.clear();
	//mtx_cache.unlock();
}



/*
void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

bool RotateVisitor::checkStore(Chunk *c)
{
	String line;
	String chunkString;
	Set<int> pitchSet;
	//Check persistent store
	ifstream fromStore ( "test.txt", ios::app );
	if ( !fromStore.is_open() ) {
	  // The file could not be opened
	}
	else 
	{
	  // Safely use the file stream
	  //Reads one string from the file
		for(int i=0;i< (c->get_children_size());i++)
		{
			pitchSet.insert(c->get_note_at(i));
		}
		for(auto element: pitchSet)
		{
			chunkString += std::to_string(element)+",";
		}
		do
		{
			std::getline(fromStore,line);
			std::vector<std::string> entry = split(line,'X');
		} while(chunkString != entry[0]);
		for(auto pair : split(entry[1],"P")
		{
			
		}
		
	}
}

void RotateVisitor::addToStore(Chunk *c)
{
	ofstream toStore ( "test.txt", ios::app );
	// Outputs to example.txt through a_file
	
	String entryString;
	Set<int> pitchSet;
	for(int i=0;i< (c->get_children_size());i++)
	{
		pitchSet.insert(c->get_note_at(i));
	}
	for(auto element: pitchSet)
	{
		entryString += std::to_string(element)+",";
	}
	entryString += "X";
	for(auto element: c->_optima)
	{
		//convert pairs to strings
		entryString += std::to_string(element.first)+","+std::to_string(element.first)+",";
	}
	entryString+="P";
	// Close the file stream explicitly
	toStore.close();
	
}

*/

