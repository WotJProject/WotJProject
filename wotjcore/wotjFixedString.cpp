#pragma once
#include "..wotjLib.cpp"
#include <iostream>
#include <string>	//only for handling streams
#include <cstring>	//for std::memcpy/memmove/etc
#include <fstream>	//only for reading/writing operator overloads.

//WIP: 
// copy functions/constructors, etc. for more types
// memcpy instead of for loops?

/*
fixed-size string designed for speed and ease of use.
-the string itself can be changed with rewrite(), but not the buffer size nor the terminator.
-the entire class will have a fixed size on compile (wip: change to declaration?).
*/
template<sizet inputLength>
class FixedString{
	private:
	struct {	//whole struct seems to get stored in const memory... ignore compiler warnings lmao its prolly fine
		const sizet length = inputLength;
		char raw[inputLength];	//wip: its now throwing errors, for some reason
		const char terminator = '\0';
	};
	
	public:
	const char* const data = raw;
	
	FixedString() :raw({0}) {}
	
	FixedString(const char* input){
		if(!input)
			return;
		for(sizet i = 0; i<length; i++){
			raw[i] = input[i];
			if(!input[i])
				break;
		}
	}
	FixedString(char* input) :raw({0}){
		if(!input)
			return;
		for(sizet i = 0; i<length; i++){
			raw[i] = input[i];
			if(!input[i])
				break;
		}
	}
	FixedString(const void* &input) :raw({0}){	//wip: doesnt support wide characters or unicode
		if(!input)
			return;
		
		char* inputRef = (char*)(input);
		
		for(sizet i = 0; i<length; i++){
			raw[i] = inputRef[i];
			if(!inputRef[i])
				break;
		}
		debug(false);
	}
	//needs final testing
	FixedString(const FixedString& input) :raw({0}){
		if(!input.getLength())
			raw[0] = '\0';
			
		if (length <= input.getLength())
			std::memcpy(raw, input.data, length);
		else if (length > input.getLength()){
			std::memcpy(raw, input.data, input.getLength());
			raw[input.getLength()] = '\0';
		}
	}
	FixedString(const std::string& input) :raw({0}){
		for(sizet i = 0; i<length; i++){
			raw[i] = input[i];
			if(!input[i])
				break;
		}
	}
	FixedString(std::nullptr_t inputNull) :raw({0}){}
	FixedString(int input):raw(char(input)){debug();}	//refactor: this was mainly for nullptr value support, but it can be expanded to read all 4 bytes
	
	~FixedString(){rewrite(0);}
	
	template<typename compareType>
	bool operator==(compareType comparator) const {
		if(data[0] != comparator[0])
			return false;
		for(sizet i = 0; i<readLength(); i++)
			if(data[i] != comparator[i])
				return false;
		if(!comparator[readLength()])	//is checking the terminator necessary?
			return true;
		return false;
	}
	template<typename compareType>
	inline bool operator!=(compareType comparator) const {
		return (*this == comparator) == false;
	}
	
	char operator[](const sizet index) const {if(index < length)return data[index]; else return '\0';}
	char operator[](sizet& index) const {if(index < length)return data[index]; else return '\0';}
	char operator[](sizet index) {if(index < length)return data[index]; else return '\0';}
	
	const char* operator=(char* inputData){
		if(!inputData)
			return rewrite(0);
		return rewrite(inputData);
	}
	const char* operator=(const char* inputData){
		if(!inputData)
			return rewrite(0);
		return rewrite(inputData);
	}
	
	inline const char* operator=(int){return rewrite(0);}
	inline const char* operator=(std::nullptr_t inputNull){return rewrite(0);}
	
	const char* operator=(void* inputData){
		if(!inputData || !((char*)inputData)[0])
			return rewrite(0);
		char* inputRef = inputData;
			return rewrite(inputRef);
	}
	template<sizet cpySize>
	const char* operator=(FixedString<cpySize>& inputData){
		return rewrite(inputData);
	}
	bool operator!(){
		return !data[0];
	}
	
	//wip- concatenation
	//const char* operator+(const char* input){
	//	wip;
	//	if(readLength() == input)
	//		return data;
	//	
	//	sizet addLength = 0;
	//	for(sizet i = 0; input[i] && i<length; i++)
	//		addLength++;
	//		
		//output = new char[addLength+readLength()];
		//atexit([&](){delete output;});
	//	*output = *data;
	//	
	//	sizet pos = readLength();
	//	for(sizet i = 0; pos<length && pos<(readLength()+addLength); i++){
	//		output[pos] = input[i];
	//		pos++;
	//	}
	//	return output;
	//}
	
	
	//added for compatibility with pointer arithmetic that relies on char*s
	constexpr char* operator+(sizet offset) const {return &raw[0] + offset;}
	constexpr char* operator+(int offset) const {return &raw[0] + offset;}
	constexpr char* operator+(unsigned int offset) const {return &raw[0] + offset;}
	
	constexpr char* operator-(sizet offset) const {return &raw[0] - offset;}
	constexpr char* operator-(int offset) const {return &raw[0] - offset;}
	constexpr char* operator-(unsigned int offset) const {return &raw[0] - offset;}
	
	friend std::ostream& operator<<(std::ostream& stream, FixedString& input){
		return stream << input.data;
	}
	friend std::istream& operator>>(std::istream& stream, FixedString& input){
		std::string inputCopy;
		stream >> inputCopy;
		input.rewrite(inputCopy);
		return stream;
	}
	
	//returns readable data length by iteration
	sizet readLength() const {
		sizet i = 0;
		while(data[i]){i++;}
		return i;
	}
	
	//returns true data length by memory allocation
	constexpr const sizet trueLength() const {
		return &terminator - &raw[0];
	}
	
	constexpr const char* const getData() const {return data;}
	constexpr const char* const get() const {return data;}
	
	//returns size in bytes
	constexpr sizet getSize() const {return trueLength();}
	//returns size in bytes
	constexpr sizet size() const {return trueLength;}
	
	//returns size in characters
	const sizet getLength() const {return readLength();}
	//returns size in characters
	constexpr sizet count() const {return trueLength;}
	
	//use this to reassign the string
	const char* rewrite(const char* inputData){
		if(!inputData[0]){
			for(sizet i = 0; i<length; i++)
				raw[i] = '\0';
			return data;
		}
		for(sizet i = 0; i<length; i++){
			raw[i] = inputData[i];
			if(!inputData[i])
				break;
		}
		
		debug(false);
		return data;
	}
	template<sizet rwSize>
	const char* rewrite(FixedString<rwSize> &inputClass){
		
		if(!inputClass.getLength())
			return rewrite(0);
			
		if (length <= inputClass.getLength())
			std::memcpy(raw, inputClass.data, length);
		else if (length > inputClass.getLength()){
			std::memcpy(raw, inputClass.data, inputClass.getLength());
			raw[inputClass.getLength()] = '\0';
		}
		debug(false);
		return data;
	}
	inline const char* rewrite(std::string inputString){
		if(!inputString[0] || !inputString.length())
			return rewrite(0);
		else
			return rewrite(inputString.c_str());
	}
	//NOTE: may have weird behavior, because streams have weird behavior.
	inline const char* rewrite(std::istream& inputStream){
		std::string inputCopy;
		inputStream >> inputCopy;
		return rewrite(inputCopy);
	}
	inline const char* rewrite(){return rewrite(0);}
	inline const char* clear(){return rewrite(0);}
	
	//wip: file reads/writes need testing
	std::fstream& operator<<(std::fstream& stream){
		out("\ndebug: fstream store operator called.");
		stream << length;
		stream << data;
		stream << '\0';
		return stream;
	}
	std::fstream& operator>>(std::fstream& stream){
		out("\ndebug: fstream read operator called.");
		sizet inLength = 0;
		stream >> inLength;
		
		const char inData[inLength];
		stream >> inData;
		
		if(length >= inLength){
			memcpy(raw, inData, inLength);
		}else{
			memcpy(raw, inData, length);
		}
		return stream;
	}
	
	//wip
	//returns true if it finds a problem, and then attempts to fix it.
	//have yet to be able to cause one, so i cant test it.
	bool debug(std::ostream& stream, bool print){
		if(print){
			stream << "\n\nFixedString debug information:\ndata:\n";
			stream << data;
			stream << "\nlength: ";
			stream << length;
			stream << "\nterminator: ";
			stream << (int)terminator;
			stream << "\nread length: ";
			stream << readLength();
			stream << "\ntrue length: ";
			stream << trueLength(); 
			stream << "\nData dump:\n Raw char print:\n";
			for(sizet i = 0; i<trueLength()+sizeof(sizet); i++)
				stream << *(&raw[i]-sizeof(sizet));
			stream << "\n Raw hex values:\n";
			for(sizet i = 0; i<trueLength()+sizeof(sizet); i++){
				stream << std::endl;
				stream << i << ": ";
				stream << std::hex << (sint16)*(&raw[i]-sizeof(sizet));
			}
			endLine;
		}
		
		if (data != raw || length != inputLength || terminator){
			std::clog << "\nERROR: " << &length << "FixedString malformed!" << std::endl;
			try{
				stream << " Attempting to fix...\n";
				*(&raw[0]-sizeof(sizet)) = inputLength;
				
				//data = raw;	//wip: use type punning or something to try and fix.
				
				//void** fixData = &((void*)(*(&data)));
				//void** fixData = &(*(&data));
				//char** fixData = &data;
				//fixData = &raw;
				//*(&data) = raw;
				
				//note: may have to return a new FixedString instance since we double-const'd it
				
				raw[inputLength] = 0;	//this may generate an error depending on compiler and flags
				
				stream << " Testing:\n" << this;
				stream << std::endl;
			}
			catch(...){
				stream << "\nERROR: "<< &length << "Attempt to fix failed!" << std::endl;
				return true;
			}
			stream << " Fix successful! Hopefully!" << std::endl;
			return true;
		}
		return false;
	}
	inline bool debug(bool print){return debug(std::clog, print);}
	inline bool debug(){return debug(std::clog, false);}
};
