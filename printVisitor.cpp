#include "printvisitor.h"


void PrintVisitor::visitBar(Bar* b)
{
    for(string_print_index=SIZEOF_TUNING; string_print_index>= 0; string_print_index--){
    	if(string_print_index == SIZEOF_TUNING) {
    		string_buffer.back()[SIZEOF_TUNING] += "   ";
    		continue;
    		
    	}
    	(string_buffer.back())[string_print_index] += "|";
		for(int j=0; j<b->get_children_size(); j++){

			b->get_child(j)->accept(this);
			}
		(string_buffer.back())[string_print_index] += "-";
		if(!tripled) string_buffer.back()[SIZEOF_TUNING] += "   ";
		tripled = false;
	}
    //TODO: add logic to this to make the number of continuous bar prints variable by the client
    
}


void PrintVisitor::visitChunk(Chunk* c){
/*
	Visit a chunk and iterate through all of its notes. Only visit notes which are on the currently set
	string index, which each enclosing bar decrements through. Decrementing is chosen arbitrarily (?)	

*/
//	cout << "visiting chunk" << endl;
	strings_closed=false;
	bool locked = false;
	
	//bar_ticks_inc(c.get_delta());
	
	int delta = 0;
	string result;
	Note* current_note;
	int j = c->get_children_size();
	if(c->get_children_size() == 0)
		return;
	for(int j=0; j<c->get_children_size(); j++){
		//cout << j << " out of " << c->get_children_size() << endl;
		current_note = c->get_note_at(j);
		if(!strings_closed)
			current_note->accept(this);
		if(strings_closed && !locked){
			int fret = current_note->get_fret();
			if(fret < 10)
				result = "-" + std::to_string(fret);
			else
				result = std::to_string(fret);		
			locked = true;
			}
			
		delta += current_note->get_delta(); //TODO: change this logic to use chunk deltas instead of note deltas
	}
	if(!strings_closed)//none of the chunk members were on the current string index, so print null space instead
		result = "--" ;
		
	//delta = log2(delta);	
	// cout << result << endl;

	//This is where the padding takes place for notes based on their note duration
	if(delta >= 0){
		
		string_buffer.back()[string_print_index] += std::string(delta,'-') + result;
	 }
	else {
	//triplets
		tripled = true;
		string_buffer.back()[SIZEOF_TUNING] += ' t';
		string_buffer.back()[string_print_index] +=  result;
	}

}

void PrintVisitor::visitNote(Note* n){
  //int fret = n->get_fret();
  if(n->get_string() == string_print_index){
      	strings_closed = true; 
      	//only one note will be printed for a chunk at a time

    }

}
