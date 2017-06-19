#include "base.h"
#include "tuning.h"
#define ACCEPTABLE  5
//Note should have a vector of possible string positions (which depend on the tuning in the tab matrix) by default
// hold an index to tabmatrix, update by incrementing 

/*
 *	Print out initialization map
 */
void print_initmap(std::map<int,vector< pair<int,int> > > initmap)
{
	for(std::map<int,vector< pair<int,int> > >::iterator iter = initmap.begin(); iter != initmap.end(); ++iter)
	{
		int k =  iter->first;
		//ignore value
		vector< pair<int,int> > v = iter->second;
		cout << k;
		for(auto p : v)
		{
			cout << "(" << p.first << "s," << p.second << "f), ";
		}
		cout << endl;
	}
}

/*
 *	Configure the pitch map: maps midi pitch values to pairs of string and fret positions.
 *	note index indicates which fret+string position is currently being used
 */
Note::PitchMap config()
{
    Note::PitchMap initmap;

    int value;

    for(int string_ind = SIZEOF_TUNING-1;string_ind>= 0;string_ind--)
    {
        for(int fret_ind = 0; fret_ind<14; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
				pair<int,int> map_point = make_pair(string_ind,fret_ind);
                
                //Insert sorted by fret number
                initmap[value].insert(std::upper_bound( initmap[value].begin(), initmap[value].end(), map_point, \
                [](const std::pair<int,int> &left, const std::pair<int,int> &right) { return left.second < right.second; }),map_point ); 
            }
            
        }
    }
    return initmap;
}

/*
 *	Construct a note with pitch, delta, track number
 */
Note::Note(int p, int d, int tn) : pitch(p), delta(d), track_num(tn), current_note_index(0) 
{
	//Negative delta values are used to represent triples
	//Negative pitch values represent rests
	if(p < 0)
	{
		
	} 
	else if ((pitch_to_frets_map.find(p)) == pitch_to_frets_map.end()) 
	{
		noteslost++;
	}
}

/*
 *	Get track number note was taken from
 */
int Note::get_track_num() const
{
	return track_num;
}

/*
 *	Get fret for the current note index
 */
int Note::get_fret() 
{
	int ret,n=0;
	if(pitch<0) return -1;
	int i = 0;
	while(++i < 10) {
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

/*
 *	Get the fret for a given note index and pitch
 */
int Note::get_fret_at(int n_index, int pitch)
{
	int ret,n=0;
	if(pitch<0) return -1;
	int i = 0;
	while(++i < 10) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[n_index].second;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<tuningMinimum){n+=12;}
			else{ n-=12;}
		}
	}
}

/*
 *	Get string for the current note index
 */
int Note::get_string() 
{
	int ret,n=0;
	int i = 0;
	while(++i < 10) {
		
		try {
			ret = pitch_to_frets_map.at(pitch+n)[current_note_index].first;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<tuningMinimum){ alter_pitch(12);}
			else{ alter_pitch(-12);}
		}
	}
}

/*
 *	Get string for a given note index and pitch
 */
int Note::get_string_at(int n_index, int pitch)
{
	int ret,n=0;
	int i = 0;
	while(++i < 10) {
		try {
			ret = pitch_to_frets_map.at(pitch+n)[n_index].first;
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<tuningMinimum){n+=12;}
			else{ n-=12;}
		}
	}
}

/*
 *	Transpose a note down by one octave
 */
void Note::decrement_octave()
{
  pitch-=12;
  octave_refcount--;
}

/*
 *	Transpose a note up by one octave
 */
void Note::increment_octave()
{
  pitch+=12;
  octave_refcount++;
}

/*
 *	Return a note to its original position
 */
void Note::rebalance_note()
{
  pitch += octave_refcount*12;
  octave_refcount = 0;
}

/*
 *	Rotate the note index forward: return to 0 if the note index exceeds the pitchmap entry
 */		
void Note::increment_note_index()
{
	int sizetemp = get_children_size();
	current_note_index = (current_note_index+1)%(sizetemp);
}

/*
 *	Get the number of fret positions for this note. Try 10 different octave offsets
 */
int Note::get_children_size() const
{
	int ret,n=0;
	if(pitch<0) return 0;
	int i = 0;
	while(++i<10) {
		try {
			ret = pitch_to_frets_map.at(pitch+n).size();
			return ret;
		}
		catch (const std::out_of_range& oor) {
			if(pitch<tuningMinimum){n +=12;}
			else{ n -=12;	}
		}
	}
}

/*
 *	Get the number of deleted notes (notes whose pitch was not mapped)
 */	
int Note::get_noteslost() const 
{
	return noteslost;
}

/*
 *	Set note index for note fret+string positions
 */
void Note::set_note_index(int n) 
{
	current_note_index = n;
}

/*
 *	Get note index for note fret+string positions
 */
int Note::get_current_note_index() const 
{
	return current_note_index;
}
	
/*
 *	Get midi pitch value of note
 */	
int Note::get_pitch() const 
{
	return pitch;
}

/*
 *	Transpose note by given interval (in semitones)
 */
void Note::alter_pitch(int n)
{
	pitch +=n; 
}

/*
 *	Get note delta
 */
int Note::get_delta()  const
{
	return delta;
}

/*
 *	Visit note
 */
void Note::accept(Visitor* v) 
{
	v->visitNote(this);
}


//At compile time: generate the pitch to fret mapping
Note::PitchMap Note::pitch_to_frets_map = config();
int Note::noteslost = 0;

