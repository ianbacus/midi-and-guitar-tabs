#ifndef __BASE__
#define __BASE__

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>

#include "visitor.h"

using namespace std;

typedef vector<pair<int,int> * > coordinate_set;

class Visitor;

class Base
{
    public:
		Base(void) {}
		virtual ~Base(void) {}
		virtual void accept(Visitor* visitor)=0;
		virtual int get_children_size(void) const=0;		
};


#endif

