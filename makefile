EXE = a
SOURCES = base.cpp bar.cpp chunk.cpp note.cpp rotatevisitor.cpp printvisitor.cpp midi2melody.cpp
OBJECTS := (SOURCES:%.cpp=%.o)
DEPS := (SOURCES:%.cpp=%.h)

INC_PATH = -I midifile/include/
LIB_PATH = -L midifile/lib/ -l midifile
all: $(EXE)

$(EXE): base.h chunk.h bar.h note.h $(SOURCES)
	g++ $(CFLAGS) -Wall -std=c++11 $(INC_PATH) $(SOURCES) main.cpp $(LIB_PATH) -o $(EXE)
parser: parser.cpp
	g++ $(CFLAGS) -std=c++11 parser.cpp main.cpp -o parser
test: main.cpp
	g++ $(CFLAGS) -std=c++11 $(INC_PATH) main.cpp $(LIB_PATH) -o test
clean:
	rm -rf *o $(EXE) parser test
