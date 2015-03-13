
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
	unsigned int format, tracks, time_div, time_sig, tempo;
	ifstream bin;
	
public:

	void read_header_chunk();
	void read_track_chunk();
	void read_varlen();
	void read_bytes_to_char(ifstream &bin,int range);
	void read_bytes_to_int(ifstream &bin, int range,unsigned int &ret);
	void discard_bytes(int range);
	
	Reader(ifstream &b);
};


