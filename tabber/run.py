import time, sys, os
sys.path.insert(0, './src/scripts/')

from collections import OrderedDict
from midi_writer import make as generate_pitch_delta
import subprocess


#set the order of input parameters here
inputParameters = OrderedDict([('fileName', 'None'),('noteOffset', '0'),('columnsPerRow', '4'),('align', '1'),('startMeasure', '0'),('endMeasure', '-1'),('condition', 'True')])

IntermediateFileToTabExecutable = './src/gen'


def join(seq, sep=','):
    return sep.join(str(i) for i in seq)



if __name__ == "__main__":
	try: 
		os.makedirs("./data/input_files")
		os.makedirs("./data/intermediates")
		os.makedirs("./data/tabs")
	except OSError:

		if not os.path.isdir("./data/intermediates"):
			raise
	try:

		#Use command line arguments for the first few arguments, then switch to defaults
		count = 1
		for key,parameter in inputParameters.items():
			try:
				inputParameters[key] = sys.argv[count]
				count += 1
			except:
				break
		
		#Translate midi file into intermediate format with raw pitch/time information
		generate_pitch_delta(newfile="data/intermediates/"+inputParameters['fileName']+".txt", \
		infile="data/input_files/"+inputParameters['fileName']+".mid",condition=inputParameters['condition'],note_offsets='(0,0,0,0,0,0)')

		#Invoke c++ tab optimizer to translate intermediate file to tabs
		spstring= [IntermediateFileToTabExecutable, 'data/intermediates/'+inputParameters['fileName']+'.txt', 'data/tabs/'+inputParameters['fileName']+'.txt',\
		inputParameters['noteOffset'], inputParameters['columnsPerRow'],inputParameters['startMeasure'],inputParameters['endMeasure'], inputParameters['align']]
		subprocess.call(spstring)

	except OSError:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError:
		print 'Bad file name: "' + fileName + '.mid"'
	



