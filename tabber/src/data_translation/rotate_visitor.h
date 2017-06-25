#ifndef __ROTATEVISITOR__
#define __ROTATEVISITOR__


#include "visitor.h"
#include "base.h"
#include <math.h>
#include <mutex>


class RotateVisitor : public Visitor
{

	private:
		vector< vector<pair<int, int> > > ValidChunkConfigurations;
		vector<int> locked_strings;
		std::mutex mtx_optima, mtx_cache;

	public:
  		virtual ~RotateVisitor() {}
  		void print_chunk_count();
    
		void CompareChunks(Chunk *c,vector<pair<int, int> >);
		virtual void VisitNote(Note* note);
		virtual void VisitBar(Bar* bar);

		virtual void VisitChunk(Chunk*);
		bool InCache(vector<pair<int, int> > input);
		void ClearCache(void);
		void RecursivelyFindLowestCostChunkConfiguration(Chunk* c);
		void ConditionallyAddToStack(comparisonResult, Chunk*, int&, int&);
};

#endif


