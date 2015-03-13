EXE = a
SOURCES = base.cpp bar.cpp chunk.cpp note.cpp
OBJECTS = main.o base.o bar.o chunk.o note.o
all: $(EXE)

$(EXE): tabber.h $(SOURCES)
	g++ $(CFLAGS) -std=c++11 $(SOURCES) -o $(EXE)
parser: parser.cpp
	g++ $(CFLAGS) -std=c++11 parser.cpp main.cpp -o parser
clean:
	rm -rf *o $(EXE) praser
