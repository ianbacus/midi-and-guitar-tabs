# midi


Small personal project for converting midi files into tablature. Utilizes a composite pattern hierarchy for creating a tree structure for the notes (individual notes belong to chunks of notes, which belong to bars). Designed a MIDI parser from scratch, but currently testing is being done with code from an existing MIDI parser library from elsewhere.

A visitor pattern is used to traverse this tree and perform operations. Currently, there are only reconfigure and print options. The reconfigure option is such an essential part of the program that the code might have to be reworked. The print option is intended to recursively print out specific elements based on their note and delta (time) values.

The reconfigure operation is necessary for the print function to operate correctly because it guarantees that there are no string overlaps.
Reconfigure TODOS:
 - Use tuning as a class parameter for the visitor, generate more "playable" tablature by generating a new tuning and testing it. Playability could be assessed by average fret spacing between notes and note clusters. 
 - Or conversely, the goal could be to generate a tab that guarantees the greatest amount of sustain (with note ons and note offs, or for notes that aren't intended to be sustained). This might require subclasses to the reconfigure option based on what to reconfigure by
 

Print TODOS:
 - guarantee alignment based on smallest note values, padding
