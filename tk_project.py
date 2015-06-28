import time, sys
try:
	# python 2.x
	import Tkinter as tk
	from Tkinter import *
except ImportError:
	# python 3.x
	import tkinter as tk
	from tkinter import *


'''	
from tkinter import *
root=Tk()
frame=Frame(root,width=300,height=300)
frame.grid(row=0,column=0)
canvas=Canvas(frame,bg='#FFFFFF',width=300,height=300,scrollregion=(0,0,500,500))
hbar=Scrollbar(frame,orient=HORIZONTAL)
hbar.pack(side=BOTTOM,fill=X)
hbar.config(command=canvas.xview)
vbar=Scrollbar(frame,orient=VERTICAL)
vbar.pack(side=RIGHT,fill=Y)
vbar.config(command=canvas.yview)
canvas.config(width=300,height=300)
canvas.config(xscrollcommand=hbar.set, yscrollcommand=vbar.set)
canvas.pack(side=LEFT,expand=True,fill=BOTH)
'''

from tkFileDialog  import askopenfilename 
from midi_writer import make as generate_pitch_delta
import subprocess
class App:
	def __init__(self, master):
		frame = Frame(master,width=1000, height=60)

		frame.config(width=10000)

		self.chosen = 'testoutput.txt'
		self.i=0
		#self.chosen = askopenfilename(initialdir='~')
		
		#creation
		self.button = Button(frame, text="QUIT", command=frame.quit)
		self.frameID = self.frametext = Text(frame, wrap=NONE, width=1000)
		self.currentscroll = Scrollbar(frame,orient=HORIZONTAL)
		
		#configuration
		self.frametext.insert(END, open(self.chosen).read())  
		self.currentscroll.config( command = self.frametext.xview )
		
		#packing
		frame.pack()
		self.button.pack(side=BOTTOM)
		self.currentscroll.pack(side=BOTTOM,fill=X)
		self.frametext.pack(side=TOP)
		
		self.autoscroll()
		
	def autoscroll(self):
		#self.canvas.move(self.tabID,-50,0)
		self.i +=10
		self.currentscroll.set(self.i,self.i)
		self.frame.after(1000, self.autoscroll)


if __name__ == "__main__":
	'''
	take a midi file name, generate a pitch_delta list (call the midi_writer module)
		- filter tracks, use the midi library track metadata information to accomplish this
		- modify individual track pitch offsets

	pass this file through the c++ module to parse the midi, translate to tabs, create a data structure to store the result
		- modify global offset of notes
		- modify display: bars per row

	print this tab into the TK gui window to autoscroll through the contents at a pre-set tempo

	'''
	
	##test args
	
	print len(sys.argv)
	
	try:
		if(len(sys.argv) == 3 and '.' not in (sys.argv[1])) and int(sys.argv[2]):
			file_name = sys.argv[1]
			global_offset = int(sys.argv[2])
			root = tk.Tk()
			app = App(root)
			generate_pitch_delta(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid")
			spstring= ['./a', 'pitch_deltas/'+file_name+'.txt', str(global_offset), '0', '0',]
			subprocess.call(spstring)
			root.mainloop()
		else:
			raise BadInput
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
		
		
		
		
		
		
		
