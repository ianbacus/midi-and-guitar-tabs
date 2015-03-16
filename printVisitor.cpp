#include "printvisitor.h"

void PrintVisitor::visitBar(Bar* b)
{
cout << "visitbar" << endl;
    char tuning[] = {'e','b','g','d','a','e'};
//  for(string_print_index=0; string_print_index<6; string_print_index++)
    for(string_print_index=7; string_print_index>= 0; string_print_index--)
  {
      cout << string_print_index << endl;
    if(string_print_index==7)
    {
        cout << "whitespace" <<endl;
      string_buffer[string_print_index] += " ";
      continue;
    }
    else
    {
        cout << "note" << endl;
      string_buffer[string_print_index] += "|";
      string_buffer[string_print_index].push_back(tuning[(string_print_index)]);
      
    }
    for(int j=0; j<b->get_children_size(); j++)
    {
      b->get_child(j)->accept(this);
    }
    //TODO: add logic to this to make the number of continuous bar prints variable by the client
    string_buffer[string_print_index] += "\n";
  }
}


void PrintVisitor::visitChunk(Chunk* c)
{
    cout << "chunking" << endl;
    bool strings_closed[] = {0,0,0,0,0,0};
  //TODO: logic for separating based on tick/notevalue
  // Currently, this filters out notes by the currently active string. The current printing method doesn't allow
  // multiple notes of the same chunk to appear at the same instant in time
  // - padding should use delta values of the chunk in terms of smallest note division for a bar
  for(int j=0; j<c->get_children_size(); j++)
  {
    Note* current_note = c->get_note_at(j);
    if(current_note->get_string() == string_print_index)
    {
     // cout << "chunk going to note" << endl;
     
      string_buffer[string_print_index] += "-"; //pre pad
      strings_closed[string_print_index] = 1;
      current_note->accept(this);
      
      string_buffer[string_print_index] += "-";
    }
//  else: update the stringsopen in reverse
  }
  for(int j=0; j<6; j++){
      if(!strings_closed[j])
       string_buffer[string_print_index] += "---"; //note width, and padding
       
  }
  
}

void PrintVisitor::visitNote(Note* n)
{
  int fret = n->get_fret();
  //cout << fret << endl;
  string_buffer[string_print_index] += std::to_string(fret);
 // cout << " string so far: ." << string_buffer[string_print_index] <<"." << endl;
  
}
