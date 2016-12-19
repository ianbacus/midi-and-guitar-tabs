#include "printvisitor.h"



PrintVisitor::PrintVisitor(std::string ofile, int cset) : string_print_index(0), \
							strings_closed(false), tripled(false), outfile(ofile), columnSet(cset)
{
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2));
}

void PrintVisitor::visitBar(Bar* b)
{
    for(string_print_index=SIZEOF_TUNING; string_print_index>= 0; string_print_index--)
    {
    	if(string_print_index == SIZEOF_TUNING) 
    	{
    		string_buffer.back()[SIZEOF_TUNING] += "   ";
    		continue;
    	}
    	(string_buffer.back())[string_print_index] += "|";
		for(int j=0; j<b->get_children_size(); j++)
		{
			b->get_child(j)->accept(this);
		}
		(string_buffer.back())[string_print_index] += "-";
		if(!tripled) 
		{
			string_buffer.back()[SIZEOF_TUNING] += "   ";
		}
		tripled = false;
		
		
		
	}
	
	
}

int PrintVisitor::get_columnSet(void)
{
	return columnSet;
}

void PrintVisitor::bar_ticks_reset(void) 
{
	bar_ticks = 0;
}

void PrintVisitor::bar_ticks_increment(int d) 
{
	bar_ticks+=d;
}
    
void PrintVisitor::overflowLines(void)
{
	columnIndex = 0;
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2) );
	//newlines();
}
void PrintVisitor::newlines(void) 
{ 
	//Push the end of line columns
	for(string_print_index=SIZEOF_TUNING+1; string_print_index>= 0; string_print_index--){
		if(string_print_index>=SIZEOF_TUNING) 
		{
			string_buffer.back()[string_print_index] += " ";
		}
		else
		{
			string_buffer.back()[string_print_index] += "|";
		}
	}
	//Push a new vector of strings to be printed on new lines
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2) );
	
	//Push open string values for the new lines
	for(string_print_index=SIZEOF_TUNING+1; string_print_index>= 0; string_print_index--)
	{
		if(string_print_index>=SIZEOF_TUNING) 
		{
			string_buffer.back()[string_print_index] += " ";
		}
		else
		{
			string_buffer.back()[string_print_index] += "|";
			string_buffer.back()[string_print_index].push_back(ptuning[(string_print_index)]);
		}
	}
}

void PrintVisitor::visitChunk(Chunk* c)
{
/*
	Visit a chunk and iterate through all of its notes. Only visit notes which are on the currently set
	string index, which each enclosing bar decrements through. Decrementing is chosen arbitrarily (?)	

*/
	strings_closed=false;
	bool locked = false;
	
	int delta = 0;
	string result;
	Note* current_note;
	int j = c->get_children_size();
	if(c->get_children_size() == 0)
		return;
	for(int j=0; j<c->get_children_size(); j++)
	{
		current_note = c->get_note_at(j);
		if(!strings_closed)
		{
			current_note->accept(this);
		}
		if(strings_closed && !locked)
		{
			int fret = current_note->get_fret();
			if(fret < 10)
				result = "-" + std::to_string(fret);
			else
				result = std::to_string(fret);		
			locked = true;
		}
		delta += current_note->get_delta();
	}
	if(!strings_closed)//none of the chunk members were on the current string index, so print null space instead
	{
		result = "--" ;
	}	
	//This is where the padding takes place for notes based on their note duration
	if(delta >= 0)
	{	
		string_buffer.back()[string_print_index] += std::string(delta,'-') + result;
	}
	else 
	{
		//triplets case
		tripled = true;
		//string_buffer.back()[SIZEOF_TUNING] += " t";
		string_buffer.back()[string_print_index] +=  result;
	}
	
}

void PrintVisitor::visitNote(Note* n)
{
	if(n->get_string() == string_print_index)
	{
    	strings_closed = true; 
	}

}

void PrintVisitor::print_out(void)
{
	ofstream ofile;
	ofile.open(outfile);
	stringstream ss;
	int cindex = 0;
	bool freshlines = false;
	for(std::vector< vector<string> >::iterator it = string_buffer.begin() ; it != string_buffer.end(); ++it) 
	{
		if((cindex+((*it)[1]).size()) > columnSet)
		{
			freshlines = true;
			cindex = 0;
		}
		for(int i=SIZEOF_TUNING; i>=0; i--)
		{
			#ifdef brokenstring
			if(i == brokenstring)
			{
				ss << " " << '\n';
			}
			#endif
			if(freshlines)
			{
				//(*it)[i].push_back('\n');
				 //ss << '\n';
			}
			ss << (*it)[i] << '\n';
		}
		cindex += ((*it)[1]).size();
		freshlines = false;
		
	}
	ofile << ss.rdbuf();
	ofile.close();

}
