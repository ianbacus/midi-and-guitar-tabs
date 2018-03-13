#include "print_visitor.h"
#include <sstream>
#include <assert.h>

using namespace std;
map<int,int> beat_value = {{1,32},{2,16},{4,8},{8,4},{16,2},{32,1}};

//Note length indications to print above the notes
std::map<int,string> quaver_map = 
{
	{1, " S"}, {2, " T"},  
    {4, " s"}, {6, " s"},
    {8, " e"}, {14," e"}, 
    {16," q"}, {24," q"},
    {32," h"}, {56," h"}, 
    {64," w"},
};


/*
 *	Initialize print Visitor with output file and columns per row
 */	
PrintVisitor::PrintVisitor(uint32_t maximumLineWidth, 
                           vector<string> instrumentStringNames) 
    : 
        CurrentLineWidth(0),
        InstrumentStringNames(instrumentStringNames),
        MaximumLineWidthCharacters(maximumLineWidth)
{
}


PrintVisitor::~PrintVisitor(void) 
{
    TablatureBuffer.erase(std::begin(TablatureBuffer), 
                          std::end(TablatureBuffer));
}


string PrintVisitor::TempVisitNote(Note *note)
{
    string result;
    std::stringstream sstream;
    int fret = note->GetFretForCurrentNotePosition();

    if (fret < 0) //rest note
    {
        sstream << std::string(NoteTokenWidth,TablaturePadding);;
    }

    else if(fret < 16)
    {
        sstream << TablaturePadding <<  std::hex << std::uppercase << fret;
    }
    
    else
    {
        sstream <<  std::hex << std::uppercase << fret;
    }   
    
    result = sstream.str();
    return result;
}

uint32_t PrintVisitor::GetNumberOfTablaturePrintRows(void)
{
    
    const uint32_t numberOfTablatureRows = InstrumentStringNames.size();
    const uint32_t numberOfTablaturePrintRows = 
        NumberOfPaddingRows + numberOfTablatureRows;
    
    return numberOfTablaturePrintRows;
}

vector<string> PrintVisitor::GenerateTablatureStartColumn(void)
{
    const uint32_t numberOfTablaturePrintRows = GetNumberOfTablaturePrintRows();
    const uint32_t numberOfTablatureRows = InstrumentStringNames.size();
    const uint32_t widthOfStartColumn = 3;
    
	vector<string> columnOfStringData(numberOfTablaturePrintRows);
    
    uint32_t maxIndex = numberOfTablatureRows;
    columnOfStringData[0] = "|  ";
    
    for(int32_t instrumentCourseIndex = numberOfTablatureRows-1; 
            instrumentCourseIndex >= 0;
            instrumentCourseIndex--)
    {
        
        const uint32_t reverseAdjustedOffset = maxIndex-instrumentCourseIndex;
          
        string rowData = "|" + InstrumentStringNames[instrumentCourseIndex] + "|";
        
        columnOfStringData[reverseAdjustedOffset] = rowData;
    }
    
    return columnOfStringData;
    
} //end GenerateTablatureStartColumn

vector<string> PrintVisitor::GenerateTablatureColumn(Chunk *chunk)
{
    const uint32_t numberOfTablaturePrintRows = GetNumberOfTablaturePrintRows();
    const uint32_t numberOfTablatureRows = InstrumentStringNames.size();

    const uint32_t offset = numberOfTablatureRows;
    
	vector<string> columnOfStringData(numberOfTablaturePrintRows);
        
    vector<uint32_t> unmodifiedStringIndices;
    const int32_t chunkDelta = chunk->GetDelta();
    const string quaverString = TranslateDeltaAndAppendQuaverCodes(chunkDelta);
    
    //Insert padding rows
    columnOfStringData[0] = quaverString;
    
    string chunkDeltaScaledPadding(chunkDelta,TablaturePadding);
    
    for(uint32_t instrumentCourseIndex = 0;
            instrumentCourseIndex<numberOfTablatureRows;
            instrumentCourseIndex++)
    {
        unmodifiedStringIndices.push_back(instrumentCourseIndex);
    }
    
    for (Note *note : chunk->GetElements())
    {
        const uint32_t courseIndex = note->GetStringIndexForCurrentNotePosition();
        const string rowData = TempVisitNote(note);
        
        //The strings are stored in reverse order, and offset by the padding row(s)
        const uint32_t columnAdjustedOffset = offset-courseIndex;
        
        unmodifiedStringIndices.erase(std::remove(unmodifiedStringIndices.begin(), 
            unmodifiedStringIndices.end(), courseIndex), unmodifiedStringIndices.end()); 
        
        columnOfStringData[columnAdjustedOffset] = rowData + chunkDeltaScaledPadding;
    }
    
    for (uint32_t courseIndex : unmodifiedStringIndices)
    {
        const uint32_t columnAdjustedOffset = offset-courseIndex;
        const string paddingPlaceholder(NoteTokenWidth,TablaturePadding);
        
        columnOfStringData[columnAdjustedOffset] = paddingPlaceholder + chunkDeltaScaledPadding;
    }
    
    
    return columnOfStringData;
    
} //end GenerateTablatureColumn

//Concatenate a range of row groupings
vector<string> PrintVisitor::ConcatenateColumnsIntoMeasureStrings(vector<vector<string> > columns)
{
    const uint32_t rowGroupNumberOfRows = columns.back().size();
    
    
    vector<string> rowGroup(rowGroupNumberOfRows);
    
    for(vector<string> column : columns)
    {        
        for(uint32_t rowIndex = 0; rowIndex<rowGroupNumberOfRows;rowIndex++)
        {
            string rowData = column[rowIndex];
            rowGroup[rowIndex] += rowData;
        }
    }
    return rowGroup;
    
} //end ConcatenateColumnsIntoMeasureStrings

//Concatenate two row groupings
vector<string> PrintVisitor::ConcatenateRowGroups(
        const vector<string> rows0,
        const vector<string> rows1)
{
    vector<string> rowGroup;
    
    uint32_t rows0Size = rows0.size();
    uint32_t rows1Size = rows1.size();
    
    if(rows0Size == rows1Size)
    {
        for(uint32_t rowIndex=0; rowIndex < rows1Size; rowIndex++)
        {
            string rowData0 = rows0[rowIndex];
            string rowData1 = rows1[rowIndex];

            rowGroup.push_back(rowData0+rowData1);
        }
    }
    
    return rowGroup;
    
} //end ConcatenateRowGroups

void PrintVisitor::VisitBar(Bar* currentBar)
{
    const uint32_t numberOfTablatureRows = InstrumentStringNames.size();
    
    vector<Chunk*> chunks = currentBar->GetElements();
    
    vector<string> breakColumn(numberOfTablatureRows+1, "|");
    vector<vector<string> > tablatureColumns;
    
    for(Chunk *chunk : chunks)
    {
        vector<string> chunkColumn = GenerateTablatureColumn(chunk);
        
        tablatureColumns.push_back(chunkColumn);
    }
    
    tablatureColumns.push_back(breakColumn);
    
    vector <string> tablatureRows = ConcatenateColumnsIntoMeasureStrings(tablatureColumns);
        
    const uint32_t measureLength = tablatureRows[0].size();
    const uint32_t newLineWidth = (measureLength + CurrentLineWidth);
    const bool createNewRow = newLineWidth > MaximumLineWidthCharacters;
    
    if(createNewRow || TablatureBuffer.size() == 0)
    {
        vector<string> startColumn = GenerateTablatureStartColumn();
        tablatureColumns.insert(std::begin(tablatureColumns), startColumn);
        
        
        vector <string> newTablatureRows =  
            ConcatenateColumnsIntoMeasureStrings(tablatureColumns);
        
        
        TablatureBuffer.push_back(newTablatureRows);
        CurrentLineWidth = measureLength;
    }
    
    else
    {
        vector<string> currentTablatureRows = TablatureBuffer.back();
        
        currentTablatureRows = ConcatenateRowGroups(currentTablatureRows, 
                                                    tablatureRows);
        
        TablatureBuffer.back() = currentTablatureRows;
        CurrentLineWidth = newLineWidth;
    }
    
} //end VisitBar


void PrintVisitor::VisitChunk(Chunk* currentBar)
{
    
}

void PrintVisitor::VisitNote(Note* currentBar)
{
    
}

/*
 *	Add padding to the string buffers based on the given delta
 */
string PrintVisitor::TranslateDeltaAndAppendQuaverCodes(int delta)
{
    string quaverCodeString;
    
	//Case 1: the delay between the chunks can be represented with one of the defined note duration codes
	if(quaver_map.find(delta) != quaver_map.end()) 
	{
		quaverCodeString = quaver_map[delta] + std::string(delta,' ');
	}
	
	//Case 2: a triplet (todo: handle this better)
	else if(delta<0)
	{
		quaverCodeString = "^ ";
	}
	
	
	//Case 3: the delay between chunks is the result of one or more rests, and a normal note duration
	else
	{
		int extra_delta=0;
        int resultingDelta = 0;

		while(quaver_map.find(delta) == quaver_map.end())
		{
			delta--;
			extra_delta++;
		}
        
        resultingDelta = delta+extra_delta;
		
		quaverCodeString = quaver_map[delta] + std::string(resultingDelta,' ');
	}    
    
    return quaverCodeString;
}

/*
 *	Print the contents of the string buffer to the chosen file
 */	
void PrintVisitor::WriteTablatureToOutputFile(string fileName)
{
	ofstream outputFileStream;
	stringstream tablatureStringStream;
    
    for(vector<string> tablatureRow: TablatureBuffer)
    {
        
        for(string tablatureRowData : tablatureRow)
        {
            tablatureStringStream << tablatureRowData << "\r\n";
        }
        
        tablatureStringStream << "\r\n";
    }
    
	outputFileStream.open(fileName);
	outputFileStream << tablatureStringStream.rdbuf();
	outputFileStream.close();
    
} //end WriteTablatureToOutputFile
