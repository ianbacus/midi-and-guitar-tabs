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
			
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", \
			infile="data/input_files/"+fileName+".mid",condition="True")
			
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', 'data/tabs/'+fileName+'.txt', \
			noteOffset, columnsPerRow, startMeasure, endMeasure]
			subprocess.call(spstring)
			
			#note_offset, columns/row, start_measure, end_measure(-1=max)
			

		else:
			raise ValueError
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError:
		print 'Bad file name: "' + fileName + '.mid"'
