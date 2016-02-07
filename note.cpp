#include "base.h"
#include "tuning.h"
#define ACCEPTABLE  5
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 
//

Note::PitchMap config()
{
//PitchMap is std::map <int, vector< pair<int,int> >  > 
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
                //this will help for determining how many placements there are for a note, 
                //and quickly indexing them (is this any faster than indexing the array? TODO)
  
                //cout << "gridmap: inserting note " << value << " at coordinate string " << geetarstring[string_ind] <<", fret " << fret_ind << endl;
                //tab_matrix[string_ind][fret_ind] = value;
                	
            }
            
        }
    }
    return initmap;
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

int Note::get_fret_at(int n_index, int pitch){
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
void Note::decrement_octave(){
  pitch-=12;
  octave_refcount--;
}

void Note::increment_octave(){
  pitch+=12;
  octave_refcount++;
}
void Note::rebalance_note(){
  pitch += octave_refcount*12;
  octave_refcount = 0;
}
		
int Note::get_fret() {
//TODO: make this method static and replace it elsewhere in code
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
//	cout << get_children_size() << " alternates, at " << current_note_index << ", "<< (current_note_index+sizetemp+1)%sizetemp  << " is next." << endl;
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

/*	//compare Notes by index in the tabmatrix
	for(int s=0;s<=5;s++)
	{
		for(int fret=0;fret<=20;fret++)
		{
			//inefficient, but the matrix is a fixed size so this runs in O(1) time
			if(tab_matrix[i][j] == pitch)
			{
				
			}
		}
	}
	*/
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
	

Note::PitchMap Note::pitch_to_frets_map = config();
int Note::noteslost = 0;