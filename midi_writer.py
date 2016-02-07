import midi, collections, sys, os
#from sortedcontainers import SortedDict

#Indexed by track to perform selective "filtered" modifications
note_offsets = (00,00,00,00,00,00,00,00,00,00,00,00,00,00) #in pitches, bitches
delta_offsets = (00,00,00,00) #in ticks?




#		  (00,01,02,03,04,05,06,07,08,09,10)

#condition used to filter out tracks. tracks are enumerated. a condition should be ideally used 
#to introduce the maximum amount of playable voices in a contrapuntal piece while maintaining
#a good balance of ornamentation and functional harmony. note doublings, non-functional melodic
# continuation. 
#
#condition = "count in [1]"

'''
Trill detection: 
- two notes that are within four pitches
- played (more rapidly than other notes, by std dev?) at least 2 times: "abab" patterns

Trill patterns should be replaced by halved trills (for now), later they should be notated as well for the print visitor (like triples)




'''

DELTA_VAL = 8.0

lowest = 0
max = 0
def float_eq( a, b, eps=0.0001 ):
    return abs(a - b) <= eps
    
    
def forward_test():
	'''
	Test inputting a file and re-rendering it into a new file to see if VishnuBob's project is the source of the midi errors
	Forwarding a file in to out works fine.
	'''
	inp = midi.read_midifile("midi_files/988-v01.mid")
	out = midi.containers.Pattern()
	
	for count, track in enumerate(inp):
		out.append(midi.containers.Track())
		print(count)
		for event in track:
			if type(event) not in [midi.events.ProgramChangeEvent, midi.events.ControlChangeEvent]:
				out[count].append(event)
			else:
				print("EVENT")
	
	midi.write_midifile("test.mid",out)
	
    
def make(newfile,infile,condition):
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
	#print DELTA_VAL, p.resolution
	
	rhythm_map = {(1*DELTA_VAL/p.resolution)}
	
	for count,track in enumerate(p):
		
		if eval(condition): #insert logic here to filter tracks
			trackname = ""
			lowest = 200
			max = 0
			instant = 0
			note_count = 0
			note_sum =1
			try:
				instant += (delta_offsets[count]*DELTA_VAL/p.resolution)
			except:
				pass
			for obj in track:					
				if type(obj) is midi.events.TimeSignatureEvent:
					instlist[instant] = obj.get_numerator(),obj.get_denominator()
				if type(obj) is midi.events.TrackNameEvent:
					trackname = obj.text
				
				try:
					instant += (obj.tick*DELTA_VAL/p.resolution)
				except:
					pass
				#Note events: update the ticks of the pitch-delta map, add in note on event pitches at the instant
				if type(obj) in [midi.events.NoteOnEvent,midi.events.NoteOffEvent]:
				
					#if (obj.name == 'Note Off') or (float_eq(obj.get_velocity(),0.0)):	
						#tempor[instant] = {pitch} #use last pitch

					#elif obj.name == 'Note On' and type(obj) is midi.events.NoteOnEvent:# and not (float_eq(obj.get_velocity(),0.0)):
					if (type(obj) is midi.events.NoteOnEvent) and not (float_eq(obj.get_velocity(),0.0)):
						'''
						if note_count == 0:
							instant = obj.tick*DELTA_VAL/p.resolution
						#elif obj.tick != 0:
						else:#what was this for?
						'''
						
						
						pitch = obj.get_pitch()
						try:
							pitch += note_offsets[count]
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
						note_count +=1
			try:
				#pass
				print "track "+str(count) +" average pitch: " +str(note_sum/note_count)+"["+str(lowest)+","+str(max)+"]: "+ trackname
			except:
				pass			
			note_sum=0
			
	#each instant will have an associated group of notes. the first of these should have a delta relative to the previous instant,
	# and the other chunk/set members should have a delta of 0
	#test to see first few objects of each track
	for set in tempor.values():
		try:
			while len(set)> 5:
				set.pop()
		except:
			pass		

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
	if len(sys.argv) == 1:
		forward_test()
	else:
		condition = sys.argv[2]
		file_name = sys.argv[1]
		
		'''
		#MASS TRANSLATE
		for file in os.listdir(file_name):
			if '.mid' in file:
				try:
					make(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file, condition="True")
				except:
					print "error parsing " + str(file_name)
		'''
		make(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid",condition=condition)
		#make(newfile="pitch_deltas/"+file_name+".txt", infile=file_name,condition=condition)

#	outstring = str(obj.data[0]) + ',' + str(obj.data[1])
#	outfile.write(outstring+'\n')

