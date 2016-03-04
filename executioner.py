import time, sys

from midi_writer import make as generate_pitch_delta
import subprocess

if __name__ == "__main__":
	'''
	take a midi file name, generate a pitch_delta list (call the midi_writer module)
		- filter tracks, use the midi library track metadata information to accomplish this
		- modify individual track pitch offsets

	pass this file through the c++ module to parse the midi, translate to tabs, create a data structure to store the result
		- modify global offset of notes
		- modify display: bars per row

	print this tab into the TK gui window to autoscroll through the contents at a pre-set tempo

	'''
	try:
		if len(sys.argv) == 6:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			print endMeasure
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", infile="data/input_files/"+fileName+".mid",condition="True")
			
			spstring= ['./a', 'data/intermediates/'+fileName+'.txt', noteOffset, columnsPerRow, startMeasure, endMeasure]
			
			#note_offset, columns/row, start_measure, end_measure(-1=max)
			subprocess.call(spstring)

		else:
			raise ValueError
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'

