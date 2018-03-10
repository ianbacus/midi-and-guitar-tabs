#ifndef __NOTE__
#define __NOTE__

#include "base.h"
#include "tuning.h"

typedef struct
{
    uint32_t StringValue;
    uint32_t FretValue;
    
} FretboardPosition;

typedef std::map <int, vector< pair<int,int> >  > PitchMap;

class Note : public Base 
{
	private:
            static const uint32_t MaximumNumberOfOctaves = 30;
            static const uint32_t OctaveValueMidiPitches = 12;
            
            int TrackNumber;
            int PitchMidiValue;
            int NoteDurationBeats;
            int CurrentPitchMapRepositionIndex;
            uint32_t Repositions;
                
	public:
            static PitchMap PitchToFretMap;
            static int DeletedNotesCount;

            //Read pitchmap
            static int get_fret_at(const int mapIndex,const int pitchMidiValue);
            static int get_string_at(const int mapIndex, const int pitchMidiValue);

            Note(int pitch, int duration, int trackNumber); 
            virtual ~Note() {}

            //Getters
            int get_fret() const;
            int get_string() const;
            int get_current_note_index() const;
            int get_noteslost() const ;
            int get_delta()  const;
            int get_track_num() const;
            int get_pitch() const ;
            uint32_t GetAttemptedRepositions(void) const;
            virtual int GetNumberOfElements() const;

            //Setters
            void set_note_index(int mapIndex) ;
            void increment_note_index();
            void ResetAttemptedRepositions(void);

            void alter_pitch(int n);

            virtual void DispatchVisitor(Visitor* v);

};

#endif
