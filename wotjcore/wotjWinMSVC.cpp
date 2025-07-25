/*
Move ALL OS-dependent and platform-dependent functionality to libraries like these.
*/
#pragma once
#include <windows.h>
#include <stdio.h>  //included for printf() used by imported code
#include <wchar.h>
#include <stdlib.h>








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
typedef int keyt;	//platform-dependent key input type. usually an int.







#define pause std::cout << std::endl; system("pause")	//pause syscall
//#define getKey _getwch()	shorthand to capture keyboard input
#define clearScreen system("cls")	//clears the terminal screen using a syscall. slow, do NOT use in loops, will kill framerate.

#define terminalResetCursor std::cout << "\033[0;0f"   //sets cursor position to 0,0
#define terminalMoveCursor(cursorX,cursorY) std::cout << "\033["<<cursorX<<";"<<cursorY<<"f"   //moves cursor position too coordinates. ensure numbers are enclosed in quotes, ex: ("1","2").
#define terminalHideCursor std::cout << "\033[?25l"    //hides the cursor
#define terminalCommand(inputCommand) std::cout << "\033[" << inputCommand    //raw command input, must be in quotes. provides escape character.
#define terminalBufferAlt std::cout << "\x1b[?1049h" //swaps to alternate buffer. VT mode is required for this to work. effectively just clears the screen.
#define terminalBufferMain std::cout << "\x1b[?1049i" //swaps to main buffer. VT mode is required for this to work. doesnt seem to do anything.


inline const char* bulletPoint = "•";
#define filePathSymbol '\\'

//refactor: need to look up the proper platform defs
#define wotj_WINDOWS

bool consoleVTmode = 0; //keeps track of if Virtual Terminal mode is enabled. used by other functions in this library, so dont get rid of it!

//key numeric input values.
//wip: the move to key_t types and wide character support means we probably need to re-test these
enum inputKeys : keyt {
	zeroKey = 48, oneKey, twoKey, threeKey, fourKey, fiveKey, sixKey, sevenKey, eightKey, nineKey,
	escKey = 27, ESC = 27, tabKey = 9, enterKey = 10, carRetKey = 13,
	upKey = 72, downKey = 80, leftKey = 75, rightKey = 77,
	plusKey = 43, minusKey = 45, questionKey = 63,
	pgupKey = 73, pgdnKey = 81
		};

//refactor: move all terminal functionality to separate libraries

//enables Virtual Terminal commands. Only currently works with STD_OUTPUT_HANDLE
bool consoleEnableVT(){
    //Set output mode to handle virtual terminal sequences.
    // MUST be locally defined- cannot be passed into the function. for some reason. thanks microsoft.
    static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE){
        return false;
    }
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)){    //get current console flags
        return false;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;   //enable VT flag using bitwise
    if (!SetConsoleMode(hOut, dwMode)){
        return false;
    }
    return true;
}

//switches to existing buffer. currently doesnt work properly
void inline changeConsoleBuffer(HANDLE inputBuffer){
    SetConsoleActiveScreenBuffer(inputBuffer);
    return;
}

//creates new buffer and switches to it. requires VT mode, and will attempt to enable it.
void inline changeConsoleBuffer(){
    static HANDLE newBuffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(newBuffer);
    if (!consoleEnableVT()){
        std::cout << "ERROR: Virtual Terminal mode could not be enabled." << "\nVT is required for this function.";
        return;
    }
    return;
}


//returns size of the console (in columns/rows) as {X,Y}. defaults to GetStdHandle(STD_OUTPUT_HANDLE).
COORD getConsoleSize(HANDLE inputHandle){
    short columns, rows;
    _CONSOLE_SCREEN_BUFFER_INFO csbi;    //Console Screen Buffer Info local variable
    GetConsoleScreenBufferInfo(inputHandle, &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return {columns,rows};
}
//returns size of the console (in columns/rows) as {X,Y}. No input defaults to STD_OUTPUT_HANDLE.
inline COORD getConsoleSize(){
    return getConsoleSize(GetStdHandle(STD_OUTPUT_HANDLE));    
}

//sets window width to 80 characters. it should, anyway, according to microsoft's own documentation. (it doesnt work lol)
void setConsoleSize(){
    std::cout << "\x1b[?3I" <<std::endl;
}


//sets up console with desired settings. WIP.
void consoleInitialize(){
    changeConsoleBuffer();
    terminalHideCursor;
    return;
}




//wip: currently only looks at standard input stream. needs proper testing.
keyt getKey(std::istream& stream, bool setTerminal, keyt& k){
	if(setTerminal){
		consoleInitialize();
	}
	
	k = _getwch();
	
	return k;
}