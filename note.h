#ifndef __NOTE__
#define __NOTE__
#include "base.h"

class Note : public Base 
{
	private:
	    	typedef std::map <int, vector< pair<int,int> * >  > PitchMap;
	 	static PitchMap pitch_to_frets_map;
	    	static PitchMap config();
	    	int pitch;
	    	int delta;
		int current_note_index;
	public:
		Note(int);

		int get_fret() const;
		int get_string() const;
		
		void increment_note_index();
		//int get_current_note_index const {return current_note_index;}
		int get_pitch() const {return pitch;}
		
		
		virtual void accept(Visitor* v) {v->visitNote(this);}
		//virtual int get_children_size() const {return Base_structs::pitch_to_frets_map.find(pitch)->second).size();}
		virtual int get_children_size() const;// {return Base::get_pitch_to_frets_entry_size(pitch);}
		virtual bool compare(Note*) const;
//		virtual void reconfigure();
	
//		void set_fret_string();
	

};

#endif
