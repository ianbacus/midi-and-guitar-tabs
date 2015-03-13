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
test:
	$(CC) $(CFLAGS) $(INC_PATH) $(OBJECT_FILES) $(LIB_PATH) -o $(TARGET)
clean:
	rm -rf *o $(EXE) parser
