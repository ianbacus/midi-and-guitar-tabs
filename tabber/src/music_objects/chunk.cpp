#include "base.h"

#include "note.h"
#include "bar.h"
#include "chunk.h"

#define ALLOWABLE_FRET_SPACING  3

Chunk::Chunk(int d) : delta(d), _recursion_lock(0) {}

Chunk::~Chunk()
{		
	empty_stack();
	for (std::vector< Note* >::iterator it = _chunk_notes.begin() ; it != _chunk_notes.end(); ++it)
		delete (*it);
	_chunk_notes.clear();

}

/* 
 *	Get length of chord/chunk (in notes)
 */
int Chunk::get_children_size() const 
{
	return _chunk_notes.size();
}

/* 
 *	Generate a vector of <note-index, pitch> pairs. The note index indicates which fretboard
 *	position the pitch is currently set to
 */
vector<pair<int, int> > Chunk::get_note_indices()
{
	vector<pair<int, int> > indices;
	for(int i=0; i<get_children_size(); i++){
		indices.push_back(pair<int,int>( get_note_at(i)->get_note_position_pitch_map_index(), get_note_at(i)->get_pitch()));
	}
	return indices;
}

/* 
 *	Dispatch a Visitor on a chunk
 */
void Chunk::accept(Visitor* v) 
{
	v->VisitChunk(this);
}

/* 
 *	Insert a note into the vector of notes. Insert in sorted order by the number of frettable
 *	positions for that note.
 */
void Chunk::add_note(Note* n) 
{
	//insert at iterator to first element with a pitch-map entry larger than the candidate note
	_chunk_notes.insert(std::upper_bound( _chunk_notes.begin(), _chunk_notes.end(), n,  \
    [](Note *a, Note*b) { 																\
    	return (a->get_children_size() < b->get_children_size()); 						\
    }),n); 
    
}

/* 
 *	Remove a given note from the chunk
 */
void Chunk::remove_note(Note* n) 
{
	_chunk_notes.erase(std::remove(_chunk_notes.begin(), _chunk_notes.end(), n), _chunk_notes.end()); 
}

/* 
 *	Reconfigure all notes in the chunk to the fret+string positions indicated in the 
 *	current optimum object.
 */
void Chunk::force_chunk_note_indices(void)
{
	if (_optima.size() == get_children_size()){
		for(int i=0; i<get_children_size(); i++)
			get_note_at(i)->set_note_index(_optima[i].first);
	}
}

/* 
 *	Get ith note in this chunk
 */
Note* Chunk::get_note_at(int i) 
{
	return _chunk_notes[i];
}

/* 
 *	Get last note in this chunk, this note will have the most fret+string positions
 */
Note* Chunk::get_note_at(void) 
{
	return _chunk_notes.back();
}

/* 
 *	Get the offset of this chunk from the previous one
 */
int Chunk::get_delta() const 
{
	return delta;
}






//========= Below methods moved from Rotation Visitor for Threading test

/* 
 *	Remove a candidate note from the comparison stack (for rotation)
 */
void Chunk::pop_stack(void) 
{
	_comparison_stack.pop();
}

/* 
 *	Add a candidate note from the comparison stack (for rotation)
 */
void Chunk::push_stack(Note* n) 
{
	_comparison_stack.push(n); 
}

/* 
 *	Reinitialize comparison stack
 */
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


/* 
 *	Add a candidate note from the comparison stack (for rotation)
 */
comparisonResult Chunk::compare_with_stack(Note* n){
// Return true if the note is addable. This method will copy the stack of notes 
    //return false if the note should perform a rotation to a new fret/string
	//bad(0) message: not compatible with chunk
    //discard(1) message: permanently remove note
    //good(2) message: this note works with the current chunk
	if(n->get_pitch() < 0 )
	{
		return GOOD;
	}
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

	  else if(abs((n->get_fret() - current->get_fret())) <= ALLOWABLE_FRET_SPACING)
	  {
	  	// check if the fret position for the candidate note would fit with the current portion of the note stack
	  	//don't bother evaluating this for open strings
	  	return GOOD;
	  }
	  else if(abs((n->get_fret() - current->get_fret())) > ALLOWABLE_FRET_SPACING)
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

/* 
 *	Get the number of fret+string positions in the current optima
 */
int Chunk::get_optima_size(void) 
{ 
	return _optima.size(); 
}

/* 
 *	Set the current optimal configuration of fret+string positions for this chunk
 */
void Chunk::set_optima(vector<pair <int, int> > set) 
{
	_optima = set;
}

/* 
 *	Increment the recursion lock
 */
void Chunk::inc_lock()
{
	_recursion_lock++;
}

/* 
 *	Get the recursion lock value
 */
int Chunk::get_lock_val()
{
	return _recursion_lock;
}

/* 
 *	Set the recursion lock value
 */
void Chunk::set_lock_val(int n) 
{
	_recursion_lock = n;
}

	
