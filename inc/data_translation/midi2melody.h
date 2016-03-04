#ifndef __MIDI2MELODY__
#define __MIDI2MELODY__
#include "Base.h"
#include "MidiFile.h"
#include "Options.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>
#include <iostream>     // std::ostream, std::cout, std::cin
#include <sstream>
#include <fstream>

using namespace std;
vector<Bar*> score_maker(std::string infile,int,int);
vector<Bar*> convertMidiFileToText (MidiFile& midifile, int, int);
void setTempo (MidiFile& midifile, int index, double& tempo);
void checkOptions (Options& opts, int argc, char** argv);
void example (void);
void usage (const char* command);


#endif
