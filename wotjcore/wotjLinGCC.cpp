/*
Linux x86-64 GCC platform library

Move ALL OS-dependent and platform-dependent functionality to this library!

Other wotj libraries that are platform-dependent should be linked below the typedefs.

NOTE: Leave in commented-out Windows commands, in case we need to translate them.
*/
#pragma once

#include <unistd.h> //includes some terminal functionality for Linux, including read() used by getkey
#include <termios.h>	//required for terminal commands
#include <cstdio>	//currently used with getkey functions.


//WIP: need to look up the proper platform defs
#define wotj_LINUX

//terminal documentation:
//https://man7.org/linux/man-pages/man3/termios.3.html



typedef signed char sint8;	//8-bit signed
typedef unsigned char uint8;	//8-bit unsigned

typedef short sint16;	//16-bit signed
typedef unsigned short uint16;	//16-bit unsigned

typedef int sint32;	//32-bit signed
typedef unsigned int uint32;	//32-bit unsigned

typedef long long sint64;	//64-bit signed
typedef unsigned long long uint64;	//64-bit unsigned

typedef float sfloat32;	//32-bit floating-point
typedef double sfloat64;	//64-bit floating-point

//typedef unsigned long size_t;	//used in the STL a lot for size-based functions, defined here for compatability in case its not provided.
//typedef unsigned short wchar_t;	//not always provided. used in some situations in place of char.

typedef size_t sizet;	//platform-dependent memory size. no underscores allowed!
typedef wchar_t wchar;	//die _t
typedef key_t keyt;	//platform-dependent key input type. usually an int.


const char* bulletPoint = "•";	//deprecated, will likely remove on next pass
#define filePathSymbol '/'



//other platform-dependent wotj libraries go here
#include "wotjBitwise_x86.cpp"





//NOTE: ENTER key value may vary by siutation. may be either 10 (actual linebreak symbol) or 13 (carriage return symbol).
enum inputKeys : keyt {
	zeroKey = 48, oneKey, twoKey, threeKey, fourKey, fiveKey, sixKey, sevenKey, eightKey, nineKey,
	escKey = 27, ESC = 27, tabKey = 9, enterKey = 10, carRetKey = 13,
	plusKey = 43, minusKey = 45, questionKey = 63, backspaceKey = 127,
	redoKey = 25, undoKey = 26
};
//wip; linux/unicode key inputs are confusing
enum inputKeysWide : keyt {
	upKey = 0x1B5B41, downKey = 0x1B5B42, leftKey = 0x1B5B44, rightKey = 0x1B5B43,
	homeKey = 0x1b5b48, endKey = 0x1b5b46,
	pgupKey = 0x1b5b357e,  pgdnKey = 0x1b5b367e,
	insKey = 0x1b5b327e, delKey = 0x1b5b337e,
	f1Key = 0x1b4f50, f2Key, f3Key, f4Key
};




enum GUIenum : uint8 {guiNeither,guiX11,guiWayland,guiBoth};

//interpreted by GUIenum. guesses current Linux GUI at runtime, in case gui-specific considerations must be provided.
///DO NOT MODIFY unless you have a better detection method to completely replace it with!
const GUIenum currentGUI = (*([](){return (GUIenum(bool(std::system("$WAYLAND_DISPLAY"))*2+bool(std::system("$DISPLAY"))));}))();


#define pause std::cout << "\nPress any key to continue....\n" << std::endl; getKey(true);	//pauses until user input

//refactor: move terminal stuff to separate library.
#define clearScreen std::system("tput reset")	//clears the terminal screen.
#define terminalCommand(inputCommand) std::cout << "\033[" << inputCommand    //raw ANSI command input, must be provided as a string.

#define terminalHideCursor std::cout << "\033[?25l"    //hides the cursor
#define terminalShowCursor std::cout << "\033[?25h"    //shows the cursor
#define terminalResetCursor std::cout << "\033[0;0f"   //sets cursor position to 0,0

///@NOTE: standard types can usually be passed directly. enums and custom types may need to be passed using std::to_string().
#define terminalMoveCursor(cursorX,cursorY) std::cout << "\033["<< (cursorX) <<";"<< (cursorY) <<"f"   //moves cursor position to coordinates. raw value version.

//ANSI color codes. must be converted to a string before passing to the output.
//refactor: wrap in a namespace to make names slightly less bad
enum terminalFormat : char {
	tfDefault = 0,
	tfBold = 1,
	tfUnderline = 4,
	tfNoUnderln = 24,
	tfSwapColor = 7,
	tfUnSwapColor = 27,
	
	tcfgBlack = 30,
	tcfgDrkRed = 31,
	tcfgDrkGreen = 32,
	tcfgDrkYellow = 33,
	tcfgDrkBlue = 34,
	tcfgDrkMagenta = 35,
	tcfgDrkCyan = 36,
	tcfgLightGrey = 37,
	
	tcfgDrkGrey = 90,
	tcfgRed = 91,
	tcfgGreen = 92,
	tcfgYellow = 93,
	tcfgBlue = 94,
	tcfgMagenta = 95,
	tcfgCyan = 96,
	tcfgWhite = 97,
	
	tcbgBlack = 40,
	tcbgDrkRed = 41,
	tcbgDrkGreen = 42,
	tcbgDrkYellow = 43,
	tcbgDrkBlue = 44,
	tcbgDrkMagenta = 45,
	tcbgDrkCyan = 46,
	tcbgLightGrey = 47,
	
	tcbgDrkGrey = 100,
	tcbgRed = 101,
	tcbgGreen = 102,
	tcbgYellow = 103,
	tcbgBlue = 104,
	tcbgMagenta = 105,
	tcbgCyan = 106,
	tcbgWhite = 107
};

//resets color and formatting to default
const std::ostream& terminalClearFormat(std::ostream& stream) {return stream << "\033[0;0m";}
const std::ostream& terminalClearFormat() {return std::cout << "\033[0;0m";}

std::ostream& terminalApplyFormat(std::ostream& stream, terminalFormat format){
	//wip: need performance testing on whether its faster to pass them all directly to stream, or to a string first.
	std::string apply = "\033[";
	apply += std::to_string(format);
	apply += 'm';
	stream << apply;
	//stream << "\033[" << std::to_string(format) << 'm';
	return stream;
}
std::ostream& terminalApplyFormat(std::ostream& stream, terminalFormat format1, terminalFormat format2){
	stream << "\033[" << std::to_string(format1) << ";" << std::to_string(format2) << 'm';
	return stream;
}
std::ostream& terminalApplyFormat(std::ostream& stream, terminalFormat format1, terminalFormat format2, terminalFormat format3){
	stream << "\033[" << std::to_string(format1) << ";" << std::to_string(format2) << ";" << std::to_string(format3) << 'm';
	return stream;
}
template<typename ...recursiveType>
std::ostream& terminalApplyFormat(std::ostream& stream, terminalFormat format, recursiveType ...recursiveInput){
	stream << "\033[" << std::to_string(format) << 'm';
	terminalApplyFormat(stream, recursiveInput...);
	return stream;
}

//WIP: 24-bit colors!
inline std::ostream& tmRGBforeground(std::ostream& stream, uint8 red, uint8 green, uint8 blue){
	return (stream << "\033[38;2;" << red << ";" << green << ";" << blue << 'm');
}
inline std::ostream& tmRGBforeground(uint8 red, uint8 green, uint8 blue){return tmRGBforeground(std::cout, red, green, blue);}

inline std::ostream& tmRGBbackground(std::ostream& stream, uint8 red, uint8 green, uint8 blue){
	return (stream << "\033[38;2;" << red << ";" << green << ";" << blue << 'm');
}
inline std::ostream& tmRGBbackground(uint8 red, uint8 green, uint8 blue){return tmRGBbackground(std::cout, red, green, blue);}




//WARNING: terminal code is still largely experimental!

static struct termios defaultSettings; 	//default terminal flags, NOT set on startup.

//initializes the Linux terminal for raw text I/O
bool initLinuxTerminal(const bool disableNewLineCarRet){
	static struct termios currentSettings;	//current (possibly modified) terminal flags
	
	tcgetattr(STDIN_FILENO, &currentSettings); //extracts current flag state to struct
	defaultSettings = currentSettings;
	
	currentSettings.c_lflag &= ~(ECHO | ICANON);	//turns off echo (printing input characters), Canonical Mode (reading entire lines vs bytes), and some Ctrl commands (ISIG & IEXTEN)
	//currentSettings.c_iflag &= ~(IXON | ICRNL);	//turns off additional Ctrl commands
	if(disableNewLineCarRet)
		currentSettings.c_oflag &= ~(OPOST);
	
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &currentSettings);

	atexit([](){tcsetattr(STDIN_FILENO,TCSAFLUSH,&defaultSettings);});

	// std::cout << "\n\rInitialization finished.\n\r";
	return true;
}
//initializes the Linux terminal.
//no inputs defer to default settings
bool initLinuxTerminal(){
	return initLinuxTerminal(false);
}


//returns key input.
//refactor: remove reference? move ref to overload?
keyt& getKey(std::istream& stream, const bool setTerminal, keyt& k){
	static struct termios oldSettings;
	
	if(setTerminal){
		tcgetattr(0, &oldSettings);	
		
		struct termios newSettings = oldSettings;
		
		newSettings.c_lflag &= ~(ICANON | ECHO | ISIG);
		newSettings.c_iflag &= (IGNBRK);
		//newSettings.c_iflag &= ~(ICRNL);	//carriage return on new line. may change Enter key value between 13 and 10.
		newSettings.c_cc[VTIME] = 0;	//some sort of input timeout. doesnt seem to impact anything in practice, may rely on other factors.
		newSettings.c_cc[VMIN] = 1;	//minimum input bytes to read as input.
		newSettings.c_cc[VEOF] = true;
		tcsetattr(0,TCSAFLUSH, &newSettings);
	}
	
	k = getchar_unlocked();
	
	if (k == escKey){
		if(stream.peek() == 0x5b){	// '['
			getchar_unlocked();	//refactor: use stream-dependent version to avoid potential issues.
			
			switch(stream.peek()){
				case 0x32:{
					k = insKey;
					stream.ignore();
					break;
				}
				case 0x33:{
					k = delKey;
					stream.ignore();
					break;
				}
				case 0x35:{
					k = pgupKey;
					stream.ignore();
					break;
				}
				case 0x36:{
					k = pgdnKey;
					stream.ignore();
					break;
				}
				case 0x41:{	// 'A'
					k = upKey;
					break;
				}
				case 0x42:{	// 'B'
					k = downKey;
					break;
				}
				case 0x43:{	// 'C'
					k = rightKey;
					break;
				}
				case 0x44:{	// 'D'
					k = leftKey;
					break;
				}
				case 0x46:{
					k = endKey;
					break;
				}
				case 0x48:{
					k = homeKey;
					break;
				}
				default:{
					k = 0x1b5b00 + stream.peek();
					break;
				}
			}
			
			stream.ignore();
		}else{
			k=0x1b;
			stream.ignore(0-1, 0);
		}
	}
	if(setTerminal)
		tcsetattr(0, TCSAFLUSH, &oldSettings);
	
	return k;
}
keyt getKey(){
	keyt k = 0;
	struct termios getSettings;
	tcgetattr(0, &getSettings);
	
	getKey(std::cin, (getSettings.c_lflag & !(ICANON | ECHO)), k);
	
	return k;
}
keyt& getKey(keyt& k){
	struct termios getSettings;
	tcgetattr(0, &getSettings);
	
	getKey(std::cin, (getSettings.c_lflag & !(ICANON | ECHO)), k);
	
	return k;
}
keyt getKey(std::istream& stream){
	keyt k = 0;
	struct termios getSettings;
	tcgetattr(0, &getSettings);
	
	getKey(stream, (getSettings.c_lflag & !(ICANON | ECHO)), k);
	
	return k;
}
keyt getKey(const bool setTerminal){
	keyt k = 0;
	
	getKey(std::cin, &setTerminal, k);
	
	return k;
}
keyt& getKey(const bool setTerminal, keyt& output){
	return getKey(std::cin, &setTerminal, output);
}














//remnants from windows, in case i need to make a linux version:

//these ANSI codes should theoretically work on linux as well, but i havent yet found a reason to use them.
//#define terminalBufferAlt std::cout << "\x1b[?1049h" //swaps to alternate buffer. VT mode is required for this to work. effectively just clears the screen.
//#define terminalBufferMain std::cout << "\x1b[?1049i" //swaps to main buffer. VT mode is required for this to work. doesnt seem to do anything.


// //switches to existing buffer. currently doesnt work properly
// void inline changeConsoleBuffer(HANDLE inputBuffer){
//     SetConsoleActiveScreenBuffer(inputBuffer);
//     return;
// }

// //creates new buffer and switches to it. requires VT mode, and will attempt to enable it.
// void inline changeConsoleBuffer(){
//     static HANDLE newBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
//     SetConsoleActiveScreenBuffer(newBuffer);
//     if (!consoleEnableVT()){
//         std::cout << "ERROR: Virtual Terminal mode could not be enabled." << "\nVT is required for this function.";
//         return;
//     }
//     return;
// }


//returns size of the console (in columns/rows) as {X,Y}. defaults to GetStdHandle(STD_OUTPUT_HANDLE).
// COORD getConsoleSize(HANDLE inputHandle){
//     short columns, rows;
//     _CONSOLE_SCREEN_BUFFER_INFO csbi;    //Console Screen Buffer Info local variable
//     GetConsoleScreenBufferInfo(inputHandle, &csbi);
//     columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
//     rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

//     return {columns,rows};
// }
// //returns size of the console (in columns/rows) as {X,Y}. No input defaults to STD_OUTPUT_HANDLE.
// inline COORD getConsoleSize(){
//     return getConsoleSize(GetStdHandle(STD_OUTPUT_HANDLE));    
// }

// //sets window width to 80 characters. it should, anyway, according to microsoft's own documentation. (it doesnt work lol)
// void setConsoleSize(){
//     std::cout << "\x1b[?3I" <<std::endl;
// }


// //sets up console with desired settings. WIP.
// void consoleInitialize(){
//     changeConsoleBuffer();
//     terminalHideCursor;
//     return;
// }