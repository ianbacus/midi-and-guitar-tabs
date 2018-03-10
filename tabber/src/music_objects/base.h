#ifndef __BASE__
#define __BASE__


#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;


//typedef vector<pair<int,int> * > coordinate_set;

class Visitor;

class Base
{
    public:
        Base() {}
        virtual ~Base() {}
        virtual void DispatchVisitor(Visitor*)=0;
        virtual int GetNumberOfElements() const=0;		
};

#include "visitor.h"
#include "note.h"
#include "bar.h"
#include "chunk.h"


#endif

