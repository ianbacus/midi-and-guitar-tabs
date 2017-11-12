#ifndef __NOTE__
#define __NOTE__
#include "base.h"

#include "tuning.h"

typedef std::map <int, vector< pair<int,int> >  > PitchMap;

class Note : public Base 
{

    private:
        
	 	static PitchMap pitch_to_frets_map;
        static const unsigned int PitchOctave = 12;
        static const unsigned int MaximumOctavesCount = 10;
		static int noteslost;
        
        //Pitch map methods
        static PitchMap ConfigurePitchMap(void);
 
    public:
    
		static int get_fret_at(int note_index, int pitch);
		static int get_string_at(int note_index, int pitch);
        
 	private:
		int TrackNumber;
		int Pitch;
		int Delta;
		int NotePositionPitchMapIndex;
		int NumberOfOctaveOffsets;       
        
	public:
		
		Note(int pitch, int delta, int trackNumber); 
		virtual ~Note(void);
		int get_fret(void) ;
		
		
		int get_string(void);
		int get_noteslost(void) const ;
		void set_note_index(int n) ;
		int get_note_position_pitch_map_index(void) const;
		void increment_note_index(void);
		void decrement_octave(void);
		void increment_octave(void);
		void rebalance_note(void);
		
		int get_track_number(void) const;
		int get_pitch(void) const ;
		void alter_pitch(int n);
		int get_delta(void)  const;
		
		virtual void accept(Visitor* vistior);
		virtual int get_children_size(void) const;


};

#endif
