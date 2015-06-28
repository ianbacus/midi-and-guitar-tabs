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
	//	virtual void reconfigure(); //removed parameter Base*
		bool compare_chunks(Chunk*);
		Chunk(int d=0) : delta(d) {}
		virtual ~Chunk(){
			for (std::vector< Note* >::iterator it = _chunk_notes.begin() ; it != _chunk_notes.end(); ++it)
  				delete (*it);
  			_chunk_notes.clear();
  
		}
	
	
};


#endif
