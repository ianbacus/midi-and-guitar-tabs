# writes tab scores for guitar
from winsound import *
from time import *
from tkinter import *
aListType	= [1,2,3] # for type testing
lBeep = True # If true it will play through the built in speaker.
			  # otherwise it will search for associated wav files
			  # One file per note needed
cSoundsFolder = 'Guitar/'

class TabNote:
	'''
	Class for storing information about the position of a note on the fretboard, and information about how to playback the note
	'''
	def __init__(self, nS, nF, nP=0, cFile="asd"):
	
		nScaleFactor = 1.05946309436 # exp(log(2) / 12) # even temper diatonic		
		self.nString   = nS
		self.nFret	 = nF
		self.nScalePos = nP
		self.nTone	 = int(440 * nScaleFactor ** (nP - 6)) # concert pitch
		self.lPause	= False
		self.cSound	= cSoundsFolder + cFile # wav file
	def __repr__(self):
		return "n"
	def play(self, nI):
		if lBeep:
			Beep(self.nTone, nI)  # milliseconds
		else:
			nStartTime = time()
			PlaySound(self.cSound, SND_FILENAME + SND_ASYNC)
			sleep(nI + nStartTime - time())	  # seconds
	def drawNote(self, canvas, nStart, aLines):	
		canvas.create_rectangle(nStart - 5, aLines[self.nString - 1] - 5, nStart + 5, aLines[self.nString - 1] + 5, width = 0,fill = 'white')
		canvas.create_text(nStart, aLines[self.nString - 1], text = str(self.nFret))
		

class TabPause:
	def __init__(self, nS, nF, nP, cFile):
		self.nString   = nS
		self.lPause	= True
	def __repr__(self):
		return "p"
	def play(self, nI):
		if lBeep:
			sleep(nI / 1000) # milliseconds
	def drawNote(self, canvas, nStart, aLines):
		pass
		

class TabView:
	def __init__(self, strings,tempo,nBarsPerLine):
		self.numCourses = len(strings)
		self.nBarsPerLine = nBarsPerLine
		self.barsCount = nBarsPerLine
		self.nW = 1400
		self.nH = 800
		self.app = Tk()
		self.canvas = Canvas(self.app, width = self.nW, height = self.nH, bg = 'white')
		self.canvas.pack(expand = YES, fill = BOTH)
		self.aBoxes = []
		self.debug_count = 0
		

	def PlayNote(aB, nI):
		nDelta = 0
		if isinstance(aB, type(aListType)):
			for i in aB:
				nTime = time()
				PlayNote(i, nI / len(aB) - nDelta)
				nSpent = time() - nTime
				nDelta = nSpent - nI / len(aB) # catchup if going too slowly
				if nDelta < 0:
					nDelta = 0
		else:
			aB.play(nI)
	def Render(self):
		'''
		
		'''
		self.nW = self.nW * 0.75
		nLines = 1#(len(aFirstPart) + 0.0) / self.nBarsPerLine
		if int(nLines) != nLines:
			nLines = int(nLines + 1)

		nLineStep = self.nH * 0.8 / 5 # 5 lines
		self.canvas.create_text( 300, 50, text = 'Coolest tab ever', font = ('Arial', 20, 'bold'))
		for i in range(nLines):
			self.aBoxes.append(self.DrawLines((nLineStep * i  + 100), nLineStep * 0.75, i))
	def PlayTune(self, aBars):
		nDelta = 0
		if lBeep:
			nDuration = 1300 # beeps are in milliseconds
		else:	
			nDuration = 1.5  # seconds per bar
		for aB in aBars:
			nTime = time()
			PlayNote(aB, nDuration -nDelta)
			nSpent = time() - nTime
			nDelta = nSpent - nDuration # catchup if going too slowly
			if nDelta < 0:
				nDelta = 0

	def DrawLines(self, nUp, nLineH, nLineNo): 
		nStartX = self.nW * 0.1
		nStopX  = self.nW * 0.1 * self.barsCount
		nLineUp = nLineH * 0.75 / self.numCourses # six strings
		nStartY = nUp + nLineUp
		nStopY  = nUp + self.numCourses * nLineUp
		nXStep  = (self.nW*2) / 5#self.nBarsPerLine
		aLineBox = []
		aBarBox = []
		for i in range(self.numCourses):
			self.canvas.create_line(nStartX, nUp + i * nLineUp + nLineUp, nStopX, nUp + i * nLineUp + nLineUp)
			aLineBox.append( nUp + i * nLineUp + nLineUp)

		for i in range(self.barsCount):
			self.canvas.create_line(nStartX + i * nXStep, nStartY, nStartX + i * nXStep, nStopY)
			aBarBox.append([nStartX + i * nXStep, nXStep])
		self.canvas.create_text(nStartX - 20,nUp + 1.5 * nLineUp, text = "T", font = ('Courier', 14,'bold'))
		self.canvas.create_text(nStartX - 20,nUp + 3.5 * nLineUp, text = "A", font = ('Courier', 14,'bold'))
		self.canvas.create_text(nStartX - 20,nUp + 5.5 * nLineUp, text = "B", font = ('Courier', 14,'bold'))
		if nLineNo == 0: # time signature
			self.canvas.create_text(nStartX - 10, nUp + 2.5 * nLineUp, text = "2", font = ('Courier', 14,'bold'))
			self.canvas.create_text(nStartX - 10, nUp + 4.5 * nLineUp, text = "4", font = ('Courier', 14,'bold'))
		return [aLineBox, aBarBox]


	def PlaceNote(self, aB, nI, nStart, aLines):
		#recurse through a bar
		if isinstance(aB, list):
			for i in aB: #for chunk/note
				if isinstance(i, list):
					for j in i:
						j.drawNote(self.canvas, nStart, aLines)
				else:
					i.drawNote(self.canvas, nStart, aLines)
					#self.PlaceNote(i, nI / len(aB), nStart, aLines)
				nStart += nI / len(aB)
		else:
			pass#aB.drawNote(self.canvas, nStart, aLines)
			
	def PlaceParts(self, aPart):
		nBarStep = 0
		for i in self.aBoxes:
			aLines = i[0]
			for j in range(self.nBarsPerLine):
				if len(aPart) <= nBarStep:
					for k in aB:
						pass#nStart += nDuration/ len(aB)
					return
				aB = aPart[nBarStep]
				nBarStep = nBarStep + 1
				nDuration = i[ 1][ j][ 1] * .9 #0.95 
				nStart	= i[1][ j][ 0] + nDuration * 0.1
				for k in aB:
					self.PlaceNote(k, nDuration/ len(aB), nStart, aLines)
					#nStart += nDuration/ len(aB)

# EADGBE tuning

dl  = 0 # not needed here
dls = 0 # not needed here
p   = TabPause(0,0,1,'Pause.wav')	 # pause
el  = TabNote(6,0,1,'Elow.wav')	  # E low 
fl  = TabNote(6,1,2,'Flow.wav')	  
fls = TabNote(6,2,3,'FlowSharp.wav')	  # F low sharp
gl  = TabNote(6,3,4,'Glow.wav')
gls = TabNote(6,4,5,'GlowSharp.wav')
al  = TabNote(5,0,6,'Alow.wav')
blf = TabNote(5,1,7,'BlowFlat.wav')	  # B low flat
bl  = TabNote(5,2,8,'Blow.wav')
cl  = TabNote(5,3,9,'Clow.wav')
cls = TabNote(5,4,10,'ClowSharp.wav')
dm  = TabNote(4,0,11,'Dmid.wav')
dms = TabNote(4,1,12,'DmidSharp.wav')
em  = TabNote(4,2,13,'Emid.wav')
fm  = TabNote(4,3,14,'Fmid.wav')
fms = TabNote(4,4,15,'FmidSharp.wav')
gm  = TabNote(3,0,16,'Gmid.wav')
gms = TabNote(3,1,17,'GmidSharp.wav')
am  = TabNote(3,2,18,'Amid.wav')
bmf = TabNote(3,3,19,'BmidFlat.wav')
bm  = TabNote(2,0,20,'Bmid.wav')
bmAlt = TabNote(3,4,20,'Bmid.wav') # B middle Alternate fingering
cm  = TabNote(2,1,21,'Cmid.wav')
cms = TabNote(2,2,22,'CmidSharp.wav')
d  = TabNote(2,3,23,'D.wav')
ds = TabNote(2,4,24,'Dsharp.wav')
e   = TabNote(1,0,25,'E.wav')
f   = TabNote(1,1,26,'F.wav')
fs  = TabNote(1,2,27,'FSharp.wav')
g   = TabNote(1,3,28,'G.wav')
gs  = TabNote(1,4,29,'GSharp.wav')
a   = TabNote(1,5,30,'A.wav')
bf  = TabNote(1,6,31,'BFlat.wav')
b   = TabNote(1,7,32,'B.wav')
c   = TabNote(1,8,33,'C.wav')
cs  = TabNote(1,9,34,'CSharp.wav')
dh   = TabNote(1,10,35,'Dhigh.wav')
dhs  = TabNote(1,11,36,'DhighSharp.wav')
eh  = TabNote(1,12,37,'Ehigh.wav') # E high
fh  = TabNote(1,13,38,'Fhigh.wav')
fhs = TabNote(1,14,39,'FhighSharp.wav')
gh  = TabNote(1,15,40,'Ghigh.wav')
ghs = TabNote(1,16,41,'gHighSharp.wav')
ah  = TabNote(1,17,42,'Ahigh.wav')
bhf = TabNote(1,18,43,'BhignFlat.wav')
bh  = TabNote(1,19,44,'Bhign.wav')
ch  = TabNote(1,20,45,'Chigh.wav')
chs = TabNote(1,21,46,'ChignSharp.wav')
dd  = TabNote(1,22,47,'DD.wav')
dds = TabNote(1,23,48,'DDsharp.wav')


#print "End"