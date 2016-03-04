#ifndef __CHUNK__
#define __CHUNK__
#include "Note.h"
#include "base.h"

class Chunk : public Base 
{
	private:
		
		stack<Note*> _comparison_stack;
		int delta;
		int _recursion_lock;
		vector<Note*> _chunk_notes;
		
	public:
		
		vector<pair<int, int> > _optima;
		void add_note(Note* n);
		void remove_note(Note* n);
		void empty_stack();
		void pop_stack();
		void push_stack(Note* n);
		void print_stack();
		int compare_with_stack(Note*);
		
		int get_optima_size(void);
		void set_optima(vector<pair <int, int> > set);
		void force_chunk_note_indices( void );
		
		void inc_lock();
		int get_lock_val();
		void set_lock_val(int n);
		
		Note* get_note_at(int i);
		Note* get_note_at(void); 
		int get_delta() const;
		
		
		virtual int get_children_size() const ;
		virtual vector<pair<int, int> > get_note_indices();
		virtual void accept(Visitor* v) ;
		bool compare_chunks(Chunk*);
		
		Chunk(int d=0);
		virtual ~Chunk(void);
};

enum {
BAD, DISCARD, GOOD,
};

#endif
