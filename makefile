EXE = a
SOURCES = base.cpp bar.cpp chunk.cpp note.cpp
OBJECTS = main.o base.o bar.o chunk.o note.o

INC_PATH = -I midifile/include/
LIB_PATH = -L midifile/lib/ -l midifile
all: $(EXE)

$(EXE): tabber.h $(SOURCES)
	g++ $(CFLAGS) -std=c++11 $(SOURCES) -o $(EXE)
parser: parser.cpp
	g++ $(CFLAGS) -std=c++11 parser.cpp main.cpp -o parser
test: main.cpp
	g++ $(CFLAGS) $(INC_PATH) main.cpp $(LIB_PATH) -o test
clean:
	rm -rf *o $(EXE) parser test
