'''
Take an input midi file and translate it into an intermediate file containing only time signature information, and a map of tick instants to pitch values
'''

import midi, collections, sys, os
from collections import Counter

import json

note_offsets = (00,00,00,00,00,00,00,00,00,00,00,00)

#Limit the TickToPitchMidiValueDictionaryal resolution. This value also affects the scaling of time values in terms of beats
MaximumNotesPerBeat = 8.0

MaximumChunkSize = 6
RoundTo = 100

lowest = 0
maxNote = 0


def float_eq( a, b, eps=0.35):
    return abs(a - b) <= eps
    

#translate midi to intermediate format (list of musical events, newline separated)


timeSignatureEvents = {}
TickToPitchMidiValueDictionary = collections.OrderedDict()
		

def BuildTickToPitchMidiValueDictionary(midiTracks,maximumNumberOfTracks):
	
	active_notes = {}
        resolution = float(midiTracks.resolution)
	
	sortedTracks = range(len(midiTracks))
	trackNumberList = {i:0 for i in range(len(midiTracks))}
	
	if (maximumNumberOfTracks > 0):
		global MaximumChunkSize
		MaximumChunkSize = maximumNumberOfTracks
	elif (maximumNumberOfTracks == 1337):
		#preprocess tracks, sort by their average pitch value
		for currentTrackNumber, track in enumerate(midiTracks):
			numberOfNotes = 0
			noteSum = 0	
			highestPitchMidiValue = 0
			lowestPitchMidiValue = 200
			
			for midiEvent in track:
				if type(midiEvent) is midi.events.NoteOnEvent:
					pitchMidiValue = midiEvent.get_pitch()
	
					#Track info: Determine if this pitch surpasses the current minimum or maximum value
					lowestPitchMidiValue = min(lowestPitchMidiValue,pitchMidiValue)
					highestPitchMidiValue = max(highestPitchMidiValue,pitchMidiValue)
					noteSum += pitchMidiValue
					numberOfNotes += 1
			if(numberOfNotes == 0):
				del trackNumberList[currentTrackNumber]
			else:
				noteAverage = noteSum / numberOfNotes
				trackNumberList[currentTrackNumber] = noteAverage

		sortedTracks = sorted(trackNumberList, key=trackNumberList.get)
		print(str(sortedTracks) +" "+ str(len(sortedTracks)) +" "+ str(maximumNumberOfTracks))
		print(len(sortedTracks) + maximumNumberOfTracks)
		
		while(len(sortedTracks) > maximumNumberOfTracks):
			middleIndex = len(sortedTracks)/2
			sortedTracks.pop(middleIndex)

	print(sortedTracks)
		
	for currentTrackNumber,track in enumerate(midiTracks):
		#only process unfiltered tracks
		if currentTrackNumber not in sortedTracks:
			continue
		
		currentEventTickValue = 0
                track.make_ticks_abs()
                for midiEvent in track:	
                        initialValue = midiEvent.tick

			nextTickValue =  2*(midiEvent.tick*(MaximumNotesPerBeat/float(midiTracks.resolution)))

			currentEventTickValue = round(nextTickValue)

			#Time signature events are added to a separate data structure
			if type(midiEvent) is midi.events.TimeSignatureEvent:
				timeSignatureEvents[currentEventTickValue] = midiEvent.get_numerator(),midiEvent.get_denominator()

			#Note events: update the ticks of the pitch-delta map: delta:pitch
                        if type(midiEvent) in [midi.events.NoteOnEvent]:#,midi.events.NoteOffEvent]:
				pitchMidiValue = midiEvent.get_pitch()

				#Transpose the track individually
				if(currentTrackNumber < len(note_offsets)):
					pitchMidiValue += note_offsets[currentTrackNumber]

				#Note on events: add the pitch information to the intermediate file for rendering in the tab	
				if (type(midiEvent) is midi.events.NoteOnEvent) and not (float_eq(midiEvent.get_velocity(),0.0)):
			
                                        active_notes[pitchMidiValue] = currentEventTickValue
					
					pitchMidiValueEntry = {'pitch':pitchMidiValue, 'trackNumber':currentTrackNumber}

					try:
						TickToPitchMidiValueDictionary[currentEventTickValue].append(pitchMidiValueEntry)

					except KeyError:
						TickToPitchMidiValueDictionary[currentEventTickValue] = [pitchMidiValueEntry]

				#Note off events: update the durations of notes as they expire
				'''
				elif (type(midiEvent) is midi.events.NoteOffEvent):

					tickValueOfActiveNote = active_notes[pitchMidiValue]
					active_notes[pitchMidiValue]

					for count,item in enumerate(TickToPitchMidiValueDictionary[tickValueOfActiveNote]):
						try:
							if item[0] == pitchMidiValue:
								TickToPitchMidiValueDictionary[tickValueOfActiveNote][count][2] = 
								(currentEventTickValue-tickValueOfActiveNote) #duration
						except:
							pass
				'''
		#end of track
	#end of all tracks 

def WriteExtractedMidiDataToIntermediateFile(newfile):
	with open(newfile, 'w') as intermediateOutputFile:
	
		#Get a sorted list of all tick instances where midi events occur, then process each instant in order
		sortedMidiEventTicksList = sorted(TickToPitchMidiValueDictionary.iterkeys())
		for tick in sortedMidiEventTicksList:
			
			chunk = TickToPitchMidiValueDictionary[tick]
			beatUnit = 0;
	
			#Add time signature events to output file
			for timeSignatureTickInstant,timeSignature in timeSignatureEvents.items():
				if timeSignatureTickInstant == tick:
					
					beatsPerMeasure = str(timeSignature[0])
					beatUnit = str(timeSignature[1])
					intermediateOutputFile.write("SIGEVENT\n")
					intermediateOutputFile.write(beatsPerMeasure+','+beatUnit+'\n')  

			#Determine the number of ticks between the current event and the next one
			try:
				currentTickIndex = sortedMidiEventTicksList.index(tick)
				nextTick = sortedMidiEventTicksList[currentTickIndex+1]
				delta = nextTick - tick
			#Catch the last note case
			except:
				delta = 4

			while len(chunk) > MaximumChunkSize: 
				chunk.pop(len(chunk)/2)
			#write the event data to a file
			for event in chunk:

				note = event['pitch']
				trackNumber = event['trackNumber']
				
				currentLineOfIntermediateFile = str(note) + ','

				#replace triplet note deltas with -1 so that they can be treated specially

				beatValues = [1,2,4,8,16,32]

				alignedTriplets = [x*(2.0/3.0) for x in [1,4,16]]
				unalignedTriplets = [x*(4.0/3.0) for x in [1,4,16]]

				#DEBUGSTR = str(delta)
				
#				newDelta = str(round(delta,RoundTo))

				newDelta = delta

                                if(newDelta < 1):
                                    if (float_eq(round(newDelta,3),0.666)):
			    	        newDelta = -1
                                    else:
                                        newDelta = 0

				currentLineOfIntermediateFile += str(newDelta)
				'''
				
				newDelta = ""
				if True in [float_eq(delta,tripletCase) for tripletCase in alignedTriplets]:
					newDelta = "-1"
 
				elif True in [float_eq(delta,tripletCase) for tripletCase in unalignedTriplets]:
					newDelta = ((-2.0*delta)-1)*(2.0/3.0)
				
				else:
					newDelta = str(round(delta))

				'''
				#currentLineOfIntermediateFile += str(newDelta)
				#DEBUGSTR += ' '+ str(newDelta)
				currentLineOfIntermediateFile+= ','+str(trackNumber)
				intermediateOutputFile.write(currentLineOfIntermediateFile+'\n')

				#set delta to 0 for subsequent notes in the same chunk
				delta = 0

			#end of chunk iteration

		#end of tick list iteration

	#end of file writing

def make(newfile,infile,maximumNumberOfTracks, note_offsets):

	#Read midi file, translate it into track objects
	midiTracks = midi.read_midifile(infile)
	
	#Extract useful information (time signature changes, note on and off events) into a dictionary
	BuildTickToPitchMidiValueDictionary(midiTracks,int(maximumNumberOfTracks))

	#Write the output to an intermediate file
	WriteExtractedMidiDataToIntermediateFile(newfile)
