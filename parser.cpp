#include "reader.h"

using namespace std;

Reader::Reader (string fn)
{
  index = 0;
  header = 0;
  format=0;
  tracks=0;
  time_div=0;
  time_sig=0; 
  tempo=0;
  	
  string filename = fn;
 // generate_map();
  
    ifstream temp(fn, ios::binary );
    bin.swap(temp);
    
  if (!bin)
  {
    cout << "Could not open \"" << filename << "\".\n";
    
  }

}

void Reader::read_midi_event(unsigned int event)
//Reads either 1 or 2 bytes, depending on subtype of midi event.
//Event type is passed in and split into event and channel information.
{
	unsigned int p1,p2,channel,type;
	type = (event >> 4);
	channel = event & 0x0F;
	read_bytes_to_int(1,p1);
	cout << hex << p1;
	if(type == 0x9 or type == 0xA)
	{
		cout << "note:" << p1 << endl;
		discard_bytes(1);
	}
	else if(type == 0xC or type == 0xD)
	{
//		cout <<
	}
	else
	{
//		cout << "shitnote" << endl;
		discard_bytes(1);
	}
}

void Reader::read_sys_event()
//Always a variable length of data followed by the data
{
	unsigned int data;
	unsigned int len = read_varlen();
	read_bytes_to_int(len, data); 
	
}

void Reader::read_meta_event(bool &end)
//Meta-event contains a type, a variable or single byte length, and data.
{
	unsigned int type=0, data=0,len=0;
	read_bytes_to_int(1,type);
	cout <<"type"<< (hex) << type << ": ";
	switch(type)
	{
		case 0x7F:
			len = read_varlen();
			discard_bytes(len);
			break;
		case 0x2F:
			end = true;
			discard_bytes(1);
			cout << "**********" << endl;
			break;
		default:
			if(type <= 0x07 && type >= 0x01)
			{
				string str ="";
				len = read_varlen();
				read_bytes_to_char(len, str);
				cout << "string: " << str << endl;
			}
			else
			{
				read_bytes_to_int(-1,data);
			}
			break;			
	}
	if(end)
	{
		cout <<"this is the end.";
	}
}
/*
void Reader::generate_map()
{
  int it;
//  string chrom_scale{'C','1', 'D' ,'3','E', 'F', '6','G', '7','A', '9','B' };
  string chrom_scale ("C1D3EF6G7A9B") ;
  for(int in=1;in<127;in++)
  {
    if(it == 11)
    {
      it=0;
    }
    char n = chrom_scale[it];
    notemap.insert ( pair<int,char>(in,n) );
    it++;
  }
}
*/


unsigned int Reader::read_varlen()
{
/* Read a variable length byte */
    char byte;
    int b, c;
    bin.get(byte);
    index++;
    b = (unsigned char)byte;
    if ( b & 0x80 ) //if MSB is 1... (MSB of 1 means that the following \
    //byte will be a continuation, not a separate piece of data)
    {
       b &= 0x7F; // strip MSB
       do
       {
         index++;
         bin.get(byte);
         c = (unsigned char)byte; // This will be the "extended" portion of the byte
         b = (b << 7) + (c & 0x7F);
       } while (c & 0x80);
    }
    return(b);
}

void Reader::discard_bytes(int range)
{
  char byte;
  bin.get(byte);
    
  if(range == -1)  //to read entire chunk based on size?
    range = (unsigned char)byte+1;

  index += range;
  for(int in = 0; in<range-1;++in)
  {
    bin.get(byte);
  }
}

void Reader::read_bytes_to_int(int range,unsigned int &ret)
// This function will convert all the bytes in range into an integer and store that in ret.
// Inputting a range value -1 will instruct the function to use the first byte as a range indicator, and it will call itself
{
	char byte;
	bin.get(byte);

	if(range == -1)  //to read entire chunk based on size?
      range = (unsigned char)byte+1;
      
	else
    	ret = (ret<<8) + (unsigned char)byte;
	index += range;
	for(int in = 0; in<range-1;in++)
	{
		bin.get(byte);
    	ret = (ret<<8) + (unsigned char)byte;
	}
}


void Reader::read_bytes_to_char( int range, string &ret)
// This function will convert all the bytes in range into characters of a string.
// Inputting a range value -1 will instruct the function to use the first byte as a range indicator, and it will call itself
{
  char byte;
  bin.get(byte);

  if(range == -1)//reads a length byte and automatically reads for how long it specifies
  {
    range = (unsigned char)byte+1;
  }
  else
  	{	  
  		int b = (unsigned char) byte;
		ret += b;
	}
  index += range;
  for(int in=0;in<range-1;in++)
  {
	  bin.get(byte);
	  int b = (unsigned char) byte;
	  ret += b;
  }
}

void Reader::read_track_chunk()
// Reads four bytes to indicate length of the chunk in bytes.
//Continuously read variable length delta followed by event data.
//Event data is stored in 3 types, with a separate function to handle each type.
{

  char byte;
  unsigned int chunk_len,event,delta;
  bool end;
  read_bytes_to_int(4,chunk_len); //chunk length is always 4 bytes

  cout << "chunklen:" << chunk_len << endl;
  end = 0;
  while(!end and index<300)
  {
    event = 0;
    delta = read_varlen(); //deltas are variable length
    cout << "delta: " << delta << endl;
	
	cout << "index:" << dec << index << " ";
	read_bytes_to_int(1,event);
	//cout << (hex) << event << endl;
	
	switch(event & 0xf0)
	{
	  case 0x80:        // note off (2 more bytes)
      case 0x90:        // note on (2 more bytes)
      case 0xA0:        // aftertouch (2 more bytes)
      case 0xB0:        // cont. controller (2 more bytes)
      case 0xE0:        // pitch wheel (2 more bytes)
      //hmmm
	  case 0xC0:        // patch change (1 more byte)
      case 0xD0:        // channel pressure (1 more byte)
			cout << "midi - " << (hex) << event <<" ";
			read_midi_event(event);
			break;
	  
	  
	  case 0xF0:
         switch (event) {
            case 0xff:                 // meta event
				cout << "meta - ";
				read_meta_event(end);
				break;
            case 0xf0:                // sysex
               // read until you find a 0xf7 character
               	cout << "sys - ";
				read_sys_event();
				break;
               /*
               byte = 0;
               while (byte != 0xf7 && !bin.eof()) {
                   read_bytes_to_int(1,byte);  // meta data
               }
               break;*/
         }
         break; 
      default:
		  break;
	}
	//cout << "***************";
  }
}

void Reader::read_header_chunk()
{

	unsigned int chunk_len;
	cout << "***************";
    read_bytes_to_int(4,chunk_len);
	read_bytes_to_int(2,format);
	read_bytes_to_int(2,tracks);
	read_bytes_to_int(2,time_div);
  cout << "\nChunk length (should be 6):" << chunk_len<< "\nFormat: "<< \
  format<< "\nTrack chunks: "<<tracks;
	if(time_div & 0x8000)
	{
	cout << "\nregular deltas" << endl;
		//bits 0-14: delta time units per quarter note	
	}
	else
	{
	cout << "\nSMPTE deltas" << endl;
		//bits 0-7: number of delta units per SMPTPE frame
		//bits 8-14: negative(ignore sign bit) number representing the number of SMPTE frames per second
	}
//	discard_bytes( chunk_len - 6 ); //discard extra data
	cout << "***************\n";

}
