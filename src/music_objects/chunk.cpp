#include "base.h"
#define ALLOWABLE  3

Chunk::Chunk(int d) : delta(d), _recursion_lock(0) {}

Chunk::~Chunk()
{		
	empty_stack();
	for (std::vector< Note* >::iterator it = _chunk_notes.begin() ; it != _chunk_notes.end(); ++it)
		delete (*it);
	_chunk_notes.clear();

}

int Chunk::get_children_size() const 
{
	return _chunk_notes.size();
}

vector<pair<int, int> > Chunk::get_note_indices()
{
	vector<pair<int, int> > indices;
	for(int i=0; i<get_children_size(); i++){
		indices.push_back(pair<int,int>( get_note_at(i)->get_current_note_index(), get_note_at(i)->get_pitch()));
	}
	return indices;
}

void Chunk::accept(Visitor* v) 
{
	v->visitChunk(this);
}
		
void Chunk::add_note(Note* n) 
{
	_chunk_notes.push_back(n);
}

void Chunk::remove_note(Note* n) 
{
	_chunk_notes.erase(std::remove(_chunk_notes.begin(), _chunk_notes.end(), n), _chunk_notes.end()); 
}

void Chunk::force_chunk_note_indices(void)
{
	if (_optima.size() == get_children_size()){
		for(int i=0; i<get_children_size(); i++)
			get_note_at(i)->set_note_index(_optima[i].first);
	}
}

void Chunk::pop_stack(void) 
{
	_comparison_stack.pop();
}

void Chunk::push_stack(Note* n) 
{
	_comparison_stack.push(n); 
}

void Chunk::empty_stack(void)
{
	while(!_comparison_stack.empty()){
		pop_stack();
	}
}

void Chunk::print_stack(void) 
{
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
	  
	  if(n->get_fret() == 0 || current->get_fret() == 0)
	  {
	  	//open frets will not cause conflicts
	  	stack_copy.pop();
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
	  	stack_copy.pop();
	  
	}
	return GOOD;
	
}

int Chunk::get_optima_size(void) 
{ 
	return _optima.size(); 
}

void Chunk::set_optima(vector<pair <int, int> > set) 
{
	_optima = set;
}

void Chunk::inc_lock()
{
	_recursion_lock++;
}

int Chunk::get_lock_val()
{
	return _recursion_lock;
}

void Chunk::set_lock_val(int n) 
{
	_recursion_lock = n;
}

Note* Chunk::get_note_at(int i) 
{
	return _chunk_notes[i];
}

Note* Chunk::get_note_at(void) 
{
	return _chunk_notes.back();
}

int Chunk::get_delta() const 
{
	return delta;
}
		
