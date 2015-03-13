#include "reader.h"
#include "Options.h"
#include "MidiFile.h"


using namespace std;

void read_chunk(Reader &ro)
{
  
	string type;
	string mtrk = "MTrk";
	string mthd = "MThd";
	
	ro.read_bytes_to_char(4,type);
	cout << type << endl;
//	if(type.compare(mthd) == 0)
	if(type == "MThd")
	{
	cout << "reading header chunk" << endl;
		ro.read_header_chunk();
	}
//	else if(type.compare(mtrk) == 0)
	else if(type == "MTrk")
	{
	cout << "reading track chunk" << endl;
		ro.read_track_chunk();
	}
	else
	{
	  cout << "fuckup" << endl;
	}

}


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
 // Reader robj(fn);
  //read_file(robj);
  
MidiFile midifile;
   int tracks = midifile.getTrackCount();
   midifile.read(fn)
  return 0;
}
