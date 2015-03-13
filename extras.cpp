//////////////////////////////
//
// MidiFile::read -- Parse a Standard MIDI File and store its contents
//      in the object.
//
// Borrowed from Craig Sapp

int MidiFile::read(const char* filename) {
   rwstatus = 1;
   timemapvalid = 0;
   if (filename != NULL) {
      setFilename(filename);
   }

   fstream input;
   input.open(filename, ios::binary | ios::in);

   if (!input.is_open()) {
      return 0;
   }

   rwstatus = MidiFile::read(input);
   return rwstatus;
}


//
// string version of read().
//


int MidiFile::read(const string& filename) {
   timemapvalid = 0;
   setFilename(filename);
   rwstatus = 1;

   fstream input;
   input.open(filename.data(), ios::binary | ios::in);

   if (!input.is_open()) {
      return 0;
   }

   rwstatus = MidiFile::read(input);
   return rwstatus;
}


//
// istream version of read().
//

int MidiFile::read(istream& input) {
   rwstatus = 1;
   if (input.peek() != 'M') {
      // If the first byte in the input stream is not 'M', then presume that
      // the MIDI file is in the binasc format which is an ASCII representation
      // of the MIDI file.  Convert the binasc content into binary content and
      // then continue reading with this function.
      stringstream binarydata;
      Binasc binasc;
      binasc.writeToBinary(binarydata, input);
      binarydata.seekg(0, ios_base::beg);
      if (binarydata.peek() != 'M') {
         cerr << "Bad MIDI data input" << endl;
	 rwstatus = 0;
         return rwstatus;
      } else {
         rwstatus = read(binarydata);
         return rwstatus;
      }
   }

   const char* filename = getFilename();

   int    character;
   // uchar  buffer[123456] = {0};
   ulong  longdata;
   ushort shortdata;


   // Read the MIDI header (4 bytes of ID, 4 byte data size,
   // anticipated 6 bytes of data.

   character = input.get();
   if (character == EOF) {
      cerr << "In file " << filename << ": unexpected end of file." << endl;
      cerr << "Expecting 'M' at first byte, but found nothing." << endl;
      rwstatus = 0; return rwstatus;
   } else if (character != 'M') {
      cerr << "File " << filename << " is not a MIDI file" << endl;
      cerr << "Expecting 'M' at first byte but got '"
           << character << "'" << endl;
      rwstatus = 0; return rwstatus;
   }

   character = input.get();
   if (character == EOF) {
      cerr << "In file " << filename << ": unexpected end of file." << endl;
      cerr << "Expecting 'T' at first byte, but found nothing." << endl;
      rwstatus = 0; return rwstatus;
   } else if (character != 'T') {
      cerr << "File " << filename << " is not a MIDI file" << endl;
      cerr << "Expecting 'T' at first byte but got '"
           << character << "'" << endl;
      rwstatus = 0; return rwstatus;
   }

   character = input.get();
   if (character == EOF) {
      cerr << "In file " << filename << ": unexpected end of file." << endl;
      cerr << "Expecting 'h' at first byte, but found nothing." << endl;
      rwstatus = 0; return rwstatus;
   } else if (character != 'h') {
      cerr << "File " << filename << " is not a MIDI file" << endl;
      cerr << "Expecting 'h' at first byte but got '"
           << character << "'" << endl;
      rwstatus = 0; return rwstatus;
   }

   character = input.get();
   if (character == EOF) {
      cerr << "In file " << filename << ": unexpected end of file." << endl;
      cerr << "Expecting 'd' at first byte, but found nothing." << endl;
      rwstatus = 0; return rwstatus;
   } else if (character != 'd') {
      cerr << "File " << filename << " is not a MIDI file" << endl;
      cerr << "Expecting 'd' at first byte but got '"
           << character << "'" << endl;
      rwstatus = 0; return rwstatus;
   }

   // read header size (allow larger header size?)
   longdata = MidiFile::readLittleEndian4Bytes(input);
   if (longdata != 6) {
      cerr << "File " << filename
           << " is not a MIDI 1.0 Standard MIDI file." << endl;
      cerr << "The header size is " << longdata << " bytes." << endl;
      rwstatus = 0; return rwstatus;
   }

   // Header parameter #1: format type
   int type;
   shortdata = MidiFile::readLittleEndian2Bytes(input);
   switch (shortdata) {
      case 0:
         type = 0;
         break;
      case 1:
         type = 1;
         break;
      case 2:    // Type-2 MIDI files should probably be allowed as well.
      default:
         cerr << "Error: cannot handle a type-" << shortdata
              << " MIDI file" << endl;
         rwstatus = 0; return rwstatus;
   }

   // Header parameter #2: track count
   int tracks;
   shortdata = MidiFile::readLittleEndian2Bytes(input);
   if (type == 0 && shortdata != 1) {
      cerr << "Error: Type 0 MIDI file can only contain one track" << endl;
      cerr << "Instead track count is: " << shortdata << endl;
      rwstatus = 0; return rwstatus;
   } else {
      tracks = shortdata;
   }
   clear();
   if (events[0] != NULL) {
      delete events[0];
   }
   events.resize(tracks);
   for (int z=0; z<tracks; z++) {
      events[z] = new MidiEventList;
      events[z]->reserve(10000);   // Initialize with 10,000 event storage.
      events[z]->clear();
   }

   // Header parameter #3: Ticks per quarter note
   shortdata = MidiFile::readLittleEndian2Bytes(input);
   if (shortdata >= 0x8000) {
      int framespersecond = ((!(shortdata >> 8))+1) & 0x00ff;
      int resolution      = shortdata & 0x00ff;
      switch (framespersecond) {
         case 232:  framespersecond = 24; break;
         case 231:  framespersecond = 25; break;
         case 227:  framespersecond = 29; break;
         case 226:  framespersecond = 30; break;
         default:
               cerr << "Warning: unknown FPS: " << framespersecond << endl;
               framespersecond = 255 - framespersecond + 1;
               cerr << "Setting FPS to " << framespersecond << endl;
      }
      // actually ticks per second (except for frame=29 (drop frame)):
      ticksPerQuarterNote = shortdata;

      cerr << "SMPTE ticks: " << ticksPerQuarterNote << " ticks/sec" << endl;
      cerr << "SMPTE frames per second: " << framespersecond << endl;
      cerr << "SMPTE frame resolution per frame: " << resolution << endl;
   }  else {
      ticksPerQuarterNote = shortdata;
   }


   //////////////////////////////////////////////////
   //
   // now read individual tracks:
   //

   uchar runningCommand;
   MidiEvent event;
   vector<uchar> bytes;
   int absticks;
   int xstatus;
   // int barline;

   for (int i=0; i<tracks; i++) {
      runningCommand = 0;

      // cout << "\nReading Track: " << i + 1 << flush;

      // read track header...

      character = input.get();
      if (character == EOF) {
         cerr << "In file " << filename << ": unexpected end of file." << endl;
         cerr << "Expecting 'M' at first byte in track, but found nothing."
              << endl;
         rwstatus = 0; return rwstatus;
      } else if (character != 'M') {
         cerr << "File " << filename << " is not a MIDI file" << endl;
         cerr << "Expecting 'M' at first byte in track but got '"
              << character << "'" << endl;
         rwstatus = 0; return rwstatus;
      }

      character = input.get();
      if (character == EOF) {
         cerr << "In file " << filename << ": unexpected end of file." << endl;
         cerr << "Expecting 'T' at first byte in track, but found nothing."
              << endl;
         rwstatus = 0; return rwstatus;
      } else if (character != 'T') {
         cerr << "File " << filename << " is not a MIDI file" << endl;
         cerr << "Expecting 'T' at first byte in track but got '"
              << character << "'" << endl;
         rwstatus = 0; return rwstatus;
      }

      character = input.get();
      if (character == EOF) {
         cerr << "In file " << filename << ": unexpected end of file." << endl;
         cerr << "Expecting 'r' at first byte in track, but found nothing."
              << endl;
         rwstatus = 0; return rwstatus;
      } else if (character != 'r') {
         cerr << "File " << filename << " is not a MIDI file" << endl;
         cerr << "Expecting 'r' at first byte in track but got '"
              << character << "'" << endl;
         rwstatus = 0; return rwstatus;
      }

      character = input.get();
      if (character == EOF) {
         cerr << "In file " << filename << ": unexpected end of file." << endl;
         cerr << "Expecting 'k' at first byte in track, but found nothing."
              << endl;
         rwstatus = 0; return rwstatus;
      } else if (character != 'k') {
         cerr << "File " << filename << " is not a MIDI file" << endl;
         cerr << "Expecting 'k' at first byte in track but got '"
              << character << "'" << endl;
         rwstatus = 0; return rwstatus;
      }

      // Now read track chunk size and throw it away because it is
      // not really necessary since the track MUST end with an
      // end of track meta event, and 50% of Midi files or so
      // do not correctly give the track size.
      longdata = MidiFile::readLittleEndian4Bytes(input);

      // set the size of the track allocation so that it might
      // approximately fit the data.
      events[i]->reserve(longdata/2);
      events[i]->clear();

      // process the track
      absticks = 0;
      // barline = 1;
      while (!input.eof()) {
         longdata = extractVlvTime(input);
         //cout << "ticks = " << longdata << endl;
         absticks += longdata;
         xstatus = extractMidiData(input, bytes, runningCommand);
         if (xstatus == 0) {
            rwstatus = 0;  return rwstatus;
         }
         event.setMessage(bytes);
         //cout << "command = " << hex << (int)event.data[0] << dec << endl;
         if (bytes[0] == 0xff && (bytes[1] == 1 ||
             bytes[1] == 2 || bytes[1] == 3 || bytes[1] == 4)) {
           // mididata.push_back('\0');
           // cout << '\t';
           // for (int m=0; m<event.data[2]; m++) {
           //    cout << event.data[m+3];
           // }
           // cout.flush();
         } else if (bytes[0] == 0xff && bytes[1] == 0x2f) {
            // end of track message
            // uncomment out the following three lines if you don't want
            // to see the end of track message (which is always required,
            // and added automatically when a MIDI is written.
            event.tick = absticks;
            event.track = i;
            events[i]->push_back(event);

            break;
         }

         if (bytes[0] != 0xff && bytes[0] != 0xf0) {
            event.tick = absticks;
            event.track = i;
            events[i]->push_back(event);
         } else {
            event.tick = absticks;
            event.track = i;
            events[i]->push_back(event);
         }

      }

   }

   theTimeState = TIME_STATE_ABSOLUTE;
   return 1;
}

