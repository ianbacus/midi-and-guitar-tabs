import time, sys, os

from midi_writer import make as generate_pitch_delta
import subprocess

if __name__ == "__main__":
	try: 
		os.makedirs("./data/intermediates")
	except OSError:
		if not os.path.isdir("./data/intermediates"):
			raise
	try:
		note_offsets = '(0,0,0,0,0,0)';
		if len(sys.argv) > 7:
			note_offsets = endMeasure = sys.argv[6]
		if len(sys.argv) >= 6:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", \
			infile="data/input_files/"+fileName+".mid",condition="True",note_offsets=note_offsets)
			
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', 'data/tabs/'+fileName+'.txt', \
			noteOffset, columnsPerRow, startMeasure, endMeasure]
			subprocess.call(spstring)
			
		else:
			raise ValueError
	except OSError:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError:
		print 'Bad file name: "' + fileName + '.mid"'
	