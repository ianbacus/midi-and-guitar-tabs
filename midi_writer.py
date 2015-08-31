import midi, collections, sys
#from sortedcontainers import SortedDict

offsets = (00,00,00,00,00,00,12,00,00,00,00,00,00)
#		  (00,01,02,03,04,05,06,07,08,09,10)
condition = "True"#count in [0,1]"


DELTA_VAL = 8.0

lowest = 0
max = 0
def float_eq( a, b, eps=0.0001 ):
    return abs(a - b) <= eps
    
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
	print "FAGGOTISH"
	print DELTA_VAL, p.resolution
	rhythm_map = {(1*DELTA_VAL/p.resolution)}
	for count,track in enumerate(p):
		if eval(condition): #insert logic here to filter tracks
			lowest = 200
			max = 0
			instant = 0
			note_count = 0
			note_sum =1
			for obj in track:
				##print obj
				
				if type(obj) is midi.events.TimeSignatureEvent:
					instlist[instant] = obj.get_numerator(),obj.get_denominator()
				
				#Note events: update the ticks of the pitch-delta map, add in note on event pitches at the instant
				elif  type(obj) is midi.events.NoteOnEvent or midi.events.NoteOffEvent:
				
					if obj.name == 'Note Off':	
						#tempor[instant] = {pitch} #use last pitch
						instant += (obj.tick*DELTA_VAL/p.resolution)
					elif obj.name == 'Note On':# and not (float_eq(obj.get_velocity(),0.0)):
						'''
						if note_count == 0:
							instant = obj.tick*DELTA_VAL/p.resolution
						#elif obj.tick != 0:
						else:#what was this for?
						'''
						pitch = obj.get_pitch()
						try:
							pitch += offsets[count]
						except:
							pass
						note_sum+=pitch
						if pitch < lowest:
							lowest = pitch
						if pitch > max:
							max = pitch
						try:
							tempor[instant].add(pitch)
							'''
							if obj.data[1] == 0:# and previous_obj:
								#tempor[instant].add(previous_obj.data[0])
								tempor[instant].add(obj.data[0])
						
							else:
								tempor[instant].add(obj.data[0])
								#tempor[instant].add(previous_obj.data[0])
							'''
						except KeyError:
							tempor[instant] = {pitch}
						instant += (obj.tick*DELTA_VAL/p.resolution)
						note_count +=1
					#elif obj.name== 'Note Off':
					else:	
						#tempor[instant] = {pitch} #use last pitch
						instant += (obj.tick*DELTA_VAL/p.resolution)
					#else:
					#	print obj.name, str(obj.get_velocity())
				else:
					try:
						print type(obj)
						print obj.tick, obj.pitch	
					except:
						print "fucking dickn uts"
						pass			
			print count, lowest, max
			try:
				print "track "+str(count) +" average pitch: " +str(note_sum/note_count)
			except:
				pass			
			note_sum=0
			
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
		for i,pair in instlist.items():#instant list
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
			'''
			if delta not in [note_1,note_2,note_4,note_8,note_16,note_32,note_64]:
				print "TUPLE"
			else:
				pass#print rhythm_map[delta]
			##print instant/2.4,chunk
			'''
			for note in chunk:
				#write the notes to a file
				outstring = str(note) + ','
				if float_eq(delta,DELTA_VAL/12.0):
					#then we got us a tuple note
					outstring+="-1"
				else:
					outstring+= str(round(delta))
				outfile.write(outstring+'\n')
				delta=0
			prev_instant = instant
	
if __name__ == "__main__":

	file_name = sys.argv[1]
	make(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid")

#	outstring = str(obj.data[0]) + ',' + str(obj.data[1])
#	outfile.write(outstring+'\n')

