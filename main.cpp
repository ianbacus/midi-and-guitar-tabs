#include "reader.h"

void read_file(Reader &ro)
{
	unsigned int track_cnt = 0;
	read_chunk(ro);
	while(track_cnt < 2) //ro.get_tracks())
	{
		read_chunk(ro);
		track_cnt++;
	}
}

int main()
{
  string fn ("canon.mid");
  Reader robj(fn);
  read_file(robj);
  return 0;
}
