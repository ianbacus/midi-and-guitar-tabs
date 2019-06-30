#include "GUIprintvisitor.h"

/*
 *
 */
GUIPrintVisitor::GUIPrintVisitor(void) : 
string_print_index(0), strings_closed(false), tripled(false)
{
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2));
}

/*
 *
 */
void GUIPrintVisitor::VisitBar(Bar* b)
{

	string_buffer.back()[0] += "[[";
	for(int j=0; j<b->GetNumberOfElements(); j++)
	{
		b->GetElementWithIndex(j)->DispatchVisitor(this);
	}
	string_buffer.back()[0] += "]],";
	tripled = false;
}

/*
 *
 */
void GUIPrintVisitor::bar_ticks_reset(void)
{
	bar_ticks = 0;
}

/*
 *
 */
void GUIPrintVisitor::bar_ticks_increment(int d)
{
	bar_ticks+=d;
}

/*
 *
 */
void GUIPrintVisitor::newlines(void)
{
}

/*
 *
 */
void GUIPrintVisitor::VisitChunk(Chunk* c)
{
/*
	Visit a chunk and iterate through all of its notes. Only visit notes which are on the currently set
	string index, which each enclosing bar decrements through. Decrementing is chosen arbitrarily (?)

*/
	int delta = 0;
	Note* current_note;
	int numChilds = c->GetNumberOfElements();
	if(c->GetNumberOfElements() == 0){
		return;
	}
	for(int j=0; j<c->GetNumberOfElements(); j++)
	{
		delta += c->GetElementWithIndex(j)->GetNoteDurationBeats();
	}
	for(int i=0;i<=delta;i++)
		string_buffer.back()[0] += "p," ;

	if(numChilds > 1)
		string_buffer.back()[0] += std::string("[");

	for(int j=0; j<c->GetNumberOfElements(); j++)
	{
		current_note = c->GetElementWithIndex(j);
		current_note->DispatchVisitor(this);
	}
	if(numChilds > 1)
	{
		string_buffer.back()[0] += std::string("]");
		string_buffer.back()[0] += ",";
	}
}

/*
 *
 */
void GUIPrintVisitor::VisitNote(Note* n)
{
	int delta = 0;
	std::string result = "TabNote(" + std::to_string(n->GetStringIndexForCurrentNotePosition()) + "," + std::to_string(n->GetFretForCurrentNotePosition()) + "),";
	//This is where the padding takes place for notes based on their note duration
	if(delta >= 0)
	{
		string_buffer.back()[0] += result;
	}
	else
	{
		//triplets case
		tripled = true;
		string_buffer.back()[0] +=  result;
	}

}

void WriteTablatureToOutputString(string& x)
{
	
}
/*
 *
 */
void GUIPrintVisitor::WriteTablatureToOutputFile(string outfile)
{
	ofstream ofile;
	ofile.open(outfile);
	stringstream ss;
	for(std::vector< vector<string> >::iterator it = string_buffer.begin() ; it != string_buffer.end(); ++it)
	{
		for(int i=SIZEOF_TUNING; i>=0; i--)
		{
			ss << (*it)[i];
		}
	}
	ofile << ss.rdbuf();
	ofile.close();

}
