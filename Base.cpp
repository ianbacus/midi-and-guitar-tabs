#include "tabber.h"

void Base::grid_init()
{
    int value;
    for(int string_ind = 0;string_ind<6;string_ind++)
    {
        for(int fret_ind = 0; fret_ind<20; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
                tab_matrix[string_ind][fret_ind] = value;
                pair<int,int> map_point = make_pair(string_ind,fret_ind);
                pitch_to_frets_map[value].push_back(&map_point); //add note to (note : location on fretboard) pitch_to_frets. this will help for determining how many placements there are for a note, and quickly indexing them (is this any faster than indexing the array? TODO)
            }
        }
    }
}


int Base::get_string(int pitch,int index) const
{
	return (*pitch_to_frets_map.find(pitch) )[index]->first;
}

int Base::get_fret(int pitch, int index) const
{
	return (*pitch_to_frets_map.find(pitch))[index]->second;
}

size_t Base::get_pitch_to_frets_entry_size(int pitch) const
{
	return (pitch_to_frets_map.find(pitch))->size();
}
