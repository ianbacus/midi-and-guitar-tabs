import time, sys, os
sys.path.insert(0, './src/scripts/')

from collections import OrderedDict
from midi_writer import make as generate_pitch_delta
import subprocess


if __name__ == "__main__":


	#set the order of input parameters here. They are ordered from left to right based on how frequently they are estimated to be used
	inputParameters = OrderedDict(
	[	('fileName', False),	\
		('noteOffset', '0'),	\
		('columnsPerRow', '4'),	\
		('align', '1'),			\
		('startMeasure', '0'),	\
		('endMeasure', '-1'),	\
		('condition', 'True')
	])

	IntermediateFileToTabExecutable = './src/gen'


	try: 
		os.makedirs("./data/input_files")
		os.makedirs("./data/intermediates")
		os.makedirs("./data/tabs")
	except OSError:

		if not os.path.isdir("./data/intermediates"):
			raise

	#Use command line arguments for the first (argc-1) parameters 
	argCount = 1
	for key,parameter in inputParameters.items():
		try:
			inputParameters[key] = sys.argv[argCount]
			argCount += 1
		except:
			break

	try:
		fileName = inputParameters['fileName']
		if False==fileName:
			raise ValueError("Error: no file name entered. Enter a '.mid' file from data/input_files.")

		#Translate midi file into intermediate format with raw pitch/time information
		generate_pitch_delta(	newfile="data/intermediates/"+fileName+".txt",	\
								infile="data/input_files/"+fileName+".mid", 	\
								condition=inputParameters['condition'],			\
								note_offsets='(0,0,0,0,0,0)')

		#Invoke c++ tab optimizer to translate intermediate file to tabs
		spstring= [	IntermediateFileToTabExecutable,		\
				  	'data/intermediates/'+fileName+'.txt',	\
				  	'data/tabs/'+fileName+'.txt',			\
				  	inputParameters['noteOffset'],			\
					inputParameters['columnsPerRow'],		\
					inputParameters['startMeasure'],		\
					inputParameters['endMeasure'],			\
					inputParameters['align']				\
				  ]
		subprocess.call(spstring)

	except OSError as e:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError as e:
		print e.message#'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError as e:
		print 'Bad file name: "' + fileName + '.mid"'
	



