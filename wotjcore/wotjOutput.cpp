#pragma once
#include "..wotjLib.cpp"
#include <cstdlib>
#include <iostream>
// #include <fstream>
#include <streambuf>
// #include <sys/types.h>
#include <string>
#include <string_view>	//currently only used in wordWrap()
#include <fstream>	//currently only used by out() fstream overloads. may refactor into the file library.

///DOD refactor: clean up this mess
//	-goal here is to reduce reliance on switch statements
//	-separate functions for the formats? ex. outlbr() or outpair()
//	-can they coexist with the overloaded out() versions?
//	-would a singleton or namespace be better than standalone functions?


const std::streambuf* gDefaultStream_cin = std::cin.rdbuf();	//default stream target for cin
const std::streambuf* gDefaultStream_cout = std::cout.rdbuf();	//default stream target for cout
const std::streambuf* gDefaultStream_cerr = std::cerr.rdbuf();	//default stream target for cerr (unbuffered error output)
const std::streambuf* gDefaultStream_clog = std::clog.rdbuf();	//default stream target for clog (buffered error output)

//refactor: cannot move within classes due to overloading. must remain global?
// perhaps encapsulate out() in a class? will need to do testing to see if its worth it.
static uint32 sListNumerator = 0;

//format type used for out functions
//refactor: comments are there to demonstrate format, but different IDEs interpret comments differently...
enum FormatEnum : char {
	//prints inputs directly without any formatting
	///ex: Helloinput1input2input3World
	noFormat = 0,
	//places a space between each input and at the end.
	///ex: Helloinput1 input2 input3 World
	spaceFormat,	
	//places a linebreak before each input and after all inputs are printed.
	/*
	ex; Hello
	
	input1
	
	input2
	
	input3
	
	World
	*/
	lbrFormat,
	lineBreakFormat,
	//places a comma and a space between each input.
	///ex: Helloinput1, input2, input3World
	commaFormat,
	// places a comma and linebreak after each input and linebreaks after all inputs are printed
	/* 
	ex; Hello
	
	input1,
	input2,
	input3
	
	World	
	*/
	commaBreakFormat,
	/*number and linebreak with each element, and linebreak after all inputs are printed.
	ex; Hello
	
	1. input1
	2. input2
	3. input3
	
	World
	*/
	numberedFormat,
	//linebreak before every other element and after all inputs are printed, and place a space between pairs.
	/// ex:Hello
	//
	/// input1 input2
	//
	/// input3
	//
	/// World
	pairFormat,
	/*linebreak before and after, and precede each input with "ERROR #: ". WIP.
	ex:Hello.
	 ERROR: input1.

	 ERROR: input2.
	 
	 ERROR: input3.
	World*/
	errorFormat,
	//precede each input with a dot (as defined in the platform library).
	//linebreak before each line, and after all are finished.
	///ex: Hello
	//
	///-input1
	//
	///-input2
	//
	///-input3
	//
	///World
	listFormat
};


///@REMINDERS: testing has revealed std::cout has slight performance hurdles, anywhere from 10~100% slower (with default optimization flags),
// when stringing multiple inputs in the same line, ex: std::cout << input1 << input2 << std::endl;
// making it even slower than a recursive function wrapper for std::cout that outputs the same exact data.
//
//Also, it looks like the operator overloading mechanics for >>/<< may cause inline functions or operations to be processed inside-out,
// so pass them in as separate lvalues instead if this issue presents itself.


//Combined line output function. use format enums to dictate output format.
//*accepts Format enums and recursive inputs. 
//*inputs will ALWAYS be the last (for variadic recursion), and Format enum will always be first (after stream object).
template<typename inputType>
std::ostream& out(std::ostream& stream, FormatEnum format, inputType input){

	switch(format){
		case numberedFormat:{
			sListNumerator++;	//must be done separately, cannot be inlined with <<
			stream << lbr;
			stream << sListNumerator;
			stream << ". ";
			stream << input;
			stream << lbr;
			sListNumerator = 0;
			break;
		}
		case spaceFormat:{
			stream << input;
			stream << ' ';
			break;
		}
		case commaBreakFormat:
		case lbrFormat:
		case lineBreakFormat:{
			stream << lbr;
			stream << input;
			stream << lbr;
			break;
		}
		case pairFormat:{
			if(!sListNumerator){
				stream << lbr; 
				stream << input;
			}
			else{
				stream << ' ';
				stream << input;
				sListNumerator = 0;
			}
			stream << lbr; 
			break;
		}
		//will deprecate as soon as proper error functions are written.
		case errorFormat:{
			if(!sListNumerator){
				stream << "\nERROR ";
				//stream << g_ERROR;	//global errors are still WIP
				stream << ": ";
			}
			stream << input;
			stream << std::endl;
			sListNumerator = 0;
			break;
		}
		case listFormat:{
			stream << lbr;
			stream << "•";
			stream << input;
			stream << lbr;
			break;
		}
		case commaFormat:
		case noFormat:
		default:{
			stream << input;
			break;
		}
	}
	return stream;
}

//Combined line output function. use format enums to dictate output format.
///*accepts Format enums and recursive inputs. 
///*inputs will ALWAYS be the last (for recursion), and Format enum will always be first (after stream object).
template<typename inputType, typename... recursiveType>
std::ostream& out(std::ostream& stream, FormatEnum format, inputType input, recursiveType... inputRecursive){

	switch(format){	
		case numberedFormat:{
			sListNumerator++;	//must be done separately, cannot be inlined with <<
			stream << lbr;
			stream << sListNumerator;
			stream << ". ";
			stream << input;
			break;
		}
		case spaceFormat:{
			stream << input;
			stream << ' ';
			break;
		}
		case pairFormat:{
			if(!sListNumerator){
				stream << lbr; 
				stream << input;
				sListNumerator++;
			}
			else{
				stream << ' ';
				stream << input;
				sListNumerator = 0;
			}
			break;
		}
		case lbrFormat:
		case lineBreakFormat:{
			stream << lbr;
			stream << input;
			break;
		}
		case commaFormat:{
			stream << input;
			stream << ", ";
			break;
		}
		case commaBreakFormat:{
			stream << lbr;
			stream << input;
			stream << ",";
			break;
		}
		//will deprecate as soon as proper error functions are written
		case errorFormat:{
			if(!sListNumerator){
				stream << "\nERROR ";
				//stream << g_ERROR;	//global errors are WIP
				//stream << &input;
				stream << ": ";
				sListNumerator++;
			}
			stream << input;
			break;
		}
		case listFormat:{
			stream << lbr;
			stream << "•";
			stream << input;
			break;
		}
		case noFormat:
		default:{
			stream << input;
			break;
		}
	}
	out(stream, *(&format), inputRecursive...);	//ref-deref important, stop removing it!
	return stream;
}

//output function for basic arrays. length is a uLong because size_t is a uLong.
//be sure to set the length properly, as the iterators do NOT check values.
//use FormatEnums for format.

///NOTE: will need to deprecate/rewrite this. the implementation isnt very good!
/// maybe make class-specific overloads instead?
//template<typename inputType>
//std::ostream& out(std::ostream& stream, FormatEnum format, sizet length, inputType* input){
//
//if(length < 2)
//	return out(stream, format, *input);	//a single-element array need not be processed specially
//switch(format){
//	case numberedFormat:{
//		for(sizet i = 0; i < length; i++){
//			stream << lbr;
//			stream << sListNumerator++;
//			stream << ". ";
//			stream << input[i];
//		}
//		stream << lbr;
//		sListNumerator = 0;
//		break;
//	}
//	case spaceFormat:{
//		for(sizet i = 0; i < length; i++){
//			stream << input[i];
//			stream << " ";
//		}
//		break;
//	}
//	case commaBreakFormat:{
//		for(sizet i = 0; i < length-1; i++){
//			stream << input[i];
//			stream << ",";
//			stream << lbr;
//		}
//		stream << input[length-1];
//		break;
//	}
//	case lbrFormat:
//	case lineBreakFormat:{
//		for(sizet i = 0; i < length; i++){
//			stream << lbr;
//			stream << input[i];
//		}
//		stream << lbr;
//		break;
//	}
//	case pairFormat:{
//		for(sizet i = 0; i < length; i++){
//			if(!sListNumerator){
//				stream << lbr; 
//				stream << input[i];
//				sListNumerator++;
//			}
//			else{
//				stream << ' ';
//				stream << input[i];
//				sListNumerator = 0;
//			}
//		}
//		sListNumerator = 0;
//		stream << lbr; 
//		break;
//	}
//	case errorFormat:{
//			stream << "\nERROR ";
//			//stream << g_ERROR;	//global errors are still WIP
//			stream << ": ";
//		for(sizet i = 0; i < length; i++){
//			stream << " ";
//			stream << input[i];
//		}
//		stream << std::endl;
//		break;
//	}
//	case listFormat:{
//		for(sizet i = 0; i < length; i++){
//			stream << lbr;
//			stream << "•";	//platform-specific, see platform library.
//			stream << input[i];
//		}
//		stream << lbr;
//		break;
//	}
//	case commaFormat:{
//		sizet i = 0;
//		for(sizet i = 0; i < length-1; i++){
//			stream << input[i];
//			stream << ", ";
//		}
//		stream << input[length-1];
//		break;
//	}
//	case noFormat:
//	default:{
//		for(sizet i = 0; i < length; i++){
//			stream << input[i];
//		}
//		break;
//	}
//}
//return stream;
//}

//Combined line output function. use format enums to dictate output format.
///*accepts Format enums and recursive inputs. 
///*inputs will ALWAYS be the last (for recursion), and Format enum will always be first.
template<typename inputType>
std::ostream& out(FormatEnum format, inputType input){
	return out(std::cout, format, input);
}
//Combined line output function. use format enums to dictate output format.
///*accepts Format enums and recursive inputs. 
///*inputs will ALWAYS be the last (for recursion), and Format enum will always be first.
template<typename inputType, typename... recursiveType>
std::ostream& out(FormatEnum format, inputType input, recursiveType... inputRecursive){
	return out(std::cout, format, input, inputRecursive...);
}
//basic output function, assumes no formatting.
template<typename inputType>
std::ostream& out(inputType input){
	return out(std::cout, noFormat, input);
}
//basic recursive output function, assumes no formatting.
///@WARNING: might accidentally get called if proper syntax is not followed!
///Double-check to make sure youre calling this intentionally!
template<typename inputType, typename... recursiveType>
std::ostream& out(inputType input,recursiveType... inputRecursive){
	return out(std::cout, noFormat, input, inputRecursive...);
}

//file stuff is wip; not fully tested!
template<typename inputType>
std::fstream& out(std::fstream& fileOut, FormatEnum format, inputType input){
	std::streambuf* target = fileOut.rdbuf();
	std::streambuf* previousBuffer = std::clog.rdbuf(); //pointer to preset buffer
	std::clog.rdbuf(target);
	
	out(std::clog, *(&format), input);
	
	std::clog.rdbuf(previousBuffer);
	return fileOut;
}
template<typename inputType, typename... recursiveType>
std::fstream& out(std::fstream& fileOut, FormatEnum format, inputType input, recursiveType... inputRecursive){
	std::streambuf* target = fileOut.rdbuf();
	std::streambuf* previousBuffer = std::clog.rdbuf(); //pointer to preset buffer
	std::clog.rdbuf(target);
	
	out(std::clog, *(&format), input, inputRecursive...);
	
	std::clog.rdbuf(previousBuffer);
	return fileOut;
}


//refactor: i think theres a way to get rid of this. at the very least, we dont need to be recursively inlining these anyway.
//cursed self-referential overload.
//WILL enter an infinite loop if youre not careful with it,
// but its necessary for ostream inlining to work.
//std::ostream& operator<<(std::ostream& stream, std::ostream& input){
//	return input;
//}



#include <string_view>	//its literally right here, why does this IDE throw an error?
//returns true if it is acceptable to line break at this symbol, mainly for use in wordWrap().
const bool punctCheck(std::string_view& input, sizet position){
	switch(input[position]){
		//normal punctuation- will always count these as acceptable
		// for making a new line:
		case ' ' :
		case '\n' :
		case '\r' :
		case '\t' :
		case ',' :
		case '?' :
		case '!' :
		case '/' :
		case ')' :
		case '}' :
		case ']' :
		case ';' :
		case ':' :
		case '>' :
		case '_' :
		case '%' :
		case '#' :
		case '&' :
		case '@' :
		case '*' :
		case '-' :
		case '=' :
		case 0:
			return true;
		//conditional pucntuation- only breaks up the word
		//if there is whitespace after it:
		case '+' :{	//otherwise it truncates "C++" lol
			if(punctCheck(input,(position ? position-1 : 0)))
				return true;
		}
		case '\'' :
		case '$' :
		case '\"' :
		case '^' :
		case '.' :
		{
			if(punctCheck(input,(position == input.size() ? position : position+1)))
				return true;
		}
		default:
			return false;
	}
	return false;
}

//for use with wordWrap(). not explicitly suitable for other purposes.
template<typename inputType>
constexpr sizet inputCounter(inputType& input){
	sizet i = 0;    
	while(input[i])
		i++;
	return i;
}

//wraps the string based on line length. Outputs a char*.
//estimated size may be notably larger than actual data,
// a tradeoff for avoiding unnecessary copy operations
template<typename inputType>
char* wordWrap(sizet lineLength, inputType& input){
	
	
	sizet inputPos = 0;
	sizet outputPos = 0;
	sizet sincePunct = 0;
	//const sizet inputSize = inputCounter(input);
	const sizet inputSize = [&](sizet){sizet i=0; while(input[i])i++;return i;};
	const sizet outputSize = inputSize+((inputSize/lineLength)*2)+2;
	char* output = nullptr;
	
	if(lineLength<5){	//anything less than 5 won't work properly with hyphenation
		//output = new char[30]{"Error: Line length too small!"};
		output = "Error: Line length too small!";
		return output;}

	output = new char[outputSize];	//heaped output string. unnecessary? convert to stack allocation?
	for(sizet x = 0;x<outputSize;x++)
		output[x] = ' ';
	output[outputSize] = '\0';

	if(inputSize<lineLength){
		for(; input[inputPos]; inputPos++){
			output[inputPos] = input[inputPos];
		}
		return output;
	}

	while (input[inputPos]){
		if(outputPos > outputSize){
			std::cerr << "\nERROR: Output position out of bounds!";
			std::cerr << "\n last output character:";
			std::cerr << output[outputPos];
			std::cerr << "\n outputPos: ";
			std::cerr << outputPos;
			std::cerr << "\n inputPos: ";
			std::cerr << inputPos;
			std::cerr << "\n sincePunct: ";
			std::cerr << sincePunct;
			pause;
			break;
			}
		//checks to see if on the last line
		if((inputSize-inputPos) < lineLength){
			while(input[inputPos]){
				output[outputPos] = input[inputPos];
				outputPos++; inputPos++;
			}
			output[outputPos] = '\0';
			return output;
			}
		//main Line loop
		for (int linePos = 0; linePos < lineLength; linePos++){			
			if(!input[inputPos]){
				std::clog << "\nWarning: Unexpected end of string.";
				break;
			}

			if(input[inputPos] == '\n'){
				output[outputPos] = input[inputPos];
				outputPos++;
				inputPos++;
				sincePunct = 0;
				linePos = 0;
				continue;
			}

			if (punctCheck(input,inputPos) && (input[inputPos+1]) != punctCheck(input,inputPos))
				sincePunct = 0;
			else
				sincePunct++;
			
			//skips spaces at the beginning of a line
			if(!linePos && input[inputPos] == ' ')
				inputPos++;
				
			output[outputPos] = input[inputPos];
			outputPos++;
			inputPos++;
		}

		//hyphenator
		///if it reaches the end of the line, and the word is too long,
		///it will truncate the current word and add a hyphen
		//dont include ones that text is expected to come after, like '(' or '$'. 
		if(sincePunct>(lineLength >> 1)){
			if (punctCheck(input,*(&inputPos+2))){
				outputPos-=1;	//moves back a character
				inputPos-=1;	//
			}
			else{
				outputPos-=2;
				inputPos-=2;
			}
			if (!(punctCheck(input,inputPos))){
				output[outputPos] = '-';	//if current character is not a space or punctuation, inserts a hyphen
			}
			else{
				inputPos++;	//if punctuation is found, simply moves to the next character in the input and sets the punct counter to 1
				sincePunct = 1;}
			outputPos++;
			if (output[outputPos] != '\n'){
				output[outputPos] = '\n';
				sincePunct = 0;}
			else{
				inputPos++;
				sincePunct = 1;}
			outputPos++;
			continue;
		}

		//primary line wrapping:
		outputPos-=sincePunct;
		inputPos-=sincePunct;
		output[outputPos] = '\n';
		sincePunct = 0;
		outputPos++;
		continue;
	}
	return output;
}
/*
wraps input based on line length, and returns it as a char[].
comprehensive overload Template for non-string inputs and non-ref values
*/
// template<typename inputType>
// inline char * wordWrap(inputType input, int lineLength){
// 	return wordWrap(std::string(input), lineLength);
// }
//
////empties cin stream into dummy buffer to keep it from interfering with other streams
//inline void streamClear(){
//	std::string dummyString;
//	getline(std::cin, dummyString);
//}


//manual formatting tools:
//can be inlined like so:
//
// std::cout << listMinor << "im a wee list item!";
// 
// out(listMinor,"im a wee list item!");

std::ostream& listMajor(std::ostream& stream){
	stream << lbr;
	stream << "•";
	return stream;
}
std::ostream& listMinor(std::ostream& stream){
	stream << lbr;
	stream << " -";
	return stream;
}
//dont forget to clear sListNumerator!
std::ostream& list_num(std::ostream& stream){
	
	//global uint32. used to hold numbers for numbered and paired format out() functions.
	//Make sure you clean it up when you're done with it!
	static uint32 sListNumerator = 0;
	
	stream << lbr;
	stream << sListNumerator++;
	stream << ". ";
	return stream;
}


//cleans string of non-printing characters.
//WIP: only supports narrow ASCII. for unicode, may be better to reverse switch statement so default is for printing characters.
//last parameter is false to replace with spaces, true to delete and shift everything over.
template<typename inType>
inType& sanitizePlainText(inType& input, const sizet length, const bool remove){
	if(!length || sizeof(inType)>1)
		return input;
	
	//use continue to ignore, break to remove/replace.
	for(sizet i = 0; i < length; i++){
		switch(((char*)(&input))[i]){
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '0':
			case ' ':
			case '~':
			case '!':
			case '@':
			case '#':
			case '$':
			case '%':
			case '&':
			case '*':
			case '(':
			case ')':
			case '-':
			case '_':
			case '+':
			case '=':
			case '[':
			case ']':
			case '{':
			case '}':
			case '\\':
			case '|':
			case ';':
			case ':':
			case '\'':
			case '\"':
			case ',':
			case '.':
			case '/':
			case '<':
			case '>':
			case '?':
			case '\n':
			case '\t':{
				continue;
			}
			case '^':{	//to clean escape sequences on most terminals. check flags or terminal settings?? needs testing.
				if(length > 1 && ((char*)(&input))[i+1] == '['){
					i++;
					break;
				}
				continue;
			}
			case '\0':{
				std::clog << "\nWARNING:" << &input << " Unexpected termination character encountered during sanitization at index " << i << "!";
				break;	//unsure of desired behavior here. for now, will remove/replace.
			}
			default:{
				break;
			}
		}
		if(remove)
			((char*)(&input))[i] = '\0';
		else
			((char*)(&input))[i] = ' ';
	}
	
	if(remove){
		sizet pos = 0;
		for(sizet i = 0; pos < length && i < length; i++){
			if(!((char*)(&input))[pos]){
				pos++;
			}
			((char*)(&input))[i] = ((char*)(&input))[pos];
			pos++;
		}
		for(sizet i = length-pos; i < length; i++)
			((char*)(&input))[i] = '\0';
	}
	
	return input;
}
template<typename inType>
inType& sanitizePlainText(inType& input, sizet length){return sanitizePlainText(input, length, false);}
template<typename inType>
inType& sanitizePlainText(inType& input, const bool remove){
	sizet i = 0;
	constexpr sizet max = maxS32;
	
	for(; i < max && input[i] != '\0'; i++){}
	
	if(i > (1<<16))
		std::clog << &input << "\nWARNING:" << &input << " Sanitizer automatic length guessing returned very large value: " << i;
	
	return sanitizePlainText(input, i, remove);
}
template<typename inType>
inType*& sanitizePlainText(inType*& input){
	sizet i = 0;
	constexpr sizet max = maxS32;
	
	for(; i < max && input[i] != '\0'; i++){}
	
	if(i > (1<<16))
		std::clog << &input << "\nWARNING:" << &input << " Sanitizer automatic length guessing returned very large value: " << i;
	
	return sanitizePlainText(input, i, false);
}