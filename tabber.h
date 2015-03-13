//
//  tabber.h
//
//  Created by Ian Bacus on 3/5/15.
//
//

#ifndef __tabber__
#define __tabber__

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;
#endif /* defined(____tabgenerator__) */

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
		size_t get_pitch_to_frets_entry_size(int pitch) const;
		
		//int set_tuning(string);
		
		virtual void reconfigure(Base*);
		//virtual bool compare(Base*);
		
};


class Note : public Base 
{
	public:
		Note(int);
		int pitch;
		int current_note_index;
		

		int get_fret() const;
		int get_string() const;
		void increment_note_index();
		//int get_current_note_index const {return current_note_index;}
		
		virtual bool compare(Note*) const;
	
//		void set_fret_string();
	

};


//implement as doubly linked list
class Chunk : public Base 
{
	public:
		int delta;
		vector<Note*> Chunk_Notes;
	
		void add_Note(Note*); //this should automatically enforce good fingering with the tuning matrix
		void rotate();
		vector<Note*> * get_chunk_notes_vector() {return &Chunk_Notes;}
		
		virtual void reconfigure(Base*);
		virtual bool compare_chunks(Chunk*);
	
	
};

class Bar
//linked list implementation might make most sense here
{
	public:
		vector<Chunk*> Bar_Chunks; //Chunks are children of Bar	

		void add_Chunk(Chunk*);
		//operator [] for accessing Chunks
		
		
		void reconfigure(Base*);
};



