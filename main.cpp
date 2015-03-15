#include "Reader.h"
#include "MidiFile.h"
#include "Options.h"
#include "midi2melody.h"
#include "Visitor.h"
#include "Printvisitor.h"
#include "RotateVisitor.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>




using namespace std;
/*
void config()
{
int tuning[6];
//int tab_matrix [6][20];
    int value;
    tuning[0] = 28;  //E
    tuning[1] = 33; //A
    tuning[2] = 39; //D
    tuning[3] = 43; //G
    tuning[4] = 47; //B
    tuning[5] = 52; //E
     for(int string_ind = 0;string_ind<5;string_ind++)
    {
        for(int fret_ind = 0; fret_ind<20; fret_ind++)
        {
            {
                value = tuning[string_ind] + fret_ind;
                //cout << "gridmap: inserting " << value << " at coordinate point " << string_ind <<", " << fret_ind << endl;
                //tab_matrix[string_ind][fret_ind] = value;
                pair<int,int> map_point = make_pair(string_ind,fret_ind);
                Base::pitch_to_frets_map[value].push_back(&map_point); //add note to (note : location on fretboard) pitch_to_frets. this will help for determining how many placements there are for a note, and quickly indexing them (is this any faster than indexing the array? TODO)
            }
        }
    }
}


struct Note {
      double tick;
      double duration;
      int    pitch;
      void print_data() {cout << pitch << " for " << duration << ". Tick=" << tick << endl;}
};


void read_chunk(Reader &ro)
{
  
	string type;
	string mtrk = "MTrk";
	string mthd = "MThd";
	
	ro.read_bytes_to_char(4,type);
	cout << type << endl;
//	if(type.compare(mthd) == 0)
	if(type == "MThd")
	{
	cout << "reading header chunk" << endl;
		ro.read_header_chunk();
	}
//	else if(type.compare(mtrk) == 0)
	else if(type == "MTrk")
	{
	cout << "reading track chunk" << endl;
		ro.read_track_chunk();
	}
	else
	{
	  cout << "fuckup" << endl;
	}

}


void read_file(Reader &ro)
{
	unsigned int track_cnt = 0;
	read_chunk(ro);
	while(track_cnt < 2) //ro.get_tracks())
	{
		read_chunk(ro);
		track_cnt++;
	}
}*/
//////////////////////////////
//
// convertToMelody --
//
/*
void convertToMelody(MidiFile& midifile, vector<Note>& melody) {
   int track = 0;
   midifile.joinTracks();
   
   midifile.absoluteTicks();
   if (track < 0 || track >= midifile.getNumTracks()) {
      cout << "Invalid track: " << track << " Maximum track is: "
           << midifile.getNumTracks() - 1 << endl;
   }
   int numEvents = midifile.getNumEvents(track);

   vector<int> state(128);   // for keeping track of the note states

   int i;
   for (i=0; i<128; i++) {
      state[i] = -1;
   }

   melody.reserve(numEvents);
   melody.clear();

   Note mtemp;
   int command;
   int pitch;
   int velocity;

   for (i=0; i<numEvents; i++) {
      command = midifile[track][i][0] & 0xf0;
      if (command == 0x90) {
         pitch = midifile[track][i][1];
         velocity = midifile[track][i][2];
         if (velocity == 0) {
            // note off
            goto noteoff;
         } else {
            // note on
            state[pitch] = midifile[track][i].tick;
         }
      } else if (command == 0x80) {
         // note off
         pitch = midifile[track][i][1];
noteoff:
         if (state[pitch] == -1) {
            continue;
         }
         mtemp.tick = state[pitch];
         mtemp.duration = midifile[track][i].tick - state[pitch];
         mtemp.pitch = pitch;
         melody.push_back(mtemp);
         state[pitch] = -1;
      }
   }
}
void test()
{
		
  MidiFile midifile;
  vector<Note> piece;
  int tracks;
  string filename;

	while(1)
	{
	  int track=0;
	 // Reader robj(fn);
	  //read_file(robj);
	   filename = "";
	   cin >> filename;
	   midifile = MidiFile(filename);
	   tracks = midifile.getTrackCount();
	   //int track = 0;
	   cout << tracks << " tracks." << endl;
	   while(track < tracks)
	   {
	   	cout << "Track " << track << " has " << midifile.getNumEvents(track) << " events." << endl;
	   	track++;
	   }
	   convertToMelody(midifile,piece);
	   for(auto n : piece)
	   {
	   //	n.print_data();
	   }
	}
}*/

int main(int argc, char* argv[]) {
Options options;


   checkOptions(options, argc, argv);
   MidiFile midifile(options.getArg(1));
  // Base_structs refs = new Base_structs;
  // refs.config();
   vector<Bar*> score= convertMidiFileToText(midifile);
   
   cout << "Main: note size: " << score[0]->get_child(0)->get_note_at(0)->get_children_size() << endl;
   
   int pitch = score[0]->get_child(0)->get_note_at(2)->get_pitch() ;
   
   cout << "Main: note string: " << << endl;
   Note::PitchMap pmap = Note::pitch_to_frets_map;
   pair<int,int> * &pairy = pmap.at(pitch)[0];
   cout << "Main: note string: " << pairy->first << endl;
   cout << "Main: note fret: " << pairy->second << endl;
   
   cout << "Main: chunk size: " << score[0]->get_child(0)->get_children_size() << endl;
   cout << "Main: bar size: " << score[0]->get_children_size() << endl;

   
   RotateVisitor* thefixer = new RotateVisitor();
   PrintVisitor* theprinter = new PrintVisitor();
   
   score[0]->accept(thefixer);
   //score[0]->accept(theprinter);
   //theprinter->print_out();
   return 0;

	
}
