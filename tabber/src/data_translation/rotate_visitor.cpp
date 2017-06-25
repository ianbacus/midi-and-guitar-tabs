#include "rotate_visitor.h"

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

void printNoteIndices(vector<pair <int, int> > currentNoteConfigurations)
{
//      Print the fret and string of each note in the current chunk configuration	
	std::cout << "-" << std::endl;
	int notefret = 0;
	char notestring = '0';
	for(auto i: currentNoteConfigurations)
	{
		notefret = Note::get_fret_at(i.first, i.second);
		notestring = ptuning[Note::get_string_at(i.first, i.second)];
		std::cout << notefret << notestring << std::endl;
	}
	std::cout << "-" << std::endl;
}

void RotateVisitor::VisitBar(Bar* bar) 
{
  //Go through all of the chunks in a given bar and reconfigure them
  for(int i=0; i < bar->get_children_size(); i++)
  {
  	Chunk* candidateChunk = bar->get_child(i);
  	candidateChunk->accept(this);
  }

}




void RotateVisitor::RecursivelyFindLowestCostChunkConfiguration(Chunk* candidateChunk)
{
    /*
	Recursively reconfigure note positions in a given chunk until they are acceptable

	1. Begin with least mobile notes
	2. come up with a set of different valid configurations for each chunk of notes (LUT style),
		treat each bar like a chunk and perform rotations through the positions of each chunk
		with a stack of candidate chunks
	3. optimize chunks with more than n elements, configure the n-1 sized chunks to match these
	4. find a "note center" for each chunk, formulate rules in terms of these to prioritize more
		closely spaced note centers for adjacent individual chunks than narrower chunk fret widths
	*/

	//Make sure that the permutation limit has not been reached
	candidateChunk->inc_lock();
	if (candidateChunk->get_lock_val() < pow(PERMUTATION_MAX_BASE,(candidateChunk->get_children_size() ))) 
	{
		CompareChunks(candidateChunk,candidateChunk->get_note_indices());	
		
		for(int i=0;i< (candidateChunk->get_children_size());i++)
		{
			for(int i2=0;i2<3;i2++)
			{
				if(!InCache( candidateChunk->get_note_indices() ) )
				{
					break;
				}
				candidateChunk->get_note_at(i)->accept(this);
			}
		}

		VisitChunk(candidateChunk);
	}

	else
	{
		candidateChunk->force_chunk_note_indices();
		ClearCache();
		candidateChunk->empty_stack();
	}
}

/*
 *	
 */	
void RotateVisitor::ConditionallyAddToStack(comparisonResult test, Chunk *candidateChunk, int &counter_index, int& fail_count)
{
	//Handle a test result

	switch(test)
	{
		case DISCARD: //discard note from chunk
			candidateChunk->remove_note(candidateChunk->get_note_at(counter_index));
			break;
		case GOOD: //lock in compatible note for now
			candidateChunk->push_stack(candidateChunk->get_note_at(counter_index));
			counter_index++;
			fail_count=0;
			break;
		case BAD:	
			break;
	};
}

/*
 *	
 */	
void RotateVisitor::VisitChunk(Chunk* candidateChunk) 
{
	//Visit a chunk, and re-arrange all of its notes until they are valid/playable

	const int max_permutation_limit = pow(PERMUTATION_MAX_BASE,(candidateChunk->get_children_size() ));
	int counter_index=0,fail_count=0,max_permutation_counter=0;
	comparisonResult result;

	candidateChunk->empty_stack();

	while(counter_index < candidateChunk->get_children_size())
	{
		if(max_permutation_counter > max_permutation_limit)
		{
			//go with local minimum
			break; 
		}
		else if(fail_count == candidateChunk->get_note_at(counter_index)->get_children_size()) 
		{
			//after fruitlessly exhausting all candidate positions, unwind stack by one step
			fail_count=0;
			max_permutation_counter++;
			counter_index--;
			candidateChunk->pop_stack();
			candidateChunk->get_note_at(counter_index)->accept(this);
			
		}
		else if( (result = candidateChunk->compare_with_stack((candidateChunk->get_note_at(counter_index))) ) )
		{
			ConditionallyAddToStack(result, candidateChunk, counter_index, fail_count);
		}
		else
		{
			//if the note is incompatible, begin reconfiguring the candidate note
			candidateChunk->get_note_at(counter_index)->accept(this);
			fail_count++;
		}
	}

	RecursivelyFindLowestCostChunkConfiguration(candidateChunk);
}

/*
 *	
 */	
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


bool IsFretSpacingValid(uint32_t candidateMaximumFret, uint32_t currentMaximumFret, uint32_t minimumDifferenceForReplacement)
{
	//If candidate spacing is wider than the current spacing by 2 frets, cancel candidate		

	return ((candidateMaximumFret < currentMaximumFret) && ((currentMaximumFret - candidateMaximumFret) > minimumDifferenceForReplacement));
}

/*
 *	
 */	
void RotateVisitor::CompareChunks(Chunk *candidateChunk, vector<pair <int, int> > currentNoteConfigurations) 
{
	//Test max fret spacing, determine which chunk is better between valid configurations
	/*
		If the spacings are equal, then choose the pair with the lower frets (determined by the sum of the frets)
		If the spacings are different, go with the one with the less total spacing
	*/
	if (!InCache(currentNoteConfigurations))
	{
		ValidChunkConfigurations.push_back(currentNoteConfigurations);
	
		if (candidateChunk->get_optima_size() == currentNoteConfigurations.size())
		{
			int maximumFretInCurrentConfiguration = 0;
			int maximumFretInCandidateChunk = 0;
			int optima_spacing = getFretMaxAndOptimaSpacing(maximumFretInCurrentConfiguration,candidateChunk->_optima);
			int candidate_spacing = getFretMaxAndOptimaSpacing(maximumFretInCandidateChunk,currentNoteConfigurations);
			
			//candidate will be discarded unless it is better than the current optima
			bool candidateChunkIsMorePlayable = false; 
			
			if((abs(optima_spacing-candidate_spacing) < 3))
			{
				
				if(maximumFretInCandidateChunk < maximumFretInCurrentConfiguration)
				{
					 candidateChunkIsMorePlayable = true;
				}
				
				else
				{
					 candidateChunkIsMorePlayable = false;
				}	
			}
			
			else if(IsFretSpacingValid(candidate_spacing,optima_spacing,0))
			{
				candidateChunkIsMorePlayable = true;
			}
			
			if(candidateChunkIsMorePlayable == false)
			{
				return;
			}
		}

		candidateChunk->set_optima(currentNoteConfigurations);
	}	
}

/*
 *	"Rotate" note when vvisiting (move it one position forward through its available fret+string
 *	positions.
 */	
void RotateVisitor::VisitNote(Note* n) 
{
  n->increment_note_index();
}


	
bool RotateVisitor::InCache(vector<pair<int, int> > input)
{
	/*
	*	Determine if the chunk configuration is already present in the chunk cache
	*/

	bool ret = false;
	
	for(auto entry : ValidChunkConfigurations)
	{
		if(entry == input) 
		{
			ret = true;
		}
	}
	
	return ret;
}

/*
 *	Empty cache of optimum chunk configurations
 */	
void RotateVisitor::ClearCache(void)
{
	ValidChunkConfigurations.clear();
}
