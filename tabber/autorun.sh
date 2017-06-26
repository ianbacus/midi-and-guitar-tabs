#!/bin/bash
#This script will create a tab for all files in the input_data directory
#that contain the input substring.

# sh autorun.sh folder/

for file in ./data/input_files/*$1*
do 
	foo=${file#./data/input_files/}
	python2.7 run.py "${foo%.mid}" 
done
