#!/bin/bash

function tabify(){
  python midi_writer.py $1 $2
  ./a pitch_deltas/$1.txt 24 3 0 
}
