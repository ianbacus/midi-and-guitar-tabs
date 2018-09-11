'''
Take an input midi file and translate it into an intermediate file containing only time signature information, and a map of tick instants to pitch values
'''

import midi, collections, sys, os

note_offsets = (00,00,00,00,00,00,00,00,00,00,00,00)

#Limit the TickToPitchMidiValueDictionaryal resolution. This value also affects the scaling of time values in terms of beats
MaximumNotesPerBeat = 8.0

MaximumChunkSize = 6
RoundTo = 100

LowestNote = 0
MaximumNote = 0


def float_eq( a, b, eps=0.65):
    return abs(a - b) <= eps


#translate midi to intermediate format (list of musical events, newline separated)

activeNotesMappedToTheirStartTick = {}
timeSignatureEvents = {}
TickToPitchMidiValueDictionary = collections.OrderedDict()

def MakeActiveNoteKey(pitchMidiValueEntry):

    try:
        pitchMidiValue = pitchMidiValueEntry['pitch']
        currentTrackNumber = pitchMidiValueEntry['trackNumber']
        result = (pitchMidiValue)
    except:
        result = ()

    return result

def ProcessNote(midiEvent, currentEventTickValue,currentTrackNumber):
    pitchMidiValue = midiEvent.get_pitch()

    #Transpose the track individually
    #if(currentTrackNumber < len(note_offsets)):
        #pitchMidiValue += note_offsets[currentTrackNumber]

    #Note on events: add the pitch information to the intermediate file for rendering in the tab
    pitchMidiValueEntry = {'pitch':pitchMidiValue, 'trackNumber':currentTrackNumber,'duration':0}
    activeNoteKey = MakeActiveNoteKey(pitchMidiValueEntry)

    maybeNoteOff = (float_eq(midiEvent.get_velocity(),0.0))
    if (type(midiEvent) is midi.events.NoteOnEvent) and not maybeNoteOff:

        activeNotesMappedToTheirStartTick[activeNoteKey] = currentEventTickValue

        try:
            TickToPitchMidiValueDictionary[currentEventTickValue].append(pitchMidiValueEntry)

        except KeyError:
            TickToPitchMidiValueDictionary[currentEventTickValue] = [pitchMidiValueEntry]

    #Note off events: update the durations of notes as they expire
    elif (type(midiEvent) is midi.events.NoteOffEvent) or maybeNoteOff:

        try:
            tickValueOfActiveNote = activeNotesMappedToTheirStartTick[activeNoteKey]
            durationOfActiveNote = currentEventTickValue-tickValueOfActiveNote

            #print "note off %d to %d = %d" % (tickValueOfActiveNote, currentEventTickValue, durationOfActiveNote)

            for entryIndex, testPitchMidiValueEntry in enumerate(TickToPitchMidiValueDictionary[tickValueOfActiveNote]):
                testActiveNoteKey = MakeActiveNoteKey(testPitchMidiValueEntry)
                if testActiveNoteKey == activeNoteKey:
                    TickToPitchMidiValueDictionary[tickValueOfActiveNote][entryIndex]['duration'] = min(durationOfActiveNote,(MaximumNotesPerBeat*6))
        except:
            pass


def PreprocessTracks(midiTracks):

    deleteTracks = False
    trackNumberList = {i:0 for i in range(len(midiTracks))}

    sumOfAverages = 0
    maxNoteForAllTracks = 0
    minNoteForAllTracks = 200

    #preprocess tracks, sort by their average pitch value
    for currentTrackNumber, track in enumerate(midiTracks):
        numberOfNotes = 0
        noteSum = 0
        highestPitchMidiValue = 0
        lowestPitchMidiValue = 200

        #Get the average note information for this track
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

            lowestPitchMidiValue = min(lowestPitchMidiValue,minNoteForAllTracks)
            highestPitchMidiValue = max(highestPitchMidiValue,maxNoteForAllTracks)

    sortedTracks = sorted(trackNumberList, key=trackNumberList.get)
    sortedAverages = [trackNumberList[trackIndex] for trackIndex in sortedTracks]

    if(deleteTracks):
        while(len(sortedTracks) > maximumNumberOfTracks):
            middleIndex = len(sortedTracks)/2
            sortedTracks.pop(middleIndex)

    averageOfAllTracks = sum(sortedAverages)/len(sortedAverages)
    return minNoteForAllTracks,maxNoteForAllTracks,averageOfAllTracks



def BuildTickToPitchMidiValueDictionary(midiTracks,maximumNumberOfTracks):

    reservedValueForAutomaticFiltering = 1337

    pulsesPerQuarterNote = float(midiTracks.resolution)
    print "pulsesPerQuarterNote is %s" % pulsesPerQuarterNote

    resolution = pulsesPerQuarterNote

    minNoteForAllTracks,maxNoteForAllTracks,averageOfAllTracks = PreprocessTracks(midiTracks)

    if (maximumNumberOfTracks > 0):
        global MaximumChunkSize
        MaximumChunkSize = maximumNumberOfTracks

    elif maximumNumberOfTracks == reservedValueForAutomaticFiltering:
        RemoveMiddleMidiTracks()

    for currentTrackNumber,track in enumerate(midiTracks):
        if currentTrackNumber not in [0]:
            pass
        currentEventTickValue = 0
        track.make_ticks_abs()
        for midiEvent in track:
            initialValue = midiEvent.tick

            nextTickValue =  2*(midiEvent.tick*(MaximumNotesPerBeat/float(midiTracks.resolution)))

            currentEventTickValue = round(nextTickValue)

            #Time signature events are added to a separate data structure
            if type(midiEvent) is midi.events.TimeSignatureEvent:
                timeSignatureEvents[currentEventTickValue] = midiEvent.get_numerator(),midiEvent.get_denominator()


            if type(midiEvent) in [midi.events.NoteOnEvent,midi.events.NoteOffEvent]:
                ProcessNote(midiEvent, currentEventTickValue,currentTrackNumber)

        #end of track
    #end of all tracks
    return averageOfAllTracks


def HandleSpecialDeltaValues(delta):

    #replace triplet note deltas with -1 so that they can be treated specially
    beatValues = [1,2,4,8,16,32]

    alignedTriplets = [x*(2.0/3.0) for x in [1,4,16]]
    unalignedTriplets = [x*(4.0/3.0) for x in [1,4,16]]

    if(delta < 1):
        if (float_eq(round(newDelta,3),0.666)):
            delta = 'help me'
        else:
            delta = 0

        '''

        newDelta = ""
        if True in [float_eq(delta,tripletCase) for tripletCase in alignedTriplets]:
            newDelta = "-1"

        elif True in [float_eq(delta,tripletCase) for tripletCase in unalignedTriplets]:
            newDelta = ((-2.0*delta)-1)*(2.0/3.0)

        else:
            newDelta = str(round(delta))

        '''
    return delta

def ProcessChunk(chunk,chunkDuration,iof):
    processedChunk = []
    beatUnit = 0;
    chunkDelta = HandleSpecialDeltaValues(chunkDuration)

    #Remove middle voices if the chunk is too large
    sortedChunk = sorted(chunk, key=lambda k: k['pitch'])
    sortedChunkWithRemovedDuplicates = []

    for note in sortedChunk:
        if not any(n['pitch'] == note['pitch'] for n in sortedChunkWithRemovedDuplicates):
            sortedChunkWithRemovedDuplicates.append(note)


    while len(sortedChunkWithRemovedDuplicates) > MaximumChunkSize:
        popIndex = len(sortedChunkWithRemovedDuplicates)/2
        sortedChunkWithRemovedDuplicates.pop(popIndex)

    #Extract the data for each note
    for event in sortedChunkWithRemovedDuplicates:

        noteMidiPitch = event['pitch']
        noteDuration = event['duration']
        noteTrackNumber = event['trackNumber']

        noteEventData = "%d,%d,%d,%d" % (noteMidiPitch,chunkDelta,noteTrackNumber,noteDuration)
        verboseNoteEventData = "P:%d,CD:%d,D:%d" % (noteMidiPitch,chunkDelta,noteDuration)

        processedChunk.append(noteEventData)
        iof.write(noteEventData+'\n')

        #set delta to 0 for subsequent notes in the same chunk
        chunkDelta = 0

    #end of chunk iteration



    return processedChunk

def WriteExtractedMidiDataToIntermediateFile(newfile):
    with open(newfile, 'w') as intermediateOutputFile:

        #Get a sorted list of all tick instances where midi events occur
        sortedMidiEventTicksList = sorted(TickToPitchMidiValueDictionary.iterkeys())

        #process each instant in order
        for tick in sortedMidiEventTicksList:

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
                chunkDuration = nextTick - tick

            #Catch the last note case
            except:
                chunkDuration = 4

            chunk = TickToPitchMidiValueDictionary[tick]
            processedChunk = ProcessChunk(chunk,chunkDuration,intermediateOutputFile)
            for noteEventData in processedChunk:
                pass#intermediateOutputFile.write(noteEventData+'\n')

        #end of tick list iteration

    #end of file writing

def generate_pitch_delta(newfile,infile,maximumNumberOfTracks, notesPerBeat='8.0'):

    global MaximumNotesPerBeat

    MaximumNotesPerBeat = float(notesPerBeat)

    #Read midi file, translate it into track objects
    midiTracks = midi.read_midifile(infile)

    #Extract useful information (time signature changes, note on and off events) into a dictionary
    averagePitch = BuildTickToPitchMidiValueDictionary(midiTracks,int(maximumNumberOfTracks))

    #Write the output to an intermediate file
    WriteExtractedMidiDataToIntermediateFile(newfile)

    return averagePitch
