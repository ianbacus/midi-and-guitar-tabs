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
		if len(sys.argv) == 7:
			print sys.argv[5]
			condition = sys.argv[6]
		else:
			condition = "True"
		if len(sys.argv) in [6,7]:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", \
			infile="data/input_files/"+fileName+".mid",condition=condition)
			
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', 'data/outTab.txt',\
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
	
