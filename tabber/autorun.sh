#!/bin/bash
#This script will create a tab for all files in the input_data directory
#that contain the input substring.

# sh autorun.sh folder/

for file in ./data/input_files/*$1*
do 
	foo=${file#./data/input_files/}
	python run.py "${foo%.mid}" $2 $3 $4 $5 $6 $7 $8 $9 $a 
done
