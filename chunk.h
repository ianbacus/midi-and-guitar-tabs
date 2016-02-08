#ifndef __CHUNK__
#define __CHUNK__
#include "Note.h"
#include "base.h"


//implement as doubly linked list
class Chunk : public Base 
{
	private:
		int delta;
		vector<Note*> _chunk_notes;
		stack<Note*> _comparison_stack;
		int _recursion_lock;
	public:
		
		void add_note(Note* n) {_chunk_notes.push_back(n);} //this should automatically enforce good fingering with the tuning matrix
		void remove_note(Note* n) {
			//_chunk_notes.erase( _chunk_notes.erase(std::find(_chunk_notes.begin(),_chunk_notes.end(),n)));
			_chunk_notes.erase(std::remove(_chunk_notes.begin(), _chunk_notes.end(), n), _chunk_notes.end()); 
			}
		
		//void rotate();
		
		Note* get_note_at(int i) {return _chunk_notes[i];}
		Note* get_note_at(void) {return _chunk_notes.back();}
		int get_delta() const {return delta;}
		virtual int get_children_size() const {return _chunk_notes.size();}
		virtual vector<pair<int, int> > get_note_indices(){
			vector<pair<int, int> > indices;
			for(int i=0; i<get_children_size(); i++){
				indices.push_back(pair<int,int>( get_note_at(i)->get_current_note_index(), get_note_at(i)->get_pitch()));
			}
			return indices;
		}
		//vector<Note*> * get_chunk_notes_vector() {return &_chunk_notes;}
		
		virtual void accept(Visitor* v) {v->visitChunk(this);}
		
		void empty_stack();
		void pop_stack() 
		{ 
			_comparison_stack.pop();
		}
		void push_stack(Note* n) 
		{ 
			_comparison_stack.push(n); 
		}
		void print_stack();
		int compare_with_stack(Note*);
		int get_recursion_lock() {return _recursion_lock;}
		void set_recursion_lock(int n) {_recursion_lock = n;cout<<"RECLOCK_SET:"<<_recursion_lock <<endl;}
		void inc_recursion_lock() {_recursion_lock++; cout<<"RECLOCK++"<<_recursion_lock <<endl;}
		
		
		bool compare_chunks(Chunk*);
		Chunk(int d=0) : delta(d), _recursion_lock(0) {}
		virtual ~Chunk(){
			
	  		empty_stack();
			for (std::vector< Note* >::iterator it = _chunk_notes.begin() ; it != _chunk_notes.end(); ++it)
  				delete (*it);
  			_chunk_notes.clear();
  
		}
	
	
};


enum {
BAD, DISCARD, GOOD,
};


#endif
