import time, sys

from midi_writer import make as generate_pitch_delta
import subprocess

if __name__ == "__main__":
	try:
		if len(sys.argv) == 6:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			print endMeasure
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", infile="data/input_files/"+fileName+".mid",condition="True")
			
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', noteOffset, columnsPerRow, startMeasure, endMeasure]
			
			#note_offset, columns/row, start_measure, end_measure(-1=max)
			subprocess.call(spstring)

		else:
			raise ValueError
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'

