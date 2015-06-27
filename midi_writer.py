import midi, collections, sys
#from sortedcontainers import SortedDict

DELTA_VAL = 8


def make(newfile,infile):
	'''
	Uses the midi parser provided by Vishnubob's github project
	
	DELTA_VAL is extremely important for the c++ tab generator. It determines how spacing is interpreted
	when rendering the tab. 
		Setting it to 8 sets the "instant tick" value for any 32nd note to 1. This instant tick amount
		will be relative to the resolution of the midi file itself. It will be used to logarithmically
		describe the number of padding dashes to put between notes.
		
		16 - 64th note resolution
		8 - 32nd note resolution
		4 - 16th note resolution
		2 - 8th note resolution
		1 - 4th note resolution
	
	'''
	if(False):
		infile,newfile = argv[0],argv[1]
	p = midi.read_midifile(infile)
	tempor = collections.OrderedDict()
#	tempor = {}
	instlist = {}
	mintick = 100000000000000
	for count,track in enumerate(p):
		if count: #insert logic here to filter tracks
			instant = 0
			note_count = 0
			for obj in track:
				##print obj

				if type(obj) is midi.events.TimeSignatureEvent:
					instlist[instant] = obj.get_numerator(),obj.get_denominator()
					##print instant
				elif  type(obj) is midi.events.NoteOnEvent or midi.events.NoteOffEvent:
					if obj.name == 'Note On' and obj.get_velocity() != 0:
						if note_count == 0:
							instant = obj.tick*DELTA_VAL/p.resolution
						#elif obj.tick != 0:
						else:
					
							try:
								tempor[instant].add(obj.get_pitch())
								'''
								if obj.data[1] == 0:# and previous_obj:
									#tempor[instant].add(previous_obj.data[0])
									tempor[instant].add(obj.data[0])
							
								else:
									tempor[instant].add(obj.data[0])
									#tempor[instant].add(previous_obj.data[0])
								'''
							except KeyError:
								tempor[instant] = {obj.get_pitch()}
							instant += obj.tick*DELTA_VAL/p.resolution
						note_count +=1
					else:
						instant += obj.tick*DELTA_VAL/p.resolution
						
				try:
					pass
					#print tempor[instant]	
				except:
					pass		
	#each instant will have an associated group of notes. the first of these should have a delta relative to the previous instant,
	# and the other chunk/set members should have a delta of 0
	
	#test to see first few objects of each track
	for track in p:
		instant = 0
		#print "\nTRACK ---------------------\n"
		for count,obj in enumerate(track):
			if (count < 250) and type(obj) is midi.events.NoteEvent and obj.tick != 0:
				pass
				
				##print obj.name, obj
				
	with open(newfile, 'w') as outfile:
		prev_instant = 0
#		tempor = SortedDict(tempor)
		#for instant, chunk in tempor.items():
		for i,pair in instlist.items():
				if i == 0:
					outfile.write("SIGEVENT\n")
					outfile.write( (str(pair[0]) + ',' + (str(pair[1])) )+'\n' )
		for instant in sorted(tempor.iterkeys()):
			
			try:
				pass
			except KeyError:
				pass
			for i,pair in instlist.items():
				if i == instant and i != 0:
					outfile.write("SIGEVENT\n")
					outfile.write( (str(pair[0]) + ',' + (str(pair[1])) )+'\n' )
			chunk = tempor[instant]
			delta = instant - prev_instant
			##print instant/2.4,chunk
			for note in chunk:
				#write the notes to a file
				outstring = str(note) + ',' + str(delta)
				outfile.write(outstring+'\n')
				delta=0
			prev_instant = instant
	

