#include "printvisitor.h"

void PrintVisitor::visitBar(Bar* b)
{
    char tuning[] = {'e','a','d','g','b','e'};
//  for(string_print_index=0; string_print_index<6; string_print_index++)
    for(string_print_index=6; string_print_index>= 0; string_print_index--)
  {
    //  cout << string_print_index << endl;
/*    if(string_print_index==6)
    {
      string_buffer[string_print_index] += " ";
      continue;
    }
    else*/
//    {
      string_buffer[string_print_index] += "|";
      string_buffer[string_print_index].push_back(tuning[(string_print_index)]);
      
//    }
    for(int j=0; j<b->get_children_size(); j++)
    {
        //cout << j << endl;
      b->get_child(j)->accept(this);
    }
    //TODO: add logic to this to make the number of continuous bar prints variable by the client
    string_buffer[string_print_index] += "\n";
  }
}


void PrintVisitor::visitChunk(Chunk* c)
{
    bool strings_closed=false;
  //TODO: logic for separating based on tick/notevalue
  // Currently, this filters out notes by the currently active string. The current printing method doesn't allow
  // multiple notes of the same chunk to appear at the same instant in time
  // - padding should use delta values of the chunk in terms of smallest note division for a bar
  for(int j=0; j<c->get_children_size(); j++)
  {
    //only print out the notes in the chunk on the current string being printed. If none of the notes in the chunk 
    // match the current string index, then pad the space instead.
    Note* current_note = c->get_note_at(j);
    if(current_note->get_string() == string_print_index)
    {
     // cout << "chunk going to note" << endl;
     
      string_buffer[string_print_index] += "-"; //pre pad
      strings_closed = true;
      current_note->accept(this);
      
      string_buffer[string_print_index] += "-";
    }
//  else: update the stringsopen in reverse
  }
  if(!strings_closed)
    string_buffer[string_print_index] += "---"; //note width, and padding

}

void PrintVisitor::visitNote(Note* n)
{
  int fret = n->get_fret();
  //cout << fret << endl;
  string_buffer[string_print_index] += std::to_string(fret);
 // cout << " string so far: ." << string_buffer[string_print_index] <<"." << endl;
  
}
