EXE = a
SOURCES = src/music_objects/bar.cpp 			\
	  src/music_objects/chunk.cpp 			\
	  src/music_objects/note.cpp 			\
	  src/data_translation/rotatevisitor.cpp 	\
	  src/data_translation/printvisitor.cpp 	\
	  src/data_translation/midi2melody.cpp 		\
	  src/data_translation/tuning.cpp		\

OBJECTS := (SOURCES:%.cpp=%.o)
DEPS := (SOURCES:%.cpp=%.h)

INC_PATH = -I midifile/include/				\
	   -I inc/music_objects/			\
	   -I inc/data_translation/

LIB_PATH = -L midifile/lib/ -l midifile

all: $(EXE)

$(EXE): main.cpp $(SOURCES)
	g++ $(CFLAGS) -std=c++11 $(INC_PATH) $(SOURCES) main.cpp $(LIB_PATH) -o $(EXE)
parser: parser.cpp
	g++ $(CFLAGS) -std=c++11 parser.cpp main.cpp -o parser
test: main.cpp midi2melody.cpp
	g++ $(CFLAGS) -std=c++11 $(INC_PATH) midi2melody.cpp main.cpp $(LIB_PATH) -o test
clean:
	rm -rf *o $(EXE) parser test
