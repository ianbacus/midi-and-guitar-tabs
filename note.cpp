#include "base.h"
#define ACCEPTABLE  8
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 
//

Note::PitchMap config()
{
    Note::PitchMap initmap;
    int value;
    int tuning[6];
    tuning[0] = 28;  //E
    tuning[1] = 33; //A
    tuning[2] = 39; //D
    tuning[3] = 43; //G
    tuning[4] = 47; //B
    tuning[5] = 52; //E
     for(int string_ind = 5;string_ind>= 0;string_ind--)
    {
        for(int fret_ind = 0; fret_ind<20; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
                //cout << "gridmap: inserting " << value << " at coordinate point " << string_ind <<", " << fret_ind << endl;
                //tab_matrix[string_ind][fret_ind] = value;
                pair<int,int> map_point = make_pair(string_ind,fret_ind);
                initmap[value].push_back(map_point); //add note to (note : location on fretboard) pitch_to_frets. this will help for determining how many placements there are for a note, and quickly indexing them (is this any faster than indexing the array? TODO)
  	//	vector<pair<int, int> * > &testo =  pitch_to_frets_map.at(value);
  //		cout << (pitch_to_frets_map.at(value))[0]->first << endl;
  //          	cout << value <<  "first/string " << (*pitch_to_frets_map.at(value))[0]->first;
    //        	cout << "second/fret "<< (*pitch_to_frets_map.at(value))[0]->second << endl;
            	
            }
        }
    }
    return initmap;
}

int Note::get_string() const
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[current_note_index].first;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){cout << "up an octave.." << endl;
				n+=12;
			else{ cout<<"down an octave.." << endl;
				n-=12;
		}
	}
}

int Note::get_fret() const
{
	int ret,n=0;
	while(1) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[current_note_index].second;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<28){cout << "up an octave.." << endl;
				n+=12;
			else{ cout<<"down an octave.." << endl;
				n-=12;
		}
	}
}

void Note::increment_note_index()
{
	int sizetemp = get_children_size();
	//does polymorphism apply to calls of "this?" go through base pointer, or through this pointer..
//	cout << get_children_size() << " alternates, at " << current_note_index << ", "<< (current_note_index+sizetemp+1)%sizetemp  << " is next." << endl;
	current_note_index = (current_note_index+1+sizetemp)%(sizetemp);
}
bool Note::compare(Note* note) const
{
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
			if(pitch<28){cout << "up an octave.." << endl;
				n +=12;
			else{ cout<<"down an octave.." << endl;
				n -=12;	
		}
	}
}
	
Note::PitchMap Note::pitch_to_frets_map = config();

