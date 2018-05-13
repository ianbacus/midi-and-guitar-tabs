#ifndef __NOTE__
#define __NOTE__

#include "base.h"
#include <map>
#include <vector>

using std::vector;

typedef struct FretboardPosition_t
{
    uint32_t StringIndex;
    uint32_t FretNumber;
    
    FretboardPosition_t(uint32_t string, uint32_t fret)
        :
            StringIndex(string), FretNumber(fret)
    {
        
    }
    
} FretboardPosition;

inline bool operator==(const FretboardPosition& lhs, const FretboardPosition& rhs)
{ 
    return (lhs.StringIndex == rhs.StringIndex) && 
        (lhs.FretNumber == rhs.FretNumber);
}

typedef struct NotePositionEntry_t
{
    uint32_t RepositioningIndex; 
    int32_t PitchMidiValue; 
    
    NotePositionEntry_t() = default;
    
    inline bool operator==(const NotePositionEntry_t& rhs)
    { 
        return (PitchMidiValue == rhs.PitchMidiValue) && 
            (RepositioningIndex == rhs.RepositioningIndex);
    }
    
} NotePositionEntry;

inline bool operator==(const NotePositionEntry_t& lhs, const NotePositionEntry_t& rhs)
{ 
    return (lhs.PitchMidiValue == rhs.PitchMidiValue) && 
        (lhs.RepositioningIndex == rhs.RepositioningIndex);
}


typedef std::map <uint32_t, vector< FretboardPosition >  > PitchMap;

class Note : public Base 
{
    private:
        static const uint32_t MaximumNumberOfOctaves = 30;
        static const uint32_t OctaveValueMidiPitches = 12;
        static vector<std::string> StringIndexedNoteNames;

        uint32_t TrackNumber;
        int32_t PitchMidiValue;
        uint32_t NoteDurationBeats;
        uint32_t CurrentPitchMapRepositionIndex;

        int32_t PitchOffset;
        
        static int32_t TuningMinimum;
        static int32_t TuningMaximum;
        uint32_t Repositions;


        //Setters

        void AddOffsetToPitchMidiValue(int n);

    public:
        static PitchMap PitchToFretMap;
        static int DeletedNotesCount;
        static uint32_t GetNotesLostCounterValue();
        
        static PitchMap GeneratePitchToFretMap(
            vector<std::string> stringNames,
            vector<uint16_t> instrumentCoursePitchValues, 
            const uint32_t numberOfFrets,
            const uint32_t capoFret);
        
        static std::string PrintPitchMap(PitchMap pitchMap);

        //Read pitchmap
        static uint32_t GetFretForNotePositionEntry(NotePositionEntry notePositionEntry);
        static uint32_t GetStringForNotePositionEntry(NotePositionEntry notePositionEntry);
        static std::string PrintNote(NotePositionEntry notePositionEntry);
        static bool GetPlayablePitch(uint32_t& numberOfOctaves, int32_t& pitch);

        Note(int pitch, int duration, int trackNumber); 
        virtual ~Note() {}
        virtual void DispatchVisitor(Visitor* v);

        //Getters
        vector<FretboardPosition> GetNotePositions() const;
        uint32_t GetFretForCurrentNotePosition() const;
        uint32_t GetStringIndexForCurrentNotePosition() const;
        NotePositionEntry GetCurrentNotePosition() const;
        uint32_t GetCurrentPitchmapIndex() const;
        uint32_t GetNoteDurationBeats()  const;
        uint32_t GetTrackNumber() const;
        uint32_t GetPitch() const ;
        uint32_t GetProximityToNearestTuningBoundary() const;
        
        uint32_t GetPitchOffset() const;
        void MakeNoteMorePlayable();
        
        uint32_t GetAttemptedRepositions(void) const;
        virtual uint32_t GetNumberOfElements() const;

        void ReconfigureToNextPitchmapPosition();
        void SetPitchmapPositionIndex(uint32_t mapIndex) ;
        void ResetAttemptedRepositions(void);
};


#endif