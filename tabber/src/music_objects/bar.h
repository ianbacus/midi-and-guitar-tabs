#include "base.h"
#include "chunk.h"


#ifndef __BAR__
#define __BAR__

class Bar : public Base
{
    private:
        vector<Chunk*> ChunkElements;

    public:
        virtual ~Bar();

        void PushBackElement(Chunk* c);
        void RemoveElement(Chunk* c);

        //retrieve a chunk from the bar by its index
        Chunk* GetElementWithIndex(int i);
        Chunk* GetLastElement();

        virtual void DispatchVisitor(Visitor*v);
        virtual int GetNumberOfElements() const;
};

#endif
