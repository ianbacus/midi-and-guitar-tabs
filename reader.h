#ifndef __READER__
#define __READER__
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <string>
#include <vector>
#include <map>
using namespace std;

class Reader {

private:
	int index;
	unsigned int header,format, tracks, time_div, time_sig, tempo;
	ifstream bin;
	

public:

	Reader(string );
	~Reader() = default;
//	generate_map();
	
		
	void read_midi_event(unsigned int event);
	void read_sys_event();
	void read_meta_event(bool &end);
	
	unsigned int read_varlen();
	void discard_bytes(int range);


	void read_bytes_to_int(int range,unsigned int &ret);
	void read_bytes_to_char(int range,string &);
	
	void read_track_chunk();
	void read_header_chunk();
	
	
	//EXTRAS
/*	
	
	int readMidiEvent(ostream& out, int& trackbytes, int& command);
	int read(const char* filename);
	int read(const string& filename);
	int read(istream& input);
*/
};

#endif

