#include "Reader.h"

using namespace std;


//////////////////////////////
//
// Reader::Reader -- Constructor classes
//

Reader::Reader(void) : MidiMessage() {
   clearVariables();
}


Reader::Reader(int command) : MidiMessage(command)  {
   clearVariables();
}


Reader::Reader(int command, int p1) : MidiMessage(command, p1) {
   clearVariables();
}


Reader::Reader(int command, int p1, int p2) 
      : MidiMessage(command, p1, p2) {
   clearVariables();
}


Reader::Reader(int aTime, int aTrack, vector<uchar>& message)
      : MidiMessage(message) {
   tick  = aTime;
   track = aTrack;
   eventlink = NULL;
}


Reader::Reader(const Reader& mfevent) {
   tick  = mfevent.tick;
   track = mfevent.track;
   eventlink = NULL;
   this->resize(mfevent.size());
   for (int i=0; i<this->size(); i++) {
      (*this)[i] = mfevent[i];
   }
}



//////////////////////////////
//
// Reader::~Reader -- MidiFile Event destructor
//

Reader::~Reader() {
   tick  = -1;
   track = -1;
   this->resize(0);
   eventlink = NULL;
}


//////////////////////////////
//
// Reader::clearVariables --  Clear everything except MidiMessage data.
//

void Reader::clearVariables(void) {
   tick  = 0;
   track = 0;
   seconds = 0.0;
   eventlink = NULL;
}


//////////////////////////////
//
// Reader::operator= -- Copy the contents of another Reader.
//

Reader& Reader::operator=(Reader& mfevent) {
   if (this == &mfevent) {
      return *this;
   }
   tick  = mfevent.tick;
   track = mfevent.track;
   eventlink = NULL;
   this->resize(mfevent.size());
   for (int i=0; i<this->size(); i++) {
      (*this)[i] = mfevent[i];
   }
   return *this;
}


Reader& Reader::operator=(MidiMessage& message) {
   if (this == &message) {
      return *this;
   }
   clearVariables();
   this->resize(message.size());
   for (int i=0; i<this->size(); i++) {
      (*this)[i] = message[i];
   }
   return *this;
}


Reader& Reader::operator=(vector<uchar>& bytes) {
   clearVariables();
   this->resize(bytes.size());
   for (int i=0; i<this->size(); i++) {
      (*this)[i] = bytes[i];
   }
   return *this;
}


Reader& Reader::operator=(vector<char>& bytes) {
   clearVariables();
   setMessage(bytes);
   return *this;
}


Reader& Reader::operator=(vector<int>& bytes) {
   clearVariables();
   setMessage(bytes);
   return *this;
}



//////////////////////////////
//
// Reader::unlinkEvent -- Disassociate this event with another.
//   Also tell the other event to disassociate from this event.
//

void Reader::unlinkEvent(void) { 
   if (eventlink == NULL) {
      return;
   }
   Reader* mev = eventlink;
   eventlink = NULL;
   mev->unlinkEvent();
}



//////////////////////////////
//
// Reader::linkEvent -- Make a link between two messages.  
//   Unlinking
//

void Reader::linkEvent(Reader* mev) { 
   if (mev->eventlink != NULL) {
      // unlink other event if it is linked to something else;
      mev->unlinkEvent();
   }
   // if this is already linked to something else, then unlink:
   if (eventlink != NULL) {
      eventlink->unlinkEvent();
   }
   unlinkEvent();

   mev->eventlink = this;
   eventlink = mev;
}


void Reader::linkEvent(Reader& mev) { 
   linkEvent(&mev);
}



//////////////////////////////
//
// Reader::getLinkedEvent -- Returns a linked event.  Usually
//   this is the note-off message for a note-on message and vice-versa.
//   Returns null if there are no links.
//

Reader* Reader::getLinkedEvent(void) {
   return eventlink;
}



//////////////////////////////
//
// Reader::isLinked -- Returns true if there is an event which is not
//   NULL.  This function is similar to getLinkedEvent().
//

int Reader::isLinked(void) {
   return eventlink == NULL ? 0 : 1;
}



//////////////////////////////
//
// Reader::getTickDuration --  For linked events (note-ons and note-offs),
//    return the absolute tick time difference between the two events.
//    The tick values are presumed to be in absolute tick mode rather than
//    delta tick mode.  Returns 0 if not linked.
// 

int Reader::getTickDuration(void) {
   Reader* mev = getLinkedEvent();
   if (mev == NULL) {
      return 0;
   }
   int tick2 = mev->tick;
   if (tick2 > tick) {
      return tick2 - tick;
   } else {
      return tick - tick2;
   }
}



//////////////////////////////
//
// Reader::getDurationInSeconds -- For linked events (note-ons and 
//     note-offs), return the duration of the note in seconds.  The
//     seconds analysis must be done first; otherwise the duration will be
//     reported as zero.
//

double Reader::getDurationInSeconds(void) {
   Reader* mev = getLinkedEvent();
   if (mev == NULL) {
      return 0;
   }
   double seconds2 = mev->seconds;
   if (seconds2 > seconds) {
      return seconds2 - seconds;
   } else {
      return seconds - seconds2;
   }
}

