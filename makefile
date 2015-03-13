EXE = a
SOURCES = base.cpp bar.cpp chunk.cpp note.cpp
OBJECTS = main.o base.o bar.o chunk.o note.o
all: $(EXE)

$(EXE): tabber.h $(SOURCES)
	gcc $(CFLAGS) -std=c++11 $(SOURCES) -o $(EXE)
parser: parser.c
	gcc $(CFLAGS) -std=c++11 parser.c main.c -o parser
clean:
	rm -rf *o $(EXE)
