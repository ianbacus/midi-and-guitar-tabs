import time, sys, os
sys.path.insert(0, './src/scripts/')

from collections import OrderedDict
from midi_writer import make as generate_pitch_delta
import subprocess

#set the order of input parameters here. They are ordered from left to right based on how frequently they are estimated to be used
inputParameters = OrderedDict(
[	('fileName', False),		\
	('noteOffset', '0'),		\
	('align', '0'),			\
	('maximumNumTracks', '-1'), 	\
	('startMeasure', '0'),		\
	('endMeasure', '-1'),		\
	('condition', 'True')
])


def ParseCommandLineArguments():
	#Use command line arguments for the first (argc-1) parameters 
	argumentWasSpecifiedDictionary = {}	
	argCount = 1
	
	for key,parameter in inputParameters.items():
		try:
			inputParameters[key] = sys.argv[argCount]
			argCount += 1
			argumentWasSpecifiedDictionary[key] = True
		except:
			argumentWasSpecifiedDictionary[key] = False
	
	return argumentWasSpecifiedDictionary


if __name__ == "__main__":

	

	IntermediateFileToTabExecutable = './src/gen'

	try: 
		os.makedirs("./data/intermediates")
		os.makedirs("./data/input_files")
		os.makedirs("./data/tabs")

	except OSError:
		pass

	argumentWasSpecifiedDictionary = ParseCommandLineArguments()

	try:
		#Get first two arguments

		fileName = inputParameters['fileName']

		if False==fileName:
			raise ValueError("Error: no file name entered. Enter a '.mid' file from data/input_files.")
		parsedFileName = "data/parsed_midi_data.txt"

		#Translate midi file into intermediate format with raw pitch/time information
		averagePitch = generate_pitch_delta(newfile=parsedFileName,                        		\
						infile="data/input_files/"+fileName+".mid",                 \
						maximumNumberOfTracks=inputParameters['maximumNumTracks'],  \
						note_offsets='(0,0,0,0,0,0)')
		
		if (not argumentWasSpecifiedDictionary['noteOffset']) or (inputParameters['noteOffset'] == 'auto'):
			inputParameters['noteOffset'] = str(64 - averagePitch)
		else:
			print inputParameters['noteOffset']
		
		#Invoke c++ tab optimizer to translate intermediate file to tabs
		spstring= [	IntermediateFileToTabExecutable,		\
				  	parsedFileName,						\
				  	'data/tabs/'+fileName+'.txt',		\
				  	inputParameters['noteOffset'],		\
					inputParameters['startMeasure'],	\
					inputParameters['endMeasure'],		\
					inputParameters['align']			\
				  ]
		subprocess.call(spstring)

	except OSError as e:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError as e:
		print e.message#'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError as e:
		print 'Bad file name: "' + fileName + '.mid"'
	



