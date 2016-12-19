#include "printvisitor.h"
using std::cout;
using std::endl;
map<int,int> beat_value = {{1,32},{2,16},{4,8},{8,4},{16,2},{32,1}};

std::map<int,string> quaver_map = 
{
{2," T"},{4, " s"},{6,".s"},{8," e"},{14,".e"},{16, " Q"},{24, ".Q"},{32," H"},{56,".H"},{64, " W"},
};

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
//<<<<<<< HEAD:tabber/src/data_translation/printVisitor.cpp
    	(string_buffer.back())[string_print_index] += "|-";
		for(int j=0; j<b->get_children_size(); j++)
		{

			b->get_child(j)->accept(this);
			int delta=0;
			Chunk*  Nextchunk;
		
			if(j<(b->get_children_size()-1))
			{
				Nextchunk = b->get_child(j+1);
				
				int nextchunksize = Nextchunk->get_children_size();
				if(nextchunksize == 0)
					continue;
				for(int j=0; j<nextchunksize; j++)
				{
					delta+= Nextchunk->get_note_at(j)->get_delta();;
				}
				if(string_print_index == 0)
				{
					if(!tripled)
					{ 
						if(quaver_map.find(delta) != quaver_map.end()) //item is in our map
						{
							string_buffer.back()[SIZEOF_TUNING] += quaver_map[delta]+ std::string(delta,' ');
						}
						else
						{
							int extra_delta=0;
							//split the delta, render enough empty spaces 
							//TODO: find a better (more general) way to handle unanticipated note durations
							std::cout << "stuck: delta=" << delta <<std::endl;
							if(delta<0) delta *= -2;
							while(quaver_map.find(delta) == quaver_map.end())
							{
								delta--;
								extra_delta++;
							}
							string_buffer.back()[SIZEOF_TUNING] += quaver_map[delta]+ std::string(delta,' ');
							string_buffer.back()[SIZEOF_TUNING] += quaver_map[extra_delta]+ std::string(extra_delta,' ');
							
						}
						
					}
					else if(string_print_index == 0) string_buffer.back()[SIZEOF_TUNING] += " t";
				}
			}
		}
		(string_buffer.back())[string_print_index] += "-";
		
/*		
=======

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
>>>>>>> temp:src/data_translation/printVisitor.cpp
*/
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
    
//<<<<<<< HEAD:tabber/src/data_translation/printVisitor.cpp
void PrintVisitor::newlines(bool fresh=false) 
{ 
	if(fresh == false)
	{
		for(string_print_index=SIZEOF_TUNING+1; string_print_index>= 0; string_print_index--){
			if(string_print_index>=SIZEOF_TUNING) string_buffer.back()[string_print_index] += " ";
			else
				string_buffer.back()[string_print_index] += "|";
		}
/*
=======
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
>>>>>>> temp:src/data_translation/printVisitor.cpp
*/
	string_buffer.push_back(vector<string>(SIZEOF_TUNING+2) );
	}
	
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
			int pitch = current_note->get_pitch();
			if (fret < 0) //rest note
				result = "--";
			else if(fret < 10)
				result = "-" + std::to_string(fret);
			else
				result = std::to_string(fret);
			//New test for ascii output
			//result = std::to_string(fret);
			locked = true;
		}
		delta += current_note->get_delta();
	}
	if(!strings_closed)//none of the chunk members were on the current string index, so print null space instead
	{
		result = "--" ;
	}	
	//This is where the padding takes place for notes based on their note duration
//<<<<<<< HEAD:tabber/src/data_translation/printVisitor.cpp
	if(delta >= 0){
		
		string_buffer.back()[string_print_index] += result+ std::string(delta,'-');
/*
=======
	if(delta >= 0)
	{	
		string_buffer.back()[string_print_index] += std::string(delta,'-') + result;
>>>>>>> temp:src/data_translation/printVisitor.cpp
*/
	}
	else 
	{
		//triplets case
		tripled = true;
//<<<<<<< HEAD:tabber/src/data_translation/printVisitor.cpp
		
		string_buffer.back()[string_print_index] +=  result;
	}
/*=======
		//string_buffer.back()[SIZEOF_TUNING] += " t";
		string_buffer.back()[string_print_index] +=  result;
	}
	
>>>>>>> temp:src/data_translation/printVisitor.cpp*/
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
//<<<<<<< HEAD:tabber/src/data_translation/printVisitor.cpp
		ss << '\n';
/*=======
		cindex += ((*it)[1]).size();
		freshlines = false;
		
>>>>>>> temp:src/data_translation/printVisitor.cpp
*/
	}
	ofile << ss.rdbuf();
	ofile.close();
}
