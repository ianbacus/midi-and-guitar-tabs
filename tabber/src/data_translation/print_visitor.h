//////////////////////////////////////////////////////
///
/// Convert a score object into a tablature string buffer
//////////////////////////////////////////////////////

#ifndef __PRINTVISITOR__
#define __PRINTVISITOR__


#include "visitor.h"
#include "base.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <math.h>

using namespace std;

class TablatureOutputFormatter : public Visitor
{
    private:
        //1 row for each string/course, and 1 row for duration info
        static const uint32_t NoteTokenWidth = 2;
        static const uint32_t NumberOfPaddingRows = 1;
        static const char TablatureUnfrettedPadding = '-';
        static const char TablatureSustainPadding = '=';
        
        
        static const bool UseHex = true;
        
        const uint32_t MaximumLineWidthCharacters;
        
        uint32_t CurrentLineWidth;      
        vector<string> InstrumentStringNames;
        vector<uint32_t> StringIndexedRemainingDeltaTicks;
        
        vector< vector<string> > TablatureBuffer;
        Chunk* PreviousChunk;
        
        
        uint32_t GetNumberOfTablaturePrintRows(void);
        vector<string> ConcatenateColumnsIntoMeasureStrings(vector<vector<string> >);
        
        string TranslateDeltaAndAppendQuaverCodes(int delta);
        string TempVisitNote(Note *note);
        vector<string> GenerateTablatureColumn(Chunk *chunk);
        vector<string> GenerateTablatureStartColumn();
        vector<string> ConcatenateRowGroups(
                const vector<string> rows0,
                const vector<string> rows1);
        
        
        void UpdateStringIndexedRemainingDeltaTicks(Chunk* candidateChunk);
    
    public:
  	
        TablatureOutputFormatter(uint32_t maximumLineWidth,
                     vector<string> instrumentStringNames);
        
        virtual ~TablatureOutputFormatter(void);

        virtual void VisitNote(Note*) override;
        virtual void VisitBar(Bar*) override;
        virtual void VisitChunk(Chunk*) override;
        void WriteTablatureToOutputFile(string fileName);
};

#endif


