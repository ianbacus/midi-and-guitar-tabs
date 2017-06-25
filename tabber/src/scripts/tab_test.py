import tab_thing
from tab_thing import *

'''

aB1  = [[ el , e  , el, fs]    # First line Bar1 first part
	   ,[ el , gm , el, am]]   # second part 
aB2  = [[ el , g  , el, ds]
	   ,[ el , bmf, el, am]]
aB3  = [[ el , e  , el, fs]
	   ,[ el , gm , el, am]]
aB4  = [[ el , g  , el, ds]
	   ,[ el , bmf, el, am]]

aB5  = [[ el      ,[g,e] ,g  ,     fs]    # second line
	   ,[ el      ,bl    ,el ,     al]]
aB6  = [[ds      ,e     ,[d,d],     d]    # << recursive tree structure
	   ,[fls      ,gl    ,el     , bl]]
aB7  = [[[cms,cms],cms   ,[cm,cm], cm]
	   ,[el       ,blf   ,el     , al]]
aB8  = [[[bm,e]   ,[cm,e],bm     , p]
	   ,[el       ,al    ,el     ,[cl,bl]]]

aB9  = [[p       ,[gm,em],gm   ,fms]
	   ,[blf             ,al       ]]
aB10 = [[cm      ,bm     ,[g,g], g] 
	   ,[gl      ,em     ,bmf  , em]]
aB11 = [[[fs,fs] ,fs     ,[e,e], e]
		,[bl     ,fms    ,cls  , fms]]
aB12 = [[[ds,fs] ,[e,fs] ,ds   , p]
		,[bl     ,cl     ,bl   , [gl,fls]]]

aB13 = [[el       ,[g,e] ,g      ,fs]
	   ,[el       ,bm    ,el     ,am]]
aB14 = [[ds       ,e     ,[d,d]  ,d]
	   ,[fms      ,gm    ,em     ,bmAlt]]
aB15 = [[[cms,cms],cms   ,[cm,cm],cm]
		,[em      ,bmf   ,em     ,am]]
aB16 = [[[bm ,e]  ,[cm,e],bm     ,p]
		,[em      ,am   ,em     ,[em,dm]]]

aB17 = [[p  , [e,cms] , e  , ds]
	   ,[cls          , fms]]
aB18 = [[p  , [d ,bm] , d  , cms]
	   ,[bl           , el]]
aB19 = [[p  , [cm,am] , cm , bm]
	   ,[al           , dm]]
aB20 = [[p  , [dms,bm], em, p]
	   ,[gl , fls     , el, p]]

aFirstPart = [aB1[0],aB2[0],aB3[0],aB4[0],aB5[0],aB6[0],aB7[0],aB8[0],aB9[0]
			 ,aB10[0],aB11[0],aB12[0],aB13[0],aB14[0],aB15[0],aB16[0],aB17[0]
			 ,aB18[0],aB19[0],aB20[0]]

aSecondPart = [aB1[1],aB2[1],aB3[1],aB4[1],aB5[1],aB6[1],aB7[1],aB8[1],aB9[1]
			 ,aB10[1],aB11[1],aB12[1],aB13[1],aB14[1],aB15[1],aB16[1],aB17[1]
			 ,aB18[1],aB19[1],aB20[1]]
			 
'''

# Unfortunately I cant work out a way to play both parts together using Beep,
# unless you use two computers.
# However if you are using PlaySound
#,( you will also need the associated sound files)
# You can make a second copy of the program (rename it SugarSecondPart)
# to play the second part alongside the first
# The section of code below will syncronise the two parts.

if not(lBeep):
        while (int(time()) % 10 == 0):
                sleep(1)
        t = int(time()) % 10
        while t % 10 != 0:
                t = int(time())
                #print 'syncronising', t % 10
fname = '988-v03.txt'

score_string = ""
with open(fname) as f:
	score_string = f.readline()

f.close()
#score_string = "[[TabNote(0,0),TabNote(0,1),p,p,p,]]"
#print(score_string)
score = eval(str(score_string))
#print(score)

#print(score[0:20])
#print(score[0])

myTab = TabView("eadgbe",1,3,score[0:20])

#myTab.Render()
#myTab.PlaceParts(score)

myTab.app.mainloop()

'''
print 'Playing first part'
PlayTune(aFirstPart)
#sleep(2)
print 'Playing second part'
PlayTune(aSecondPart)

'''


