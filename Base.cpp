#include "base.h"



/*
int Base::get_string_pos(int pitch,int index) const
{
	//cout << pitch << " is on string " << ((pitch_to_frets_map.at(pitch))[index]->first) << endl;
	int ret;
	try {
		ret = (pitch_to_frets_map.at(pitch))[index]->first;
	}
	catch (const std::out_of_range& oor) {
		std::cerr << "Out of Range error: " << oor.what() << '\n';
		//cout << "uh oh";
	}
	return ret;
	
}

int Base::get_fret_pos(int pitch, int index) const
{
	//const vector<pair<int,int>* > pairy = (pitch_to_frets_map.find(pitch)->second);
	//cout << pairy[index]->second << endl;
	//cout << pitch << " is on fret " << ((pitch_to_frets_map.at(pitch))[index]->second) << endl;
	int ret;
	try {
		ret = (pitch_to_frets_map.at(pitch))[index]->second;
	}
	catch (const std::out_of_range& oor) {
		std::cerr << "Out of Range error: " << oor.what() << '\n';
		//cout << "uh oh";
	}
	return ret;
}

int Base::get_pitch_to_frets_entry_size(int pitch) const
//should be renamed "get_pitch_to_frets_vector_size". Takes pitch as an input, searches the base map.
//returns the size of a vector of tab matrix coordinates associated with a pitch. Equivalently, returns the number
// of frettable locations for a note.
{
//	cout << "entry is " << (pitch_to_frets_map.find(pitch)->second).size() << " units long" << endl;
	return (pitch_to_frets_map.at(pitch)).size();

}*/

