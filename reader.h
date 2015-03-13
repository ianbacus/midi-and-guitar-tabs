
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <map>
using namespace std;

class Reader {

private:
	int index;
	unsignec int index, header,format, tracks, time_div, time_sig, tempo;
	ifstream bin;
	

public:

	Reader(ifstream &b);
	~Reader() = default;
	generate_map();
	
	void read_header_chunk();
	void read_track_chunk();
	void read_varlen();
	void read_bytes_to_char(ifstream &bin,int range);
	void read_bytes_to_int(ifstream &bin, int range,unsigned int &ret);
	void discard_bytes(int range);
	
	void read_midi_event(unsigned int event);
	void read_sys_event();
	void read_meta_event(bool &end);
	
};


