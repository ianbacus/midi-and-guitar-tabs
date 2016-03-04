import midi, collections, sys, os
from collections import Counter
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
							tempor[instant].append(pitch)
							'''
							if obj.data[1] == 0:# and previous_obj:
								#tempor[instant].add(previous_obj.data[0])
								tempor[instant].add(obj.data[0])
						
							else:
								tempor[instant].add(obj.data[0])
								#tempor[instant].add(previous_obj.data[0])
							'''
						except KeyError:
							tempor[instant] = [pitch]
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
	#for pitch_chunk in tempor.values():
		
	
	#remove those pesky trills
	#for instant,chunk in tempor:
	#	print str(instant) + str(chunk)


####

	'''		
	last_instant = 0
	counter = 0
	chunk_queue = []
	instant_to_trill = {} #dictionary of time instants to trill pair "frozen sets"
	main_trill_pairs = {} #[initial inst : pair ]
	candidate_trills = []
	countmap = {}
	for instant in tempor.iterkeys():
		instant_diff = (instant - last_instant)
		#If two successive chunks are close enough apart, begin analysis
		if(instant_diff <= 2.0) and (instant_diff != 0):
			
			#perform a scan across every combination of notes in the two chunks
			for note_select in tempor[instant]:
				for last_note_select in tempor[last_instant]:
					#if any of the two notes are within the trill range between these two chunks, add the pair of trill notes to a map
					#possible to get multiple trill pairs in one pass over two chunks
					if (1 <= abs(note_select - last_note_select) <= 4):
						trill_pair = frozenset([note_select,last_note_select]) #each trill pair should have a counter to go with it
						try:
							instant_to_trill[last_instant].append(trill_pair)
						except:
							instant_to_trill[last_instant] = [trill_pair]
			if trill_pair:
				#after inspecting the two chunks comprehensively and obtaining possible trill pairs, verify 
				#that they are trills by  
				keylist = [item for item in instant_to_trill.iterkeys()]
				keylist.sort()
				print keylist
				
				previous_pairs = instant_to_trill[keylist[(keylist.index(last_instant)-1)]] #get the trill pairs from the previous instance, if they exist
				current_pairs = instant_to_trill[last_instant]
				additions = set(current_pairs).union(set(previous_pairs).intersection(current_pairs)) #add one of these
				removals = set(previous_pairs) - set(additions) #remove ALL of these 
				filter(lambda a: a in list(removals), candidate_trills)
				candidate_trills.append(additions)
				print candidate_trills
				fslist = []
				for item in candidate_trills:
					#count the number of each occurence of a set in candidate_trills
					#unhashable: can't use dict of sets to counts
					#if the number of counts is 2 or more, delete them from the original tempor map. this can be evaluated 
					
					
					fslist.append(item)
					
					
			#if fslist contains 3 or more of any trill pair, find them by their time instance and delete them
			if
				#print str(previous_pairs=) + " " + str(current_pairs) + ":" + str(additions) + "/" + str(removals)
			trill_pair = 0
			
			
			
			#add candidate trills to a second data structure that is responsible for keeping track of the counts of each of the trill pairs
			#key list will have two chunks that need to be compared
			
			
			
			
			
			
			
			previous_instant_to_trill = instant_to_trill
							
				
							
							
							
							
							
							
							
			'''							
							
							
							
							
	"""						
						if len(instant_to_trill.keys()) >= 1: #catches the zero and one cases							
						#if there is more than one time (key) in the trill map, then determine if this tril pair matches that one (continues trilling same notes)
						#this will only be evaluated after a few iterations through the main tempor map
							pass
						#chunk_queue.append(tempor[instant])
						if len(instant_to_trill.keys()) > 3:
							main_trill_pair.add(instant_to_trill.values()[0])
							#print main_trill_pair
							for val in instant_to_trill.values(): #continuing trill
								#print val
								main_trill_pair.add(main_trill_pair)
							if main_trill_pair:
								print "TRILL AT " + str(instant) + ":" + str(main_trill_pair)
								for time in instant_to_trill.keys():
									print "\t " + str(main_trill_pair) + str(tempor[time])
									for trill_note in main_trill_pair:
										if trill_note in tempor[time]:
											tempor[time].remove(trill_note)
					else:	
						instant_to_trill = {}
						main_trill_pair = (0,0)				
		else:
			instant_to_trill = {}
			main_trill_pair = (0,0)
										
"""					

			
		#last_instant = instant
####		
		
		
		
		
	with open(newfile, 'w') as outfile:
		lost_notes = 0
		prev_instant = 0
#		tempor = SortedDict(tempor)
		#for instant, chunk in tempor.items():
		for i,pair in instlist.items():#instant list
				if i == 0:
					outfile.write("SIGEVENT\n")
					outfile.write( (str(pair[0]) + ',' + (str(pair[1])) )+'\n' )
		for instant in sorted(tempor.iterkeys()):
			for i,pair in instlist.items():
				if i == instant and i != 0:
					outfile.write("SIGEVENT\n")
					outfile.write( (str(pair[0]) + ',' + (str(pair[1])) )+'\n' )
			chunk = tempor[instant]
			chunk = list(set(chunk))
			chunk.sort()
			while len(chunk) > 4:
				#print "popped from " + str(len(pitch_chunk)/2)
				lost_notes += 1
				chunk.pop(len(chunk)/2) #remove internal voice >_<
			if len(chunk) > 5:
				print chunk
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
		print str(lost_notes) + " lost notes"
	
if __name__ == "__main__":
	if len(sys.argv) == 1:
		for file in os.listdir('midi_files'):
			if 'mess' in file:
				try:
					make(newfile="pitch_deltas/"+file+".txt", infile="midi_files/"+file, condition="True")
				except:
					print "error parsing " + str(file_name)
	else:
		condition = sys.argv[2]
		file_name = sys.argv[1]
		
		
		#MASS TRANSLATE
		
		
		make(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid",condition=condition)
		#make(newfile="pitch_deltas/"+file_name+".txt", infile=file_name,condition=condition)
		
#	outstring = str(obj.data[0]) + ',' + str(obj.data[1])
#	outfile.write(outstring+'\n')

