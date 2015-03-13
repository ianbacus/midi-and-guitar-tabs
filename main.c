#include "parser.h"


int main()
{
  string fn ("canon.mid");
  Reader robj(fn);
  read_file(robj);
  return 0;
}
