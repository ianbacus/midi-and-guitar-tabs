# midi

Use:  python midiTranslate.py <midi filename> <transpose> <measuresPerRow> <startMeasure> <endMeasure>
Example: python midiTranslate.py 988-v01 2 3 0 -1
This will load the midi file "data/input_files/988-v01.mid," transpose all of its notes down by two pitches, print 3 measures per row, and print all of the measures (-1 defaulting to the maximum size for the upper measure bound).
