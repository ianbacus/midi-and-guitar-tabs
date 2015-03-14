#include "printvisitor.h"

void printVisitor::visitBar(Bar* b)
{

  for(string_print_index=1; string_print_index<6; string_print_index++)
  {
    if(string_print_index==0)
    {
      string_buffer[string_print_index] += " ";
    }
    else
    {
      string_buffer[string_print_index] += "|";
    }
    for(int j=0; j<b->get_children_size(); j++)
    {
      b->get_child(j)->accept(this);
    }
    //TODO: add logic to this to make the number of continuous bar prints variable by the client
    string_buffer[string_print_index] += "\n";
  }
}


void printVisitor::visitChunk(Chunk* c)
{
  //TODO: logic for separating based on tick/notevalue
  for(int j=0; j<c->get_children_size(); j++)
  {
    Note* current_note = get_note_at(j);
    if(current_note->get_string() == string_print_index)
    {
      current_note->accept(this);
    }
  }
  
}

void printVisitor::visitNote(Note* n)
{
  string_buffer[string_print_index] += n->get_fret();
}
