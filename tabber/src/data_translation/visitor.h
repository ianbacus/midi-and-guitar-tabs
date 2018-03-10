#ifndef __VISITOR__
#define __VISITOR__

#include <bar.h>
#include <chunk.h>
#include <note.h>

//Forward declarations to allow inheritance of this class and methods that act on pointers \
to the base class:
class Note;
class Bar;
class Chunk;


class Visitor
{
    public:
        virtual void VisitNote(Note*)=0;
        virtual void VisitBar(Bar*)=0;
        virtual void VisitChunk(Chunk*)=0;
};


#endif
