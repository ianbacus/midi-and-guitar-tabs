#ifndef __NOTE__
#define __NOTE__
#include "base.h"

#include "tuning.h"

class Note : public Base 
{
	private:

		int pitch;
		int delta;
		int current_note_index;
		int octave_refcount;
	public:
		typedef std::map <int, vector< pair<int,int> >  > PitchMap;
	 	static PitchMap pitch_to_frets_map;
		static int noteslost;
		
		Note(int p, int d); 
		virtual ~Note() {}
		int get_fret() ;
		
		static int get_fret_at(int,int);
		static int get_string_at(int, int);
		//virtual bool compare(Note*) ;
		
		int get_string();
		int get_noteslost() const ;
		void set_note_index(int n) ;
		int get_current_note_index() const;
		void increment_note_index();
		void decrement_octave();
		void increment_octave();
		void rebalance_note();
		
		int get_pitch() const ;
		void alter_pitch(int n);
		int get_delta()  const;
		
		virtual void accept(Visitor* v);
		virtual int get_children_size() const;


};

#endif
