#include "base.h"
#include "tuning.h"
#define ACCEPTABLE  5
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 

Note::PitchMap config()
{
    Note::PitchMap initmap;

    int value;

    for(int string_ind = SIZEOF_TUNING;string_ind>= 0;string_ind--)
    {
        for(int fret_ind = 0; fret_ind<15; fret_ind++)
        {
            {
            	
                value = tuning[string_ind] + fret_ind;
				pair<int,int> map_point = make_pair(string_ind,fret_ind);
                initmap[value].push_back(map_point); 
                
                //add note to (note : location on fretboard) pitch_to_frets. 
                //this will help for determining how many placements there are for a note
                	
            }
            
        }
    }
    return initmap;
}

Note::Note(int p, int d, int tn) : pitch(p), delta(d), track_num(tn), current_note_index(0) 
{
	//Negative delta values are used to represent triples
	PitchMap::iterator it = (pitch_to_frets_map.find(p));
	if(it == pitch_to_frets_map.end()) 
	{
		noteslost++;
	}
}

int Note::get_track_num() const
{
	return track_num;
}

int Note::get_string() 
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[current_note_index].first;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){ alter_pitch(12);}
			else{ alter_pitch(-12);}
		}
	}
}

int Note::get_fret_at(int n_index, int pitch)
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[n_index].second;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){n+=12;}
			else{ n-=12;}
		}
	}
}

void Note::decrement_octave()
{
  pitch-=12;
  octave_refcount--;
}

void Note::increment_octave()
{
  pitch+=12;
  octave_refcount++;
}

void Note::rebalance_note()
{
  pitch += octave_refcount*12;
  octave_refcount = 0;
}
		
int Note::get_fret() 
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch)[current_note_index].second;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){ alter_pitch(12);}
			else{ alter_pitch(-12);}
		}
	}
}

void Note::increment_note_index()
{
	int sizetemp = get_children_size();
	current_note_index = (current_note_index+1)%(sizetemp);
}



bool Note::compare(Note* note) {
	//this function will be deleted
	if(note->get_string() == get_string()) return false;
	else
	{
		if(note->get_fret() == 0 || get_fret() == 0) return true;
		if( abs(note->get_fret() - get_fret()) > ACCEPTABLE) return false;
		else return true;
	}
}

int Note::get_children_size() const
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch+n).size();
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){n +=12;}
			else{ n -=12;	}
		}
	}
}
	
int Note::get_noteslost() const 
{
	return noteslost;
}

void Note::set_note_index(int n) 
{
	current_note_index = n;
}

int Note::get_current_note_index() const 
{
	return current_note_index;
}
		
int Note::get_pitch() const 
{
	return pitch;
}

void Note::alter_pitch(int n)
{
	pitch +=n; 
}

int Note::get_delta()  const
{
	return delta;
}

void Note::accept(Visitor* v) 
{
	v->visitNote(this);
}


//At compile time: generate the pitch to fret mapping
Note::PitchMap Note::pitch_to_frets_map = config();
int Note::noteslost = 0;

