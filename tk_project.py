import time
try:
	# python 2.x
	import Tkinter as tk
except ImportError:
	# python 3.x
	import tkinter as tk

from midi_writer import make as generate_pitch_delta

class Example(tk.Frame):
	def __init__(self, *args, **kwargs):
		tk.Frame.__init__(self, *args, **kwargs)

		self.text = tk.Text(self, height=6, width=40)
		self.vsb = tk.Scrollbar(self, orient="horizontal", command=self.text.xview)
		self.text.configure(yscrollcommand=self.vsb.set)
		self.vsb.pack(side="right", fill="y")
		self.text.pack(side="left", fill="both", expand=True)

		self.add_timestamp()

	def add_timestamp(self):
		self.text.insert("end", time.ctime() + "\n")
		self.text.see("end")
		self.after(1000, self.add_timestamp)

	def display_tabfile(self, tabfile):
		with open(infile, 'r') as tabfile:
			for line in tabfile:
				text.insert("end",line+'\n')

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
	file_name = 'bwv851'
	global_offset = 24
	
	root =tk.Tk()
	frame = Example(root)
	
	generate_pitch_delta(newfile="pitch_deltas/"+file_name+".txt", infile="midi_files/"+file_name+".mid")
	subprocess.Popen('./a pitch_deltas/'+file_name+' '+str(global_offset)+' 0 0')
	
	frame.display_tabfile( file_name+'.txt' )

