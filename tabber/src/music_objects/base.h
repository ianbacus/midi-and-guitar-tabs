#ifndef __BASE__
#define __BASE__

#include <cstdint>

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>

//typedef vector<pair<int,int> * > coordinate_set;

class Visitor;

class Base
{
    public:
        Base() {}
        virtual ~Base() {}
        
        virtual void DispatchVisitor(Visitor*)=0;
        virtual uint32_t GetNumberOfElements() const=0;
        
//        vector<Base*> GetElements(void) const;
//        Base* GetElementWithIndex(uint32_t index) const;
//        Base* GetElement(void) const;
};

#include "visitor.h"
#include "note.h"
#include "bar.h"
#include "chunk.h"


#endif

