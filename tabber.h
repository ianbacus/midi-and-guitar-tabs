//
//  tabber.h
//
//  Created by Ian Bacus on 3/5/15.
//
//

#ifndef __BASE__
#define __BASE__

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;

class Base
{
	public:
		void grid_init();
		Base();
		~Base();
		
	    std::map <int, vector< pair<int,int> * >  > pitch_to_frets_map;
		int tuning[6];
		int tab_matrix [6][20];
		
		//accessors
		
		//retrieve string and fret information from the fret map with modulo addressing (circular vector)
		int get_string(int,int) const; 
		int get_fret(int,int) const;
		int get_pitch_to_frets_entry_size(int pitch) const;
		
		//int set_tuning(string);
		
		virtual void reconfigure(Base*);
		//virtual bool compare(Base*);
		
};

#include "note.h"
#include "bar.h"
#include "chunk.h"


#endif

