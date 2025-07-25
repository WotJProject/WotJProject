#pragma once
#include "../../wotjcore/wotjLib.cpp"
#include <filesystem>
#include <vector>
#include <cmath>    //currently, only used for sqrtl() in the circle brush functions
#include <vector>	//used to hold dynamic Tile array for text tool
//#include <vector>	//currently only used for activators
#include <array>

//these are currently only used for recovery operations in main()
#include <sys/stat.h>
#include <sys/types.h>

/* master header file

right now, its mainly for storing global types.
*/

//wip: need good naming convention for vectors
//currently only used in stampData, and may deprecate
struct v2s32 {sint32 x; sint32 y;};

//Unicode-compatible tile class. treat like a char*
class Tile {
public:
	struct {
		alignas(8) terminalFormat foreground;
		terminalFormat background;
		terminalFormat style;
		char data[3];	//refactor: make 4 bytes to support full unicode range?
		const char terminator = '\0';
	};
	
private:
	
	//refactor: is an enum overkill for this?
	enum formatCategory : uint8 {
		detectNone = 0,
		detectFG,
		detectBG,
		detectStyle
	};
	
	//detects what kind of formatting is being passed
	//returns 0 if format could not be identified, else returns formatCategory enum
	formatCategory autoDetectFormat(terminalFormat&& format){
		
		if(format == tfDefault)
			return detectNone;
			
		if(format<28){
			style = format;
			return detectStyle;
		}else if(format<38){
			foreground = format;
			return detectFG;
		}else if(format<48){
			background = format;
			return detectBG;
		}else if(format<98){
			foreground = format;
			return detectFG;
		}else if(format<108){
			background = format;
			return detectBG;
		}else
			return detectNone;
	}
	//detects what kind of formatting is being passed
	//returns 0 if format could not be identified, else returns formatCategory enum
	formatCategory autoDetectFormat(const terminalFormat& format){
		
		if(format == tfDefault)
			return detectNone;
			
		if(format<28){
			style = format;
			return detectStyle;
		}else if(format<38){
			foreground = format;
			return detectFG;
		}else if(format<48){
			background = format;
			return detectBG;
		}else if(format<98){
			foreground = format;
			return detectFG;
		}else if(format<108){
			background = format;
			return detectBG;
		}else
			return detectNone;
	}
	
public:
	
	Tile()
		:data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{}
	
	Tile(const char* input)
		:data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{
		for(sizet i = 0; i<3; i++){
			data[i] = input[i];
			if(!input[i])
				break;
		}
	}
	Tile(const char* input, terminalFormat format):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{
		for(sizet i = 0; i<3; i++){
			data[i] = input[i];
			if(!input[i])
				break;
		}
		autoDetectFormat(format);
	}
	Tile(const char* input, terminalFormat format1, terminalFormat format2):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{
		for(sizet i = 0; i<3; i++){
			data[i] = input[i];
			if(!input[i])
				break;
		}
		autoDetectFormat(format1);
		autoDetectFormat(format2);
	}
	Tile(const char* input, terminalFormat formatFG, terminalFormat formatBG, terminalFormat formatStyle):
		data({0}),
		foreground(formatFG),
		background(formatBG),
		style(formatStyle)
	{
		for(sizet i = 0; i<3; i++){
			data[i] = input[i];
			if(!input[i])
				break;
		}
		autoDetectFormat(formatFG);
		autoDetectFormat(formatBG);
		autoDetectFormat(formatStyle);
	}
	Tile(char* &input):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{
		for(sizet i = 0; i<3; i++){
			data[i] = input[i];
			if(!input[i])
				break;
		}
	}
	
	Tile(char& input):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{data[0] = input;}
	
	Tile(char input):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{data[0] = input;}
	
	//may be necessary for some unicode values
	//refactor: use memcpy?
	Tile(int input):
		data({0}),
		foreground(tfDefault),
		background(tfDefault),
		style(tfDefault)
	{
		//this should also handle nullptr inputs
		if(!input){
			return;
		}else if(input < maxU8 && input > maxN8){
			data[0] = (char)(input);
		}else{
			char* const copyInput = (char*)(&input);
			data[0] = copyInput[1];
			data[1] = copyInput[2];
			data[2] = copyInput[3];
		}
	}
	
	//refactor: use memcpy?
	Tile(const Tile& input):
		data({0}),
		foreground(input.foreground),
		background(input.background),
		style(input.style)
	{
		//debug("Const reference constructor called.");
		data[0] = input.data[0];
		data[1] = input.data[1];
		data[2] = input.data[2];
	}
	
	
	//refactor: use memcpy?
	Tile& operator=(Tile& inputTile){
		data[0] = inputTile.data[0];
		data[1] = inputTile.data[1];
		data[2] = inputTile.data[2];
		
		foreground = inputTile.foreground;
		background = inputTile.background;
		style = inputTile.style;
		
		return *this;
		
	}
	//refactor: use memcpy?
	Tile& operator=(const Tile& inputTile){
		data[0] = inputTile.data[0];
		data[1] = inputTile.data[1];
		data[2] = inputTile.data[2];
		
		foreground = inputTile.foreground;
		background = inputTile.background;
		style = inputTile.style;
		
		return *this;
	}
	Tile& operator=(const char* inputTile){
		for(sizet i = 0; i<3; i++){
			data[i] = inputTile[i];
			if(!inputTile[i])
				break;
		}
		return *this;
	}
	Tile& operator=(char input){
		data[0] = input;
		data[1] = '\0';
		return *this;
	}
	
	Tile& operator=(terminalFormat input){
		autoDetectFormat(input);
		return *this;
	}
	
	//square operators are for manually adjusting data[].
	char& operator[](sizet index){
		if(index>2 || index<0)
			return data[index % 3];
		return data[index];
	}
	const char& operator[](const sizet index) const {
		if(index>2 || index<0)
			return data[index % 3];
		return data[index];
	}
	
	const bool operator!(){
		if(data[0] || data[1] || data[2])
			return false;
		else
			return true;
	}
	
	//const bool operator==(const Tile& comparator){
	//	const char* compAlias = (char*)(&comparator);
	//	const char* thisAlias = (char*)(this);
	//	
	//	for(int i=0; i<7; i++)
	//		if(compAlias[i] != thisAlias[i])
	//			return false;
	//	return true;
	//}
	const bool operator==(const Tile& comparator){
		const char* compAlias = (char*)(&comparator);
		const char* thisAlias = (char*)(this);
		
		for(int i=0; i<7; i++)
			if(compAlias[i] != thisAlias[i])
				return false;
		return true;
	}
	bool operator==(const char* comparator){
		for(int i=0; i<3; i++)
			if(data[i] != comparator[i])
				return false;
		
		return true;
	}
	//wip: needs final testing.
	bool operator==(const keyt comparator){
		const char* dummy = (char*)(&comparator);
		
		if(data[0] == dummy[0]){
			if(data[0] == '\0')
				return true;
				
			if(data[1] == dummy[1]){
				if(data[1] == '\0')
					return true;
				
				if(data[2] == dummy[2]){
					return true;
				}
			}
		}
		return false;
	}
	
	
	void clear(){
		data[0] = '\0';
		data[1] = '\0';
		data[2] = '\0';
		
		foreground = tfDefault;
		background = tfDefault;
		style = tfDefault;
		
		return;
	}
	void clearData(){
		data[0] = '\0';
		data[1] = '\0';
		data[2] = '\0';
		
		return;
	}
	void clearFormatting(){
		foreground = tfDefault;
		background = tfDefault;
		style = tfDefault;
		
		return;
	}
	
	const bool isFormatted() const {
		return !(foreground == tfDefault && background == tfDefault && style == tfDefault);
	}
	
	void copyFormatting(Tile& input){
		foreground = input.foreground;
		background = input.background;
		style = input.style;
	}
	
	
	//prints debug info and attempts to resolve any problems
	//refactor: make stream parameter?
	bool debug(bool print){
		bool foundProblem = false;
		
		if(print){
			std::clog << "\nDebug information:\n";
			
			std::clog << "Print:\n";
			std::clog << data << std::endl;
			
			std::clog << "byte by byte:\n";
			std::clog << data[0] << " ";
			std::clog << data[1] << " ";
			std::clog << data[2] << std::endl;
			
			std::clog << "numerical data:\n";
			std::clog << std::hex;
			std::clog << charuint(data[0]) << " ";
			std::clog << charuint(data[1]) << " ";
			std::clog << charuint(data[2]) << std::endl;
			
			std::clog << "formatting data:";
			std::clog << "\n foreground: ";
			std::clog << (+foreground & 255);
			std::clog << "\n background: ";
			std::clog << (+background & 255);
			std::clog << "\n style: ";
			std::clog << (+style & 255);
			
			std::clog << std::dec << std::endl;
		}
		
		const terminalFormat dummyFG = foreground;
		const terminalFormat dummyBG = background;
		const terminalFormat dummyStyle = style;
		
		if(dummyFG && autoDetectFormat(dummyFG) != detectFG){
			foundProblem = true;
			if(print)
				std::clog << "\nWARNING: Foreground value may be incorrect.";
		}
		
		if(dummyBG && autoDetectFormat(dummyBG) != detectBG){
			foundProblem = true;
			if(print)
				std::clog << "\nWARNING: Background value may be incorrect.";
		}
		
		if(dummyStyle && autoDetectFormat(dummyStyle) != detectStyle){
			foundProblem = true;
			if(print)
				std::clog << "\nWARNING: Style value may be incorrect.";
		}
		
		if(print && foundProblem){
			std::clog << "\nCorrected Values:\n" << " Foreground: " << (int)foreground << lbr;
			std::clog << " Background: " << (int)background << lbr;
			std::clog << " Style: " << (int)style << std::endl;
		}

		if(terminator != '\0'){
			foundProblem = true;
			if(print)
				std::clog << "\nFixing terminator...";
			
			data[3] = '\0';
			
			if(print && !terminator)
				std::clog << "\nTerminator fixed: " << (int)terminator << std::endl;
			else
				std::clog << "\nFix failed! Terminator: " << (int)terminator << " at " << &terminator << std::endl;
		}
		
		return foundProblem;
	}
	const bool debug(const char* message){
		std::clog << lbr << message << std::endl;
		return debug(true);
	}
	const bool debug(){return debug(false);}
};

std::ostream& operator<<(std::ostream& stream, Tile output){
	if(output.isFormatted()){
		if(output.foreground != tfDefault)
			stream << "\033[" << std::to_string(output.foreground) << "m";
		if(output.background != tfDefault)
			stream << "\033[" << std::to_string(output.background) << "m";
		if(output.style != tfDefault)
			stream << "\033[" << std::to_string(output.style) << "m";
		stream << output.data;
		stream << "\033[0m";
	}else{
		stream << "\033[0m";
		stream << output.data;
	}
	return stream;
}

std::istream& operator>>(std::istream& stream, Tile& input){
	static char dummy[7];
	stream.read(dummy,7);
	
	input.foreground = (terminalFormat)dummy[0];
	input.background = (terminalFormat)dummy[1];
	input.style = (terminalFormat)dummy[2];
	
	input.data[0] = dummy[3];
	input.data[1] = dummy[4];
	input.data[2] = dummy[5];
	
	return stream;
}

//refactor: use direct writes instead of << operator?
std::ofstream& operator<<(std::ofstream& stream, Tile& output){
	stream << output.foreground;
	stream << output.background;
	stream << output.style;
	
	stream << output.data[0];
	stream << output.data[1];
	stream << output.data[2];
	
	stream << '\0';
	
	return stream;
}

//refactor: memcpy instead?
std::ifstream& operator>>(std::ifstream& stream, Tile& input){
	static char dummy[7];
	stream.read(dummy,7);
	
	input.foreground = (terminalFormat)dummy[0];
	input.background = (terminalFormat)dummy[1];
	input.style = (terminalFormat)dummy[2];
	
	input.data[0] = dummy[3];
	input.data[1] = dummy[4];
	input.data[2] = dummy[5];
	
	return stream;
}


struct FreeLineInfo {
	bool drawLine = false;	//whether or not the first point has been placed.
	sfloat32 rise;
	sfloat32 run;
	sfloat32 slope;
	
	//points
	sint32 x1;
	sint32 y1;
	sint32 x2;
	sint32 y2;
	
	//bounds
	sint32 left;
	sint32 right; 
	sint32 top; 
	sint32 bottom;
} lineData;

//data for copy/paste tool
struct StampInfo {
	//is an enum overkill for this?
	//enum mode : uint8 {
	//	firstCopyPoint = 0,
	//	secondCopyPoint = 1,
	//	pasteSelection = 2
	//};
	
	uint8 stampMode = 0;	//three modes: place copy area points(2), and paste.
	Tile* clipboard = nullptr;	//pointer to copied area. NOTE: actual clipboard file is NOT deleted on program exit. this is intentional.
	
	uint32 clipWidth = 0;
	uint32 clipHeight = 0;
	
	//wip: i dont want to add these because the exact points do not matter and are not used, so will deprecate eventually
	v2s32 a;
	v2s32 b;
	
	//bounds
	sint32 top;	//lowest y
	sint32 bottom;	//highest y
	sint32 left;	//lowest x
	sint32 right;	//highest x
	
	void print(){
		out(pairFormat,"stampMode:",stampMode,
			"clipboard:",*(&clipboard),
			"clipWidth:",clipWidth,
			"clipHeight:",clipHeight,
			"a:","",a.x,a.y,
			"b:","",b.x,b.y,
			"top:",top,
			"bottom:",bottom,
			"left:",left,
			"right:",right);
		return;
	}
} stampData;



//shorthand ANSI colors in addressable array
//note: may deprecate in favor of 24-bit RGB some day...
const std::array<terminalFormat, 16> colorSetFG = {
	tcfgBlack,
	tcfgRed,
	tcfgDrkRed,
	tcfgYellow,
	tcfgDrkYellow,
	tcfgGreen,
	tcfgDrkGreen,
	tcfgCyan,
	tcfgDrkCyan,
	tcfgBlue,
	tcfgDrkBlue,
	tcfgMagenta,
	tcfgDrkMagenta,
	tcfgWhite,
	tcfgLightGrey,
	tcfgDrkGrey
};

//shorthand ANSI colors in addressable array
//note: may deprecate in favor of 24-bit RGB some day...
const std::array<terminalFormat, 16> colorSetBG = {
	tcbgBlack,
	tcbgRed,
	tcbgDrkRed,
	tcbgYellow,
	tcbgDrkYellow,
	tcbgGreen,
	tcbgDrkGreen,
	tcbgCyan,
	tcbgDrkCyan,
	tcbgBlue,
	tcbgDrkBlue,
	tcbgMagenta,
	tcbgDrkMagenta,
	tcbgWhite,
	tcbgLightGrey,
	tcbgDrkGrey
	
};


namespace stdtiles{
	const Tile water = {"≋",tcfgBlue,tcbgDrkBlue};
	const Tile grassSparse = {"░",tcfgGreen,tcbgDrkYellow};
	const Tile grassThick = {"▓",tcfgGreen,tcbgDrkYellow};
	const Tile grassDeep = {"▓",tcfgDrkGreen,tcbgGreen};
	const Tile grassField = {"▓",tcfgDrkYellow,tcbgGreen};	//looks the same as grassSparse
	const Tile sand = {"▓",tcfgYellow,tcbgWhite};	//this is not gonna look the same between platforms
	const Tile rock = {"░",tcfgDrkYellow,tcbgDrkGrey};
	const Tile empty = {"█",tcfgBlack,tcbgBlack,tfDefault};
	const Tile snow = {"█",tcfgWhite,tcbgWhite,tfDefault};
	
	const Tile red = {"█",tcfgDrkRed};
	const Tile blue = {"█",tcfgDrkBlue};
	const Tile green = {"█",tcfgGreen};
	const Tile yellow = {"█",tcfgYellow};
	const Tile brown = {"█",tcfgDrkYellow};
	const Tile pink = {"█",tcfgMagenta};
	const Tile purple = {"█",tcfgDrkMagenta};
	const Tile cyan = {"█",tcfgCyan};
}



//brush/tool to be selected from
enum brushEnum : uint8{
	noBrush = 0,
    squareShape,
    diamondShape,
    circleShape,
    lineVertShape,
    lineHoriShape,
	squareHollowShape,
    circleHollowShape,
    diamondHollowShape,
	bucketFill,
	bucketFillEx,
	tilePicker,
	lineFree,
	textMode,
	stampTool,	//wip
};




extern CycleNumber<uint8> colorsFGindex;
extern CycleNumber<uint8> colorsBGindex;

extern Tile cycleFGcolor[];
extern Tile cycleBGcolor[];

extern uint32 mapSizeX;
extern uint32 mapSizeY;

extern uint32 x;
extern uint32 y;

extern Tile* map;
extern uint32 mapSizeX;
extern uint32 mapSizeY;

extern CycleNumber<uint8> undoSaveTarget;


extern BoundNumber<uint8> availableUndos;
extern BoundNumber<uint8> availableRedos;
bool clipboardLoad();
bool historySave();
bool historyLoad();
extern StampInfo stampData;
extern FreeLineInfo lineData;

extern brushEnum brushType;
extern brushEnum brushPrev;

extern Tile tileSelected;
extern uint16 brushSize;
extern uint8* fillArea;
extern std::vector<Tile> textBuffer;

uint8* fillBucket(Tile* &inputMap, const uint32 fillx, const uint32 filly, const bool invert);
void brushResizeBounds(brushEnum brushInput);
void selectBrushType(const brushEnum selected);
void applyBrush(const Tile paint, const uint16 size, const brushEnum shape, const uint32 posx, const uint32 posy);

extern uint8 cycleBrushIndex;
extern const char* cycleBrush[3];
extern CycleNumber<uint16> cycleIncrement;
extern Tile cycleTile[16];
