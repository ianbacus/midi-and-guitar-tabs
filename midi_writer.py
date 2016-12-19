import midi, collections, sys, os
from collections import Counter

#note_offsets = (00,00,00,00,00,00,00,00,00,00,00,00,00,00)
delta_offsets = (00,00,00,00)


DELTA_VAL = 8.0

lowest = 0
max = 0
def float_eq( a, b, eps=0.0001 ):
    return abs(a - b) <= eps
    
    
def make(newfile,infile,condition,note_offsets):
	#condition = "count in range(3)"
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
	
	note_offsets = eval(note_offsets)
	print note_offsets
	if(False):
		infile,newfile = argv[0],argv[1]
	p = midi.read_midifile(infile)
	tempor = collections.OrderedDict()
	instlist = {}
	mintick = 100000000000000
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
						except KeyError:
							tempor[instant] = [pitch]
						note_count +=1
			try:
				print "track "+str(count) +" average pitch: " +str(note_sum/note_count)+"["+str(lowest)+","+str(max)+"]: "+ trackname
				
			except:
				pass			
			note_sum=0
	
	with open(newfile, 'w') as outfile:
		lost_notes = 0
		prev_instant = 0
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
			chunk = list(set(chunk)) #remove redundant notes (exact copies only)
			chunk.sort()
			
			while len(chunk) > 5:
				lost_notes += 1
				chunk.pop(len(chunk)/2) #remove internal voices if the texture is too thick :'(
			
			if len(chunk) > 5:
				print chunk
			delta = instant - prev_instant
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
		#default to mass translate option: generate an intermediate file for all input files
		for file in os.listdir('data/input_files'):
			try:
				make(newfile="data/intermediates/"+fileName+".txt", \
				infile="data/input_files/"+fileName+".mid",condition="True")
			except:
				print "error parsing " + str(fileName)
	elif len(sys.argv) == 3:
		condition = sys.argv[2]
		file_name = sys.argv[1]
		make(newfile="data/intermediates/"+fileName+".txt", \
		infile="data/input_files/"+fileName+".mid",condition=condition)

