EXE = gen
SOURCES = src/music_objects/bar.cpp                     \
	  src/music_objects/chunk.cpp                   \
	  src/music_objects/note.cpp                    \
	  src/data_translation/rotatevisitor.cpp        \
	  src/data_translation/printvisitor.cpp         \
	  src/data_translation/midi2melody.cpp          \
	  src/data_translation/tuning.cpp               \
	  src/main.cpp
OBJECTS := (SOURCES:%.cpp=%.o)

INC_PATH = -I midifile/include/                         \
	   -I inc/music_objects/                        \
	   -I inc/data_translation/

all: $(EXE)
$(EXE): $(SOURCES)
	g++ $(CFLAGS) -std=c++11 $(INC_PATH) $(SOURCES) $(LIB_PATH) -o $(EXE)
DEBUG: $(SOURCES)
	g++ $(CFLAGS) -std=c++11 $(INC_PATH) $(SOURCES) $(LIB_PATH) -g -o $(EXE)
	
clean:
	rm -rf *.o $(EXE)
