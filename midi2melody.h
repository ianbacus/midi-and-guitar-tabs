#ifndef __MIDI2MELODY__
#define __MIDI2MELODY__
#include "MidiFile.h"
#include "Options.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>


void convertMidiFileToText (MidiFile& midifile);
void setTempo (MidiFile& midifile, int index, double& tempo);
void checkOptions (Options& opts, int argc, char** argv);
void example (void);
void usage (const char* command);


#endif
