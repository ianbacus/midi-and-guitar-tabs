#ifndef __NOTE__
#define __NOTE__
#include "base.h"

class Note : public Base 
{
	private:

		int pitch;
		int delta;
		int current_note_index;
	public:
		typedef std::map <int, vector< pair<int,int> >  > PitchMap;
	 	static PitchMap pitch_to_frets_map;
		static int noteslost;
	    //	static PitchMap config();
	 	
	 	
	 	//****
		Note(int p, int d) : pitch(p), delta(d), current_note_index(0) {
			PitchMap::iterator it = (pitch_to_frets_map.find(p));
			if(it == pitch_to_frets_map.end()) {//cout << "note error: note lost" << endl;
				noteslost++;
			}
		}
		virtual ~Note() {}
		int get_fret() const;
		int get_string() const;
		int get_noteslost() const {return noteslost;}
		//static void increment_noteslost {noteslost++;}
		void increment_note_index();
		//int get_current_note_index const {return current_note_index;}
		int get_pitch() const {return pitch;}
		int get_delta() const {return delta;}
		
		virtual void accept(Visitor* v) {v->visitNote(this);}
		//virtual int get_children_size() const {return Base_structs::pitch_to_frets_map.find(pitch)->second).size();}
		virtual int get_children_size() const;// {return Base::get_pitch_to_frets_entry_size(pitch);}
		virtual bool compare(Note*) const;
//		virtual void reconfigure();
	
//		void set_fret_string();
	

};

#endif
