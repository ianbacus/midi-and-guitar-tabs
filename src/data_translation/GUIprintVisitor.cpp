#include "GUIprintvisitor.h"



GUIPrintVisitor::GUIPrintVisitor(std::string ofile) : string_print_index(0), strings_closed(false), tripled(false), outfile(ofile)
{
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2));
}
  	
void GUIPrintVisitor::visitBar(Bar* b)
{

	string_buffer.back()[0] += "[[";    		
	for(int j=0; j<b->get_children_size(); j++)
	{
		b->get_child(j)->accept(this);
	}
	string_buffer.back()[0] += "]],";
	tripled = false;
}

void GUIPrintVisitor::bar_ticks_reset(void) 
{
	bar_ticks = 0;
}

void GUIPrintVisitor::bar_ticks_increment(int d) 
{
	bar_ticks+=d;
}
    
void GUIPrintVisitor::newlines(void) 
{ 
}

void GUIPrintVisitor::visitChunk(Chunk* c)
{
/*
	Visit a chunk and iterate through all of its notes. Only visit notes which are on the currently set
	string index, which each enclosing bar decrements through. Decrementing is chosen arbitrarily (?)	

*/	
	int delta = 0;
	Note* current_note;
	int numChilds = c->get_children_size();
	if(c->get_children_size() == 0){
		return;
	}
//	string_buffer.back()[0] += "\n\t";
	for(int j=0; j<c->get_children_size(); j++)
	{
		delta += c->get_note_at(j)->get_delta();
	}
	for(int i=0;i<=delta;i++)
		string_buffer.back()[0] += "p," ;

	if(numChilds > 1) 
		string_buffer.back()[0] += std::string("[");

	for(int j=0; j<c->get_children_size(); j++)
	{
		current_note = c->get_note_at(j);
		current_note->accept(this);	
	}
	if(numChilds > 1)
	{
		string_buffer.back()[0] += std::string("]");
		string_buffer.back()[0] += ",";
	}
}

void GUIPrintVisitor::visitNote(Note* n)
{
	int delta = 0;
	std::string result = "TabNote(" + std::to_string(n->get_string()) + "," + std::to_string(n->get_fret()) + "),";	
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

void GUIPrintVisitor::print_out(void)
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
