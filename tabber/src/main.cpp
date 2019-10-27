#include "midi2melody.h"
#include "visitor.h"
#include "print_visitor.h"
#include "GUIprintvisitor.h"
#include "rotate_visitor.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <iostream>
#include <string.h>


void SortScoreByChunkFlexibility(vector<Chunk*>& sortedScore)
{
    sort(begin(sortedScore),end(sortedScore), [](const Chunk* left, const Chunk* right)
    {
        vector<Note*> leftNotes = left->GetElements();
        vector<Note*> rightNotes = right->GetElements();

        Note* leastMobileNoteLeft = *min_element(begin(leftNotes),end(leftNotes),
            [](const Note* lhs, const Note* rhs)
            {
                return lhs->GetProximityToNearestTuningBoundary() <
                       rhs->GetProximityToNearestTuningBoundary();

            });

        Note* leastMobileNoteRight = *min_element(begin(rightNotes),end(rightNotes),
            [](const Note* lhs, const Note* rhs)
            {
                return lhs->GetProximityToNearestTuningBoundary() <
                       rhs->GetProximityToNearestTuningBoundary();
            });

        uint32_t leastMobileNotePlacementsLeft = leastMobileNoteLeft->GetProximityToNearestTuningBoundary();
        uint32_t leastMobileNotePlacementsRight = leastMobileNoteRight->GetProximityToNearestTuningBoundary();

        if(leastMobileNotePlacementsLeft == leastMobileNotePlacementsRight)
        {
            return left->GetNumberOfPositionPermutations() < right->GetNumberOfPositionPermutations();
        }

        else
        {
            return leastMobileNotePlacementsLeft < leastMobileNotePlacementsRight;
        }
    });
}

void FixInputs(
        const uint32_t maximumLength,
        int32_t& noteOffset,
        uint32_t& lowerBound,
        uint32_t& upperBound)
{
    const int16_t transpositionLimit = 127;

    //Limit transpositions to +/- 127 pitches
	if (noteOffset < -1*transpositionLimit)
	{
		noteOffset = -1*transpositionLimit;
	}

	if (noteOffset > transpositionLimit)
	{
		noteOffset = transpositionLimit;
	}

	//Swap bounds if they inverted
	if (upperBound < lowerBound)
	{
		swap(lowerBound,upperBound);
	}

    //Clamp the bounds
    upperBound = min(upperBound, maximumLength);
    lowerBound = min(lowerBound, maximumLength);
}

void GenerateXML(vector<Chunk*> score)
{
	stringstream xmlString;

	int32_t currentMeasureIndex = -1;
	xmlString << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><!DOCTYPE score-partwise PUBLIC '-//Recordare//DTD MusicXML 2.0 Partwise//EN' 'http://www.musicxml.org/dtds/2.0/partwise.dtd'><score-partwise version=\"2.0\"> <identification>  <encoding>   <encoding-date>2018-10-07</encoding-date>   <software>Guitar Pro 7.5.1</software>  </encoding> </identification> <defaults>  <scaling>   <millimeters>6.4</millimeters>   <tenths>40</tenths>  </scaling>  <page-layout>   <page-height>1850</page-height>   <page-width>1310</page-width>  </page-layout> </defaults> <part-list>  <score-part id=\"P1\">   <part-name>Track 1</part-name>   <part-abbreviation></part-abbreviation>   <midi-instrument id=\"P1\">    <midi-channel>1</midi-channel>    <midi-bank>1</midi-bank>    <midi-program>26</midi-program>    <volume>80</volume>    <pan>0</pan>   </midi-instrument>  </score-part> </part-list> <part id=\"P1\">  <measure number=\"1\">   <attributes>    <divisions>4</divisions>    <key>     <fifths>0</fifths>     <mode>major</mode>    </key>    <time>     <beats>3</beats>     <beat-type>4</beat-type>    </time>    <staves>2</staves>    <clef number=\"1\">     <sign>G</sign>     <line>2</line>    </clef>    <clef number=\"2\">     <sign>TAB</sign>     <line>5</line>    </clef>    <staff-details number=\"1\">     <staff-tuning line=\"1\">      <tuning-step>B</tuning-step>      <tuning-octave>1</tuning-octave>     </staff-tuning>     <staff-tuning line=\"2\">      <tuning-step>E</tuning-step>      <tuning-octave>2</tuning-octave>     </staff-tuning>     <staff-tuning line=\"3\">      <tuning-step>A</tuning-step>      <tuning-octave>2</tuning-octave>     </staff-tuning>     <staff-tuning line=\"4\">      <tuning-step>D</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"5\">      <tuning-step>G</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"6\">      <tuning-step>B</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"7\">      <tuning-step>E</tuning-step>      <tuning-octave>4</tuning-octave>     </staff-tuning>    </staff-details>    <staff-details number=\"2\">     <staff-lines>7</staff-lines>     <staff-tuning line=\"1\">      <tuning-step>B</tuning-step>      <tuning-octave>1</tuning-octave>     </staff-tuning>     <staff-tuning line=\"2\">      <tuning-step>E</tuning-step>      <tuning-octave>2</tuning-octave>     </staff-tuning>     <staff-tuning line=\"3\">      <tuning-step>A</tuning-step>      <tuning-octave>2</tuning-octave>     </staff-tuning>     <staff-tuning line=\"4\">      <tuning-step>D</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"5\">      <tuning-step>G</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"6\">      <tuning-step>B</tuning-step>      <tuning-octave>3</tuning-octave>     </staff-tuning>     <staff-tuning line=\"7\">      <tuning-step>E</tuning-step>      <tuning-octave>4</tuning-octave>     </staff-tuning>    </staff-details>    <transpose number=\"1\">     <diatonic>0</diatonic>     <chromatic>0</chromatic>     <octave-change>-1</octave-change>    </transpose>   </attributes>";

	for(Chunk* chunk : score)
	{
		uint32_t noteCount = 0;
		uint32_t measureIndex = chunk->GetMeasureIndex();
		bool newMeasure = (measureIndex != currentMeasureIndex);

		if(newMeasure)
		{
			if(measureIndex != 0)
			{
				xmlString << "</measure>\r\n";
			}

			xmlString << "<measure number=\"" << measureIndex << "\">\r\n";
			currentMeasureIndex = measureIndex;

		}

		uint32_t noteDuration;

		Chunk* nextChunk = chunk->GetNextChunk();
		if(nextChunk != nullptr)
		{
			noteDuration = chunk->GetDelta();
		}

		for (Note* note : chunk->GetElements())
		{
			uint32_t stringIndex = note->GetStringIndexForCurrentNotePosition();
			uint32_t fretNumber = note->GetFretForCurrentNotePosition();
			uint32_t pitch = note->GetPitch();

			noteCount++;
			xmlString << "  <note>";
			if(noteCount > 1)
			{
				xmlString << "<chord/>";
			}

			xmlString << "<duration>" << noteDuration << "</duration>" <<
				"<pitch> <step>B</step> <octave>9</octave></pitch>"
				"<voice>1</voice><type>sixteenth</type><stem>up</stem><notehead>normal</notehead><staff>1</staff>" <<
				"<notations><technical><string>" << 7-stringIndex <<
				"</string><fret>" << fretNumber << "</fret></technical></notations></note>\r\n";

		}
	}

	xmlString << "</part></score-partwise>" << endl;

    std::ofstream musxml("musxml.xml");
	musxml << xmlString.str();
	musxml.close();
}
uint32_t ProcessScore(
        string outputFile,
        const uint32_t lowerBound,
        const uint32_t upperBound,
        vector<Chunk*>& score,
        TablatureOptimizer& tablatureRearranger,
        TablatureOutputFormatter& tablaturePrinter)

{
    uint32_t totalCost = 0;

    vector<Chunk*> sortedScore  = score;

    SortScoreByChunkFlexibility(sortedScore);
    //First pass: process and place chunks with fewer permutations before
    //those with more permutations
    for (Chunk* currentChunk : sortedScore)
    {
        uint32_t measureIndex = currentChunk->GetMeasureIndex();

        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
            tablatureRearranger.OptimizeChunk(currentChunk);
        }
    }

    tablatureRearranger.EmitDebugString("First pass complete");

    //Second pass: reposition chunks in chronological order so that user's
    //inter-chunk constraints can be satisfied
    for (Chunk* currentChunk : score)
    {
        uint32_t measureIndex = currentChunk->GetMeasureIndex();

        if((lowerBound <= measureIndex) && (measureIndex <= upperBound))
        {
            if(measureIndex == upperBound)
            {
                currentChunk->SetIsMeasureEnd(true);
            }

            currentChunk->ResetAllNotesRepositions();
            currentChunk->SetIsOptimized(false);

            totalCost += tablatureRearranger.OptimizeChunk(currentChunk);
            tablaturePrinter.VisitChunk(currentChunk);
        }
    }

    tablaturePrinter.WriteTablatureToOutputFile(outputFile);
    tablaturePrinter.WriteTablatureToOutputFile("data/outTab.txt");
}


uint32_t ProcessScoreToString(
		string& outputString,
        const uint32_t lowerBound,
        const uint32_t upperBound,
        vector<Chunk*>& score,
        TablatureOptimizer& tablatureRearranger,
        TablatureOutputFormatter& tablaturePrinter)

{
    uint32_t totalCost = 0;

    vector<Chunk*> sortedScore  = score;

    SortScoreByChunkFlexibility(sortedScore);
    //First pass: process and place chunks with fewer permutations before
    //those with more permutations
    for (Chunk* currentChunk : sortedScore)
    {
        tablatureRearranger.OptimizeChunk(currentChunk);

    }

    tablatureRearranger.EmitDebugString("First pass complete");

    //Second pass: reposition chunks in chronological order so that user's
    //inter-chunk constraints can be satisfied
    for (Chunk* currentChunk : score)
    {
		currentChunk->ResetAllNotesRepositions();
		currentChunk->SetIsOptimized(false);

		totalCost += tablatureRearranger.OptimizeChunk(currentChunk);
		tablaturePrinter.VisitChunk(currentChunk);
    }

    tablaturePrinter.WriteTablatureToOutputString(outputString);

	return totalCost;
}



int ParseFileIntoTab(
        const string inputFile,
        const string outputFile,
        int32_t noteOffset,
        uint32_t lowerBound,
        uint32_t upperBound,
        const uint32_t deltaExpansion)
{

    uint32_t totalCost = 0;
    map<string,uint32_t> parsedConstants;

	parsedConstants["Frets"] = 10;
	parsedConstants["CapoFret"] = 0;
	parsedConstants["NeckPositionCost"] = 1500;
	parsedConstants["SpanCost"] = 3000;
	parsedConstants["NeckDiffCost"] = 7500;
	parsedConstants["SuppressedSustainCost"] = 1000;
	parsedConstants["ArpeggiationDeduction"] = 1000;
	parsedConstants["NumberOfLinesPerTabRow"] = 300;
	parsedConstants["ChunkComparisonSearchLength"] = 4;

    vector<Chunk*> score;

    TabberSettings tabSettings = {0};

    //Parse user settings file
    ParseTabberSettingsFile("src/tabberSettings.txt", parsedConstants, tabSettings);


    Note::InitializePitchToFretMap(
        tabSettings.InstrumentInfo.StringIndexedNoteNames,
        tabSettings.InstrumentInfo.StringIndexedMidiPitches,
        parsedConstants["Frets"],
        parsedConstants["CapoFret"]);

    TablatureOptimizer tablatureRearranger(
        tabSettings.InstrumentInfo.StringIndexedNoteNames.size(),
        parsedConstants["Frets"],
        parsedConstants["NeckPositionCost"],
        parsedConstants["SpanCost"],
        parsedConstants["NeckDiffCost"],
        parsedConstants["SuppressedSustainCost"],
        parsedConstants["ArpeggiationDeduction"],
		parsedConstants["ChunkComparisonSearchLength"]);

    TablatureOutputFormatter tablaturePrinter(
        parsedConstants["NumberOfLinesPerTabRow"],
        tabSettings.InstrumentInfo.StringIndexedNoteNames);
	
	//std::cout << ""
	tablatureRearranger.SetPrinter(tablaturePrinter);
	
    //Sort score by number of chunk permutations
	score = ParseIntermediateFile(
        inputFile,
        noteOffset,
        deltaExpansion,
        deltaExpansion);

    FixInputs(score.size(), noteOffset, lowerBound, upperBound);

    std::cout << outputFile << ": Optimizing and printing "
            << (upperBound - lowerBound)+1
            << "measures, transposed " << noteOffset << " semi-tones: m"
            << lowerBound << " to m" << upperBound
			<< ", DE = " << deltaExpansion << endl;


    totalCost = ProcessScore(
        outputFile,
        lowerBound, upperBound, score,
        tablatureRearranger, tablaturePrinter);

    cout << "Done.";
    cout << Note::GetNotesLostCounterValue() << " notes were out of range";
	cout << "Total cost: $" << totalCost << endl;

    score.clear();

    return 0;
}
extern "C" {
 const char* javascriptWrapperFunction(
	char* fileData,
	char* tuningString,
	uint8_t* tuningData,
	int32_t noteOffset,
	uint32_t frets,
	uint32_t neckCost,
	uint32_t spanCost,
	uint32_t diffCost,
	uint32_t sustainCost,
	uint32_t arpeggioCost,
	uint32_t columnFormat)
 {
     uint32_t totalCost = 0;
     map<string,uint32_t> parsedConstants;

 	parsedConstants["Frets"] = frets;
 	parsedConstants["CapoFret"] = 0;
 	parsedConstants["NeckPositionCost"] = neckCost;
 	parsedConstants["SpanCost"] = spanCost;
 	parsedConstants["NeckDiffCost"] = diffCost;
 	parsedConstants["SuppressedSustainCost"] = sustainCost;
 	parsedConstants["ArpeggiationDeduction"] = arpeggioCost;
 	parsedConstants["NumberOfLinesPerTabRow"] = columnFormat;
	parsedConstants["ChunkComparisonSearchLength"] = 4;

     vector<Chunk*> score;

     TabberSettings tabSettings = {0};

     //Parse user settings file
	 uint32_t instrumentLength = strlen(tuningString);
	 for(int index = 0; index<instrumentLength;index++)
	 {
		 tabSettings.InstrumentInfo.StringIndexedMidiPitches.push_back(tuningData[index]);
		 tabSettings.InstrumentInfo.StringIndexedNoteNames.push_back(string(1, tuningString[index]));
	 }
	 cout << "constructing objects. " <<  endl;
     Note::InitializePitchToFretMap(
         tabSettings.InstrumentInfo.StringIndexedNoteNames,
         tabSettings.InstrumentInfo.StringIndexedMidiPitches,
         parsedConstants["Frets"],
         parsedConstants["CapoFret"]);

     TablatureOptimizer tablatureRearranger(
         tabSettings.InstrumentInfo.StringIndexedNoteNames.size(),
         parsedConstants["Frets"],
         parsedConstants["NeckPositionCost"],
         parsedConstants["SpanCost"],
         parsedConstants["NeckDiffCost"],
         parsedConstants["SuppressedSustainCost"],
         parsedConstants["ArpeggiationDeduction"],
		parsedConstants["ChunkComparisonSearchLength"]);

      TablatureOutputFormatter tablaturePrinter(
          parsedConstants["NumberOfLinesPerTabRow"],
          tabSettings.InstrumentInfo.StringIndexedNoteNames);

	 uint32_t lowerBound = 0;
	 uint32_t upperBound = score.size();
	 uint32_t deltaExpansion = 0;

	 string fileDataString = fileData;

	 cout << "parsing intermediate data" << endl;
 	score = ParseIntermediateDataString(
         fileDataString,
         noteOffset,
         deltaExpansion,
         deltaExpansion);

    std::cout << "Optimizing and printing measures, transposed "
			<< noteOffset << " semi-tones: m"
			<< ", DE = " << deltaExpansion << endl;

	 string scoreString;
     totalCost = ProcessScoreToString(
		 scoreString,
         lowerBound, upperBound, score,
         tablatureRearranger, tablaturePrinter);

     score.clear();

	 //Stack string
	 string* outputString = new string(scoreString);
	 const char* resultString = nullptr;
	 
	 if(outputString != nullptr)
	 {
		resultString = outputString->c_str();
		cout << "done" << endl;
	 }
	 
	 //Let javascript application delete memory
	 return resultString;
 }
}

int main(int argc, char* argv[])
{
    const bool Debug = false;
    const uint8_t correctNumberOfArguments = 7;

    int returnValue = 0;

    if(Debug)
    {
        returnValue = ParseFileIntoTab(
            "data/parsed_midi_data.txt",
            "data/tabs/outTab.txt", -12, 0, -1, -1);
    }

    else if(argc != correctNumberOfArguments)
    {
        cout << "Invalid entry. use the following format:\n";
        cout << ">> ./gen <inputFile> <outputFile> <pitchShift#>,";
        cout << " <measuresPerRow#> <startMeasure#> <endMeasure#>" << endl;
    }

    else
    {
        const string inputFile = argv[1]; //name of input file
        const string outputFile = argv[2];

        int noteOffset=atoi(argv[3]);

        uint32_t lowerBound=atoi(argv[4]);
        uint32_t upperBound=atoi(argv[5]);

        unsigned int align = atoi(argv[6]);

        returnValue = ParseFileIntoTab(
            inputFile,
            outputFile,
            noteOffset,
            lowerBound,
            upperBound,
            align);
    }

    return returnValue;
}
