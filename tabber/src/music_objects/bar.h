#include "base.h"
#include "chunk.h"


#ifndef __BAR__
#define __BAR__

class Bar : public Base
{
    private:
        vector<Chunk*> Chunks;

    public:
        virtual ~Bar();

        void PushBackElement(Chunk* c);
        void RemoveElement(Chunk* c);

        //retrieve a chunk from the bar by its index
        Chunk* GetElementWithIndex(uint32_t i) const;
        Chunk* GetLastElement(void) const;
        vector<Chunk*> GetElements(void) const;

        virtual void DispatchVisitor(Visitor*v);
        virtual uint32_t GetNumberOfElements() const;
};

#endif
