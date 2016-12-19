


import time, sys, os

from midi_writer import make as generate_pitch_delta
import subprocess

def join(seq, sep=','):
    return sep.join(str(i) for i in seq)



if __name__ == "__main__":
	try: 
		os.makedirs("./data/intermediates")
		os.makedirs("./data/tabs")
	except OSError:
		if not os.path.isdir("./data/intermediates"):
			raise
	try:
		
		if len(sys.argv) >= 8:
			print sys.argv[7]
			condition = sys.argv[7]
		else:
			condition = "True"
		if len(sys.argv) >= 7:
			align = sys.argv[6]
		else:
			align = "2"
		if len(sys.argv) >= 6:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			
			#midiwriter (midi to intermediate: extract note start and stop events)
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", \
			infile="data/input_files/"+fileName+".mid",condition=condition,note_offsets='(000,0,0,00,0,00)')
			
			#rotation_tabber
				#midi2melody (intermediate file converted to composite tree of score)
				#rotate visitor (composite tree re-arranged internally based on fretting rules)
				#print visitor (renders output in various formats: ascii, scrolling ui, piano tablature)
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', 'data/tabs/'+fileName+'.txt',\
			noteOffset, columnsPerRow, startMeasure, endMeasure, align]
			print ' '.join(spstring)
			subprocess.call(spstring)
			
		else:
			raise ValueError
	except OSError:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
	except IOError:
		print 'Bad file name: "' + fileName + '.mid"'
	







'''
import time, sys, os

from midi_writer import make as generate_pitch_delta
import subprocess

def join(seq, sep=','):
    return sep.join(str(i) for i in seq)



if __name__ == "__main__":
	try: 
		os.makedirs("./data/intermediates")
	except OSError:
		if not os.path.isdir("./data/intermediates"):
			raise
	try:
		align = '1';
		note_offsets = '(0,0,0,0,0,0)';
		if len(sys.argv) > 7:
			note_offsets = sys.argv[6]
		if len(sys.argv) >= 6:
			fileName = sys.argv[1]
			noteOffset = sys.argv[2]
			columnsPerRow = sys.argv[3]
			startMeasure = sys.argv[4]
			endMeasure = sys.argv[5]
			
			
			#midiwriter (midi to intermediate: extract note start and stop events)
			generate_pitch_delta(newfile="data/intermediates/"+fileName+".txt", \
			infile="data/input_files/"+fileName+".mid",condition="True",note_offsets=note_offsets)
			
			#rotation_tabber
				#midi2melody (intermediate file converted to composite tree of score)
				#rotate visitor (composite tree re-arranged internally based on fretting rules)
				#print visitor (renders output in various formats: ascii, scrolling ui, piano tablature)
			spstring= ['./gen', 'data/intermediates/'+fileName+'.txt', 'data/outTab.txt',\
			noteOffset, columnsPerRow, startMeasure, endMeasure, align]
			print ' '.join(spstring)
			subprocess.call(spstring)
			
		else:
			raise ValueError
	except OSError:
		print 'No executable found. Build the midi translator first (type "make")'
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
		print "filename, transpose, num_columns, start, end, (optional) track transpose"
	except IOError:
		print 'Bad file name: "' + fileName + '.mid"'
'''
