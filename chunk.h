#ifndef __CHUNK__
#define __CHUNK__
#include "Note.h"
#include "base.h"


//implement as doubly linked list
class Chunk : public Base 
{
	public:
		int delta;
		vector<Note*> _chunk_notes;
	
		void add_Note(Note*); //this should automatically enforce good fingering with the tuning matrix
		void rotate();
		
		Note* get_note_at(int i) {return _chunk_notes[i];}
		int get_chunk_size() {return _chunk_notes.size();}
		//vector<Note*> * get_chunk_notes_vector() {return &_chunk_notes;}
		
		virtual void accept(Visitor* v) {v.visitChunk(this);}
	//	virtual void reconfigure(); //removed parameter Base*
		bool compare_chunks(Chunk*);
	
	
};


#endif
