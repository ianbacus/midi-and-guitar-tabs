import time, sys
try:
	# python 2.x
	import Tkinter as tk
	from Tkinter import *
except ImportError:
	# python 3.x
	import tkinter as tk
	from tkinter import *

from tkFileDialog  import askopenfilename 
from midi_writer import make as generate_pitch_delta
import subprocess

#global_offset=0

class App:
	def __init__(self, master,filename,global_offset):
		frame = Frame(master,width=10000, height=200)
		#toolbar = Frame(frame,width=500,height=60)
		#toolbar.pack()
		#frame.config(width=10000)
		#toolbar.grid(row=1)
		self.file_name = filename
		self.global_offset = global_offset
		self.chosen = 'testoutput.txt'
		self.framerate=500
		self.speedctl=0
		self.dir=0
		#self.chosen = askopenfilename(initialdir='~')
		
		#creation
		
		self.button = Button(frame, text="QUIT", command=frame.quit).pack(side=BOTTOM)
		self.rewind_btn = Button(frame, text="<<", command=self.rewind).pack(side=BOTTOM)
		self.pause_btn = Button(frame, text="||", command=self.pause).pack(side=BOTTOM)
		master.bind("<space>", self.pause)
		self.play_btn = Button(frame, text=">>", command=self.play).pack(side=BOTTOM)
		
		self.slow_btn = Button(frame, text="slower", command=self.downspeed).pack(side=BOTTOM)
		self.speed_btn = Button(frame, text="faster", command=self.upspeed).pack(side=BOTTOM)
		
		self.downshift_btn = Button(frame, text="-", command=self.downshift).pack(side=BOTTOM)
		self.upshift_btn = Button(frame, text="+", command=self.upshift).pack(side=BOTTOM)
		#self.tempo=textfield
		frameID = self.frametext = Text(frame, wrap=NONE, width=1000, font=("Courier", 18))
		self.frametext.grid(row=0)
		currentscroll = Scrollbar(frame,orient=HORIZONTAL)
		
		#configuration
		self.frametext.insert(END, open(self.chosen).read())  
		currentscroll.config( command = self.frametext.xview )
		
		
		#packing
		frame.pack()
		currentscroll.pack(side=BOTTOM,fill=X)
		self.frametext.pack(side=TOP)
		
		self.autoscroll()
	def upspeed(self):
		self.speedctl += 1
	def downspeed(self):
		if self.speedctl != 0:
			self.speedctl -= 1
	def upshift(self):
		self.pause()
		self.global_offset-=1
		spstring= ['./a', 'pitch_deltas/'+self.file_name+'.txt', str(self.global_offset), '0', '0',]
		subprocess.call(spstring)
		print spstring
		self.frametext.delete(1.0, END) 
		self.frametext.insert(END, open(self.chosen).read())  
	def downshift(self):
		self.pause()
		self.global_offset+=1
		spstring= ['./a', 'pitch_deltas/'+self.file_name+'.txt', str(self.global_offset), '0', '0',]
		subprocess.call(spstring)
		print spstring
		self.frametext.delete(1.0, END) 
		self.frametext.insert(END, open(self.chosen).read())  
	def pause(self,event=None):
		self.framerate=1000
		self.dir=0
	def rewind(self):
		self.framerate=100
		self.dir=-self.speedctl
	def play(self):
		self.framerate=100
		self.dir=self.speedctl
	def set_scrollspeed(self,n):
		self.speedctl=n
	def autoscroll(self):
		self.frametext.xview(tk.SCROLL, self.dir, tk.UNITS)
		self.frametext.after(self.framerate, self.autoscroll)
		#self.frametext.xview(tk.SCROLL, 1, tk.UNITS)
		#self.frametext.after(10, self.autoscroll)


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
	try:
		if(len(sys.argv) == 3 and '.' not in (sys.argv[1])) and int(sys.argv[2]):
			file_name = sys.argv[1]
			global_offset = int(sys.argv[2])
			root = tk.Tk()
			app = App(root,file_name,global_offset)
			generate_pitch_delta(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid")
			spstring= ['./a', 'pitch_deltas/'+file_name+'.txt', str(global_offset), '0', '0',]
			subprocess.call(spstring)#join this as a thread
			root.wm_title(file_name)
			root.mainloop()
		else:
			raise ValueError
	except ValueError:
		print 'Enter the name of the midifile without a file extension and a global offset for pitch values'
		
		
		
		
		
		
		
