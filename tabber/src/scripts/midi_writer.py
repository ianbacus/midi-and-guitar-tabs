'''
Take an input midi file and translate it into an intermediate file containing only time signature information, and a map of tick instants to pitch values
'''

import midi, collections, sys, os
from collections import Counter

note_offsets = (00,00,00,00,00,00,00,00,00,00,00,00,00,00)

#Limit the TickToPitchMidiValueDictionaryal resolution. This value also affects the scaling of time values in terms of beats
MaximumNotesPerBeat = 8.0

lowest = 0
maxNote = 0
def float_eq( a, b, eps=0.0001 ):
    return abs(a - b) <= eps
    

#translate midi to intermediate format (list of musical events, newline separated)


instlist = {}
TickToPitchMidiValueDictionary = collections.OrderedDict()
		

def BuildTickToPitchMidiValueDictionary(midiTracks,trackFilterCondition):
	
	#deltaResolution = MaximumNotesPerBeat/midiTracks.resolution
	active_notes = {}
	#print "delta res = ", deltaResolution 
	for currentTrackNumber,track in enumerate(midiTracks):
		if eval(trackFilterCondition): 
			trackname = ""

			lowestPitchMidiValue = 200
			highestPitchMidiValue = 0
			instant = 0

			numberOfNotesInCurrentTrack = 0
			note_sum =1

			for midiEvent in track:					
				if type(midiEvent) is midi.events.TimeSignatureEvent:
					instlist[instant] = midiEvent.get_numerator(),midiEvent.get_denominator()
				if type(midiEvent) is midi.events.TrackNameEvent:
					trackname = midiEvent.text

				
				try:
					#instant += (midiEvent.tick*deltaResolution)
					instant += ((midiEvent.tick*MaximumNotesPerBeat)/midiTracks.resolution)
				except:
					print type(midiEvent),'midiEvent has no tick',dir(midiEvent)

				#Note events: update the ticks of the pitch-delta map, add in note on event pitches at the instant
				if type(midiEvent) in [midi.events.NoteOnEvent,midi.events.NoteOffEvent]:
					pitchMidiValue = midiEvent.get_pitch()

					#Transpose the track individually
					if(currentTrackNumber < len(note_offsets)):
						pitchMidiValue += note_offsets[currentTrackNumber]

					#Note on events: add the pitch information to the intermediate file for rendering in the tab	
					if (type(midiEvent) is midi.events.NoteOnEvent) and not (float_eq(midiEvent.get_velocity(),0.0)):
						active_notes[pitchMidiValue] = [True, instant]
						
						#Track info: Determine if this pitch surpasses the current minimum or maximum value
						lowestPitchMidiValue = min(lowestPitchMidiValue,pitchMidiValue)
						highestPitchMidiValue = max(highestPitchMidiValue,pitchMidiValue)
						note_sum+=pitchMidiValue
						numberOfNotesInCurrentTrack +=1
						

						try:
							TickToPitchMidiValueDictionary[instant].append([pitchMidiValue,currentTrackNumber])
						except KeyError:
							TickToPitchMidiValueDictionary[instant] = [[pitchMidiValue,currentTrackNumber]]


					#Note off events: update the note durations of notes as they expire
					elif (type(midiEvent) is midi.events.NoteOffEvent):
						'''
						Determine at which instant the pitchMidiValue started:
						- assume that a pitchMidiValue cannot overlap with itself - it must end before starting again
						 . a dictionary of pitchMidiValue:state can be maintained. for state=on, an instant index can be stored
						'''
						last_instant = active_notes[pitchMidiValue][1]
						active_notes[pitchMidiValue][0] = False

						#iterate through list of lists, find one whose pitchMidiValue matches, modify its third member
						for count,item in enumerate(TickToPitchMidiValueDictionary[last_instant]):
							try:
								if item[0] == pitchMidiValue:
						   			TickToPitchMidiValueDictionary[last_instant][count][2] = (instant-last_instant) #duration
									print "l85 suc"
							except:
								pass#print "l85 fails"
def print_intermediate_file(newfile):
	
	lost_notes = 0
	prev_instant = 0

	with open(newfile, 'w') as intermediateOutputFile:
	
		sorted_times = sorted(TickToPitchMidiValueDictionary.iterkeys())

		for tick_index in range(len(sorted_times)):
			tick = sorted_times[tick_index]
			
			#Add time signature events to output file
			for i,pair in instlist.items():
				if i == tick and i != 0:
					intermediateOutputFile.write("SIGEVENT\n")
					intermediateOutputFile.write( (str(pair[0]) + ',' + (str(pair[1]))+ '\n' ))

			chunk = TickToPitchMidiValueDictionary[tick]
			chunk = list(set([tuple(uniqueNote) for uniqueNote in chunk]))
			chunk.sort()

			while len(chunk) > 4:
				lost_notes += 1
				chunk.pop(len(chunk)/2)

			try:
				delta = sorted_times[(tick_index+1)] - tick
			except:
				delta = 4

			for note,track_num in chunk:
				#write the notes to a file
				currentLineOfIntermediateFile = str(note) + ','

				#replace tuple note deltas with -1 so that they can be padded in the C++ module
				if float_eq(delta,MaximumNotesPerBeat/12.0):
					currentLineOfIntermediateFile+="-1"
				else:
					currentLineOfIntermediateFile+= str(round(delta))

				currentLineOfIntermediateFile+= ','+str(track_num)
				intermediateOutputFile.write(currentLineOfIntermediateFile+'\n')
				delta=0
	

def make(newfile,infile,condition,note_offsets):
	'''
	Converts a midi file to an intermediate plaintext list of triples (note, duration, track no.), writes them to a file.
	Each triple represents a "note press," deltas indicate absolute "ticks," 0 and -1 are reserved for chords (0) and triplets (-1) for deltas
	Notes values are the same as midi pitch values. Track numbers are preserved to allow track-specific filtering in other applications
	
	'''
	
	midiTracks = midi.read_midifile(infile)
	
	BuildTickToPitchMidiValueDictionary(midiTracks,condition)
	print_intermediate_file(newfile)

if __name__ == "__main__":

	if len(sys.argv) == 1:

		#default to mass translate option: generate an intermediate file for all input files
		for file in os.listdir('data/input_files'):
			try:
				make(newfile="data/intermediates/"+fileName+".txt",infile="data/input_files/"+fileName+".mid",condition="True")
			except:
				print "error parsing " + str(fileName)

	elif len(sys.argv) == 3:

		condition = sys.argv[2]
		file_name = sys.argv[1]

		make(newfile="data/intermediates/"+fileName+".txt", infile="data/input_files/"+fileName+".mid",condition=condition)

