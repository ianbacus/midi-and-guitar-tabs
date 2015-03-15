#include "base.h"
#define ACCEPTABLE  8
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 
// 
Note::Note(int pitch_init)
{
	pitch = pitch_init;
	current_note_index = 0;

}

int Note::get_string() const
{
	//return pitch_to_frets_map[pitch][current_note_index]->second;
//	return Base_structs::(pitch_to_frets_map.find(pitch)->second)[index]->first;
//cout << "Note::" << pitch << " " << get_string_pos(pitch,current_note_index) << endl;
	return get_string_pos(pitch,current_note_index);
}

int Note::get_fret() const
{
	//return pitch_to_frets_map[pitch][current_note_index]->first;
	return get_fret_pos(pitch,current_note_index);
//	return Base_structs::(pitch_to_frets_map.find(pitch)->second)[index]->second;
	
}

void Note::increment_note_index()
{
	//does polymorphism apply to calls of "this?" go through base pointer, or through this pointer..
	current_note_index += (current_note_index+1)%(get_children_size());
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
