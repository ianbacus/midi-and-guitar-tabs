# guitar tablature generator

# What does this project do?

This project calculates optimized tablature arrangements for polyphonic midi tracks. 

Command line options can be used to transpose all tracks up or down by a specified number of semitones, multiply the time signature by a power of two to increase or decrease horizontal note density, and to limit translation to a range of measures in a given midi file. 


# How does it work?

The first step is optimization. The problem can be described as follows: 


Optimizing a chord by reconfiguring its notes:
-----------------------
A chord contains N notes. 
Each note has P possible configurations. 
The playability of two notes in a chord can be quantified as a cost. This playability cost is a function of fret spacing, string spacing, and average distance of each note from fret 0. 

One configuration must be selected from each note's set P, such that the cost of the resulting fully connected graph is minimized


Optimizing a measure by reconfiguring its chords:
-----------------------
Optimizing a measure by reconfiguring its chords:
Each chord has P possible configurations
The playability of two adjacent chords can be quantified as a cost. The playability cost here is directly proportional to the fret spacing between the mean fret of each chord. 

(Not implemented yet)

# Installation

Execute install.sh

# Execution

Any of the following work (any ommitted arguments, going from left to right, will be assigned default values if not specified):
python2.7 run.py "filename"
python2.7 run.py "filename" <semitone transposition>
python2.7 run.py "filename" <semitone transposition> <number of measures per row of tablature>
...
python2.7 run.py "filename" <semitone transposition> <number of measures per row of tablature> <number of beat units per measure> <start measure> <end measure>


Example: 

python2.7 run.py wtc1f01

output:
```
     e         e         e         e                T   T   e         e         e         e               
|e|----------------------------------------------|------------------------------------------------------|
|B|----------------------------------------------|------------------------------------------------------|
|G|----------------------------------------------|--0-----------------2-------------------0-------------|
|D|------------0---------2---------3-------------|------3---2-------------------0-----------------------|
|A|--3-------------------------------------------|------------------------------------------------------|
|E|----------------------------------------------|------------------------------------------------------|
|B|----------------------------------------------|------------------------------------------------------|

     s     s     s     s     s     s     s     s        s     s     s     e         s     T   T   e         
|e|--------------------------------------------------|------------------------------------------------------|
|B|--------------------------------------------------|--------0-----------1---------------3---1---0---------|
|G|--2-----0-----------------------0-----------2-----|--------------------------------------------0---------|
|D|--------------3-----2-----3-----2-----0-----------|--0---------------------------4-----------------------|
|A|--------------------------------------------3-----|--------3-----2-----0---------------------------------|
|E|--------------------------------------------------|------------------------------------------------------|
|B|--------------------------------------------------|------------------------------------------------------|
```
