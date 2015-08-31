#include "midi2melody.h"
#include <iomanip>

#include <stdlib.h>
#include <cmath>
using namespace std;

typedef unsigned char uchar;

// user interface variables

Options options;
int     debugQ = 0;             // use with --debug option
int     maxcount = 100000;
double  tempo = 60.0;

/*
// function declarations:
void      convertMidiFileToText (MidiFile& midifile);
void      setTempo              (MidiFile& midifile, int index, double& tempo);
void      checkOptions          (Options& opts, int argc, char** argv);
void      example               (void);
void      usage                 (const char* command);
*/
//////////////////////////////////////////////////////////////////////////

//////////////////////////////
//
// example --
//


vector<Bar*> score_maker(std::string infile, int shift,int align) {
	/* 
	  A beat overflow is calculated by multiplying the numerator of the time signature with
	  the mapped value of the denominator. This the denominator is mapped as 32/n, where n is the denominator.
	  
	  So really, the time signature ratio is multiplied by 32 to count how many 32nd notes would fit in a bar.
	  
	  4/4 time: 32 32nd notes .... 128 128th notes
	     four quarter notes : 6 quarter triplets
	     sixteen sixteenth notes : 16*3/2 = 24 sixteenth triplets
	  6/8 time: 24 32nd notes
	  
	  This is because 32nd notes are used as the most precise bits (highest resolution).
	  
	  Triplets are 2/3 of their normal note value, so in the time that 2 eighth notes pass 3 triple-eighth notes pass
	  There are no reasonably low numbers with common multiples that are compatible with triplets
	  
	  all that matters is that the beginning and end notes are aligned properly... 
	  
	  
	  handles input in this form
  	 	 line 0: 72,100
		 line 1: 72,0
	*/
    std::ifstream file( infile );
    int last;
    
    int beat_per_measure = 4;
    //how to account for triplets?
    map<int,int> beat_value = {{1,32},{2,16},{4,8},{8,4},{16,2},{32,1}};
    //map<int,int> beat_value = {{1,32},{2,16},{4,8},{8,4},{16,2},{32,1}};
    float bartime = 0;
    int counter = 0;
    int beat_overflow=36;
	vector<Bar*> score;
	
    score.push_back(new Bar());
    score.back()->add_chunk(new Chunk());
    std::string line;
    
    
	align = align*(beat_overflow/(2*(beat_per_measure)));
	bartime += align;
    while( std::getline( file, line ) ) {
      //get newline separated lines from file
      
        std::istringstream iss( line );
		if(line == "SIGEVENT"){
			std::getline(file, line);
			std::istringstream iss( line );
			std::string num,denom;
			if( std::getline( iss, num , ',') && std::getline( iss, denom )) {
				cout << num << " " << denom << endl;
				beat_overflow = ((std::stoi(num)) * (beat_value[std::stoi(denom)]) );
				cout << "BO is " << beat_overflow << endl;
			}
		}	
        std::string p,d;
		if( std::getline( iss, p , ',') && std::getline( iss, d )) {
			//delimiting character inside each line
			counter++;
			float delta = stof(d); 
			int pitch = stoi(p);
			//if(counter == 1) bartime += delta;
			if(counter == 1) cout << pitch << "," << delta << endl;
			bartime += abs(delta);
			if(bartime >= beat_overflow && delta != 0){
				//Case 1: the bar is full, create a new one with an empty initial chunk
				score.push_back(new Bar());	
			    score.back()->add_chunk(new Chunk());
				bartime = 0;
				
			}
			
			if(delta == 0){
			  //Case 2: this note must be added to the current chunk in the current bar
			  //score.back() returns the last bar
			  //score.back()->get_children_size() returns the number of chunks, used to index the last chunk
			  //since the member vectors are inaccessible, using the "back()" function on the vector is not viable
			  
			  last = score.back()->get_children_size() - 1;
			  score.back()->get_child(last)->add_note(new Note(pitch-shift,delta));
			}
			else{
			  //Case 3: a new chunk in the current bar is needed
			  
			  score.back()->add_chunk(new Chunk(delta));
			  last =  score.back()->get_children_size() - 1;
			  score.back()->get_child(last)->add_note(new Note(pitch-shift,delta));
			}	
			
			//...
		}
	}
	return score;

}
/*
void add_to_tree(int delta_start, int pitch)
{
   int delta_counter,last;
   queue<Bar*> score;
   score.push(new Bar());
   score.back().add_child(new Chunk());
   
   if(delta_start == 0)
   {
      //this note must be added to the current chunk
      last = score.back()->get_child()->get_children_size() - 1;
      score.back()->get_child(last)->add_child(new Note(pitch));
   }
   else
   {
      //a new chunk must be created, and the new note must be added to it
      score.back()->add_child(new Chunk());
      last =  score.back()->get_child()->get_children_size() - 1;
      score.back()->get_child(last)->add_child(new Note(pitch));
   }
}*/
//////////////////////////////////////////////////////////////////////////


//////////////////////////////
//
// convertMidiFileToText --
//
/*
vector<Bar*> convertMidiFileToText(MidiFile& midifile,int num, int denm) {
  
  
int truncate = denm;
int shift = 27;
   int last;
   double delta_counter;
   vector<Bar*> score;
   score.push_back(new Bar());
   score.back()->add_chunk(new Chunk()); //the first chunk has been instantiated inside the first bar
   
  
   midifile.deltaTicks();
   midifile.joinTracks();

   vector<double> ontimes(128);
   vector<int> onvelocities(128);
   int i;
   for (i=0; i<128; i++) {
      ontimes[i] = -1.0;
      onvelocities[i] = -1;
   }

   double offtime = 0.0;

   int key = 0;
   int vel = 0;
   int command = 0;
 cout << "Tempo is " << tempo << ", " << midifile.getTicksPerQuarterNote() << " ticks/Q (beat) " << endl;
cout << "actually there are " << midifile.getNumEvents(0) << " events, but I truncated down to " << truncate << endl;
  cout << "On time | offtime-ontime | key | velocity" << endl;
//  for (i=0; i<midifile.getNumEvents(0); i++) {
   for (i=0; i<truncate; i++) {

      command = midifile[0][i][0] & 0xf0;
      if (command == 0x90 && midifile[0][i][2] != 0) {
         // store note-on velocity and time
         key = midifile[0][i][1];
         vel = midifile[0][i][2];
         ontimes[key] = midifile[0][i].tick*num / midifile.getTicksPerQuarterNote();//* 240 / tempo; /// tempo; // /
             //  midifile.getTicksPerQuarterNote();
         onvelocities[key] = vel;
      } else if (command == 0x90 || command == 0x80) {
         // note off command write to output
         key = midifile[0][i][1];
         offtime = midifile[0][i].tick*num / midifile.getTicksPerQuarterNote();
         //* 60.0 / (midifile.getTicksPerQuarterNote() / tempo);
               
               long double delta_start = ontimes[key];///getTicksPerQuarterNote();
               int pitch = key;/*/
               
            /*
               cout << "extracted note: " << key << " d" << std::setprecision(9) << delta_start << endl;
               ///////////////////////////////////////////////////////////////
               if(delta_start == 0)
               {
                  //this note must be added to the current chunk
                  //score.back() returns the last bar
                  //score.back()->get_children_size() returns the number of chunks, used to index the last chunk
                  //since the member vectors are inaccessible, using the "back()" function on the vector is not viable
                  last = score.back()->get_children_size() - 1;
                  score.back()->get_child(last)->add_note(new Note(pitch-shift,delta_start));
               }
               else
               {
                  //a new chunk must be created, and the new note must be added to it
                  score.back()->add_chunk(new Chunk());
                  last =  score.back()->get_children_size() - 1;
                  score.back()->get_child(last)->add_note(new Note(pitch-shift,delta_start));
               }
               ///////////////////////////////////////////////////////////////

         cout << "note\t" << ontimes[key]
              << "\t" << offtime - ontimes[key]
              << "\t" << key << "\t" << endl;

         onvelocities[key] = -1;
         ontimes[key] = -1.0;
      }

      // check for tempo indication
      if (midifile[0][i][0] == 0xff &&
                 midifile[0][i][1] == 0x51) {
         setTempo(midifile, i, tempo);
      }
   }
return score;
}


//////////////////////////////
//
// setTempo -- set the current tempo
//


void setTempo(MidiFile& midifile, int index, double& tempo) {
   double newtempo = 0.0;
   static int count = 0;
   count++;

   MidiEvent& mididata = midifile[0][index];

   int microseconds = 0;
   microseconds = microseconds | (mididata[3] << 16);
   microseconds = microseconds | (mididata[4] << 8);
   microseconds = microseconds | (mididata[5] << 0);

   newtempo = 60.0 / microseconds * 1000000.0;
   if (count <= 1) {
      tempo = newtempo;
   } else if (tempo != newtempo) {
      cout << "; WARNING: change of tempo from " << tempo
           << " to " << newtempo << " ignored" << endl;
   }
}



//////////////////////////////
//
// checkOptions --
//

void checkOptions(Options& opts, int argc, char* argv[]) {
   opts.define("author=b",  "author of program");
   opts.define("version=b", "compilation info");
   opts.define("example=b", "example usages");
   opts.define("h|help=b",  "short description");

   opts.define("debug=b",  "debug mode to find errors in input file");
   opts.define("max=i:100000", "maximum number of notes expected in input");

   opts.process(argc, argv);

   // handle basic options:
   if (opts.getBoolean("author")) {
      cout << "Written by Craig Stuart Sapp, "
           << "craig@ccrma.stanford.edu, 22 Jan 2002" << endl;
      exit(0);
   } else if (opts.getBoolean("version")) {
      cout << argv[0] << ", version: 22 Jan 2002" << endl;
      cout << "compiled: " << __DATE__ << endl;
      exit(0);
   } else if (opts.getBoolean("help")) {
      usage(opts.getCommand().data());
      exit(0);
   } else if (opts.getBoolean("example")) {
      example();
      exit(0);
   }

   debugQ = opts.getBoolean("debug");
   maxcount = opts.getInteger("max");

   if (opts.getArgCount() != 1) {
      usage(opts.getCommand().data());
      exit(1);
   }

}
*/


/*

{

                std::string token;
                while( std::getline( iss, token, ',' ) )
                {
                    std::cout << token << std::endl;
                }
*/

//////////////////////////////
//
// usage --
//
/*
void usage(const char* command) {
   cout << "Usage: " << command << " midifile" << endl;
}*/
