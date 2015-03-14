#ifndef __CHUNK__
#define __CHUNK__
#include "Note.h"
#include "Tabber.h"


//implement as doubly linked list
class Chunk : public Base 
{
	public:
		int delta;
		vector<Note*> Chunk_Notes;
	
		void add_Note(Note*); //this should automatically enforce good fingering with the tuning matrix
		void rotate();
		vector<Note*> * get_chunk_notes_vector() {return &Chunk_Notes;}
		
		virtual void accept(Visitor* v) {v.visitChunk(this);}
	//	virtual void reconfigure(); //removed parameter Base*
		bool compare_chunks(Chunk*);
	
	
};


#endif
