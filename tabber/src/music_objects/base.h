#ifndef __BASE__
#define __BASE__


#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
using namespace std;


typedef vector<pair<int,int> * > coordinate_set;

class Visitor;

class Base
{

    public:
		Base() {}
		virtual ~Base() {}
		virtual void accept(Visitor*)=0;
		virtual int get_children_size() const=0;		
};

#include "visitor.h"
#include "note.h"
#include "bar.h"
#include "chunk.h"


#endif

