# midi

Use:  python tk_project.py <midi filename, no extension> <negative note offset>

Midi parser library doesn't support python 3, need to fix that still
For testing purposes, this project searches a directory called "midi_files" and writes extracted information into a directory called "pitch_deltas"
The midi_writer.py module assumes you have a midi library with unique names, and generates a parseable file for the c++ module with a matching name (and a .txt extension)

Right now tabs are written to a "testoutput.txt" file. Parsing can be done with a pitch_delta.txt file and the c++ module, but the tk_project will run all of the commands for you.


Small personal project for converting midi files into tablature. Utilizes a composite pattern hierarchy for creating a tree structure for the notes (individual notes belong to chunks of notes, which belong to bars). 
Designed a MIDI parser from scratch, but currently testing is being done with code from an existing MIDI parser library from elsewhere.
(update) using python module to parse midi files



General:

Composite tree is rooted by a "score" object containing a vector of bars, to chunks, to notes etc

Score
	Bar 1
		Chunk 1
			Note 1
				delta
				note
			Note 2
				delta
				note
		Chunk 2
			Note 1
				delta
				note
			Note 2
				delta
				note
			Note 3
				delta
				note
		Chunk 3
			Note 1
	Bar 2
		Chunk 1
			Note 1
				delta
				note
			Note 2
				delta
				note
		Chunk 2
			Note 1
				delta
				note
			Note 2
				delta
				note
			Note 3
				delta
				note
		Chunk 3
			Note 1
			


Extra notes:

A visitor pattern is used to traverse this tree and perform operations. Currently, there are only reconfigure and print options. The reconfigure option is such an essential part of the program that the code might have to be reworked. The print option is intended to recursively print out specific elements based on their note and delta (time) values.

The reconfigure operation is necessary for the print function to operate correctly because it guarantees that there are no string overlaps.
Reconfigure TODOS:
 - Use tuning as a class parameter for the visitor, generate more "playable" tablature by generating a new tuning and testing it. Playability could be assessed by average fret spacing between notes and note clusters. 
 - Or conversely, the goal could be to generate a tab that guarantees the greatest amount of sustain (with note ons and note offs, or for notes that aren't intended to be sustained). This might require subclasses to the reconfigure option based on what to reconfigure by
 

Print TODOS:
 - guarantee alignment based on smallest note values, padding
