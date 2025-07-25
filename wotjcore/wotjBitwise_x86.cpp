#pragma once
#include "wotjLib.cpp"

//refactor: oops learned about Two's Complement; excluding the sign bit wont give an absolute value for negative values
#define maskSign8 0xcf	//bitmask to exclude sign bit. 8 bit ver.
#define maskSign16 0xcfff	//bitmask to exclude sign bit. 16 bit ver.
#define maskSign32 0xcfffffff	//bitmask to exclude sign bit. 32 bit ver.
#define maskSign64 0xcfffffffffffffff	//bitmask to exclude sign bit. 64 bit ver.
// #define maskSign128 0xcfffffffffffffffffffffffffffffff	//bitmask to exclude sign bit. 128 bit ver.

#define maskFloat_sign 0x80000000   //mask to isolate the sign
#define maskFloat_exponent 0x3f800000   //mask to isolate the exponent
#define maskFloat_mantissa 0x007fffff   //mask to isolate the mantissa

#define maskDouble_sign 0x8000000000000000  //mask to isolate the sign
#define maskDouble_exponent 0x3ff8000000000000  //mask to isolate the exponent
#define maskDouble_mantissa 0x000fffffffffffff  //mask to isolate the mantissa

#define charsint(input) ((sint16)input & 0b1000000001111111)	//read char as signed numeric value. wip: needs final testing.
#define charuint(input) ((uint16)input & 255)	//read char as unsigned numeric value. wip: need  to do final testing on negative value conversion
//note: look up low-level optimizations for this. is int better, or is short fine? if int is better, we can just use the + operator.

#define bitSwap(a,b) a^=b; b^=a; a^=b	//swaps two values. make sure theyre the same size!

#define maxU8 0xffU
#define maxU16 0xffffU
#define maxU32 0xffffffffU
#define maxU64 0xffffffffffffffffU

#define maxS8 0x7f
#define maxS16 0x7fff
#define maxS32 0x7fffffff
#define maxS64 0x7fffffffffffffff

#define maxN8 0x80
#define maxN16 0x8000
#define maxN32 0x80000000
#define maxN64 0x8000000000000000

#define maxAuto (~0)	//will default to a regular int (sint32, -1) if type cannot be deduced


//prints out binary of input. mask is optional.
template<typename inputType>
std::ostream& printBinary(std::ostream& stream, const inputType input, const inputType mask){
	const inputType output = input & mask;
	constexpr sizet inputSize = sizeof(input)*8;	//size of input in bits
	char outputPrint[inputSize+1];
	
	for(sizet i=0; i<inputSize; i++)
		outputPrint[i] = (output & (1<<(inputSize-i-1)) ? '1' : '0');
	outputPrint[inputSize] = '\0';
	
	return stream << outputPrint;
}
//prints out binary of input. mask is optional.
template<typename inputType>
std::ostream& printBinary(const inputType input) {
	const inputType mask = ~0;
	return printBinary(std::cout, input, mask);
}

//wip: needs final testing
//sets bit at index.
template<typename inputType>
inputType setBinary (inputType &input, const int index, const bool value) {
	if(index > 8*(sizeof input)-1)	//bounds checking
		return input;
	
	input = input & ~(1<<index) + ((value>0)<<index);
	
	return input;
}



//emulates bitshifting a floating point variable as if it were a regular integer, dividing or multiplying by 2.
//uses a Union. shift is bitshift amount, or 2^shift
double bitshiftFloating_union(double inputData, const int shift){
	union {
		double input;
		struct {
			uint64 mantissa : 52;
			uint64 exponent : 11;
		};
	} U;
	
	U.input = inputData;
	U.exponent += shift;
	inputData = U.input;
	return inputData;
}
//emulates bitshifting a floating point variable as if it were a regular integer, dividing or multiplying by 2.
//uses a Union. shift is bitshift amount, or 2^shift
float bitshiftFloating_union(float inputData, const int shift){
	union {
		float input;
		struct {
			uint32 mantissa : 23;
			uint32 exponent : 8;
		};
	} U;
	
	U.input = inputData;
	U.exponent += shift;
	inputData = U.input;
	return inputData;
}

//wip: need to test negative values, i think i messed up somewhere
//	maybe multiply shift<<23 by (-1*(shift<0))?
//emulates bitshifting a floating point variable as if it were a regular integer, dividing or multiplying by 2.
//uses a bitmask. shift is bitshift amount, or 2^shift
float bitshiftFloating_mask(float inputData, uint32 shift){
	uint32 holding = *((uint32*)(&inputData));	//refactor: combine these
	const uint32 exponent = (holding + (shift<<23)) & maskFloat_exponent;
	
	holding = (holding & 0xc07fffff) + exponent;
	
	inputData = *((float*)(&holding));
	return inputData;
}
//emulates bitshifting a floating point variable as if it were a regular integer, dividing or multiplying by 2.
//uses a bitmask. shift is bitshift amount, or 2^shift
double bitshiftFloating_mask(double inputData, uint64 shift){
	uint64 holding = *((uint64*)(&inputData));
	uint64 exponent = (holding + (shift<<52)) & maskDouble_exponent;

	holding = (holding & 0xc007ffffffffffff) + exponent;
	
	inputData = *((double*)(&holding));
	return inputData;
}


//WIP. faster but less-precise n-root for floating-point numbers. Defaults to square root (2).
// void sqrtFloating(double& inputData, uchar inputRoot){
// //square exponent, square mantissa in reverse direction
// 	unionDouble U;
// 	U.input = inputData;
// 	for(uchar i = 11;i;i--){
// 		U.bits.exponent bit i |= bit i-1;
// 		}
// 	for(uchar i = 0; i<52; i++){
// 		U.bits.exponent bit i |= bit i+1;
// 	}
// 	inputData = U.input;
// 	return;
// }
//faster n-root for floating-point numbers. Defaults to square root (2).
// void sqrtFloating(float& inputData, uchar inputRoot){
// 	unionFloat U;
// 	U.input = inputData;
// 	U.bits.exponent >> inputRoot;
// 	inputData = U.input;
// 	return;
// }
// //faster n-root for floating-point numbers. Defaults to square root (2).
// template<typename inputType>
// void sqrtFloating(inputType& input){
// 	sqrtFloating(input,1);
// }

//overload for division operator. WIP.
// template<typename inputType>
// double& operator/ (double &input, inputType inputDivisor){
// 	if (inputDivisor bit 0 == 1)
// 		return input/inputDivisor;
// 	else{

// 		return bitshiftFloating(input, inputDivisor);
// 	}
// }



//breaks down float into component bits. define with unionFloat NAME, then use NAME.input to set, and ex: NAME.bits.mantissa to return.
union unionFloat{
    float input;	//primary float container
	struct {	//struct for bit-field array; float = -1^sign * 2^(exponent-127) * 1.(mantissa)
    	uint32 mantissa : 23;	//fraction data; extract to a dummy unionFloat.mantissa to get real value, or manually: 1+(bit1*2^-1,...bit23*2^-23)
    	uint32 exponent : 8;	//biased, so -127 to get real value.
    	uint32 sign : 1;	//sign bit.
	} bits;
	unsigned char bytes[4];	//individual byte array
};

//breaks down double into component bits. define with unionDouble NAME, then use NAME.input to set, and ex: NAME.bits.mantissa to return.
union unionDouble{
    double input;	//primary container
	struct {	//struct for bit-field array; double = -1^sign * 2^(exponent-1023) * 1.(mantissa)
    	uint64 mantissa : 52;	//fraction data; extract to a dummy unionDouble.mantissa to get real value, or manually: 1+(bit1*2^-1,...bit52*2^-52)
    	uint64 exponent : 11;	//biased, so -1023 to get real value.
    	uint64 sign : 1;	//sign bit.
	} bits;
	uint8 bytes[8];	//individual byte array
};

//breaks down int into component bits. define, then use NAME.input to set.
union unionInt {
	int input;	//primary int container
	struct {	//struct for bit-field array
		uint32 data : 32;	//raw numerical data
	} bits;
	struct {	//struct for signed bit-field array
		uint32 data : 31;	//raw numerical data
		uint32 sign : 1;	//sign bit
	} signedBits;
	uint8 bytes[4];	//individual byte array
};

//breaks down signed char into component bits. define, then use NAME.input to set.
union unionChar {
	char input;	//primary char container
	struct {	//struct for bit-field array
		uint8 data : 8;	//raw numerical data
	} bits;
	struct {	//struct for bit-field array
		uint8 data : 7;	//raw numerical data
		uint8 sign : 1;	//sign bit
	} signedBits;
	uint8 byte;	//unsigned byte access
};

//generic n-byte union that provides direct byte access. use NAME.input to set.
template<typename inputType>
union unionByte {
	inputType input;
	struct {	//struct for bit-field array
		uint64 data : (8*(sizeof input));	//raw bit data.
	} bits;
	uint8 bytes[sizeof input];	//individual byte array
};

union Pun16{
	uint16 ui16Pun;
	sint16 si16Pun;
	uint8 ui8Pun[2];
	sint8 si8Pun[2];
	wchar wcharPun;
};

//32-bit union for multiple type punning
union Pun32{
	sint32 si32Pun;	//signed int
	uint32 ui32Pun;	//unsigned int
	keyt keytPun;
	long longPun;	//signed long
	unsigned long ulongPun;	//unsigned long
	sfloat32 sf32Pun;	//float
	sint16 si16Pun[2];	//signed short
	uint16 ui16Pun[2];	//unsigned short
	wchar wcharPun[2];
	sint8 si8Pun[4];	//signed char
	uint8 ui8Pun[4];	//unsigned char
	struct {
		uint32 data : 31;
		uint32 sign : 1;
	} intBits;
	struct {
		uint32 mantissa : 23;
		uint32 exponent : 8;
		uint32 sign : 1;
	} floatBits;
};
//64-bit union for multiple type punning
union Pun64{
	sint64 si64Pun;	//signed longlong
	uint64 ui64Pun;	//unsigned longlong
	sizet sizetPun;
	sfloat64 sf64Pun;	//double
	sfloat32 sf32Pun[2];	//float
	sint32 si32Pun[2];	//signed int
	long longPun[2];	//signed long
	unsigned long ulongPun[2];	//unsigned long
	keyt keytPun[2];
	uint32 ui32Pun[2];	//unsigned int
	sint16 si16Pun[4];	//signed short
	uint16 ui16Pun[4];	//unsigned short
	sint8 si8Pun[8];	//signed char
	uint8 ui8Pun[8];	//unsigned char
	struct {
		uint64 data : 63;
		uint64 sign : 1;
	} intBits;
	struct {
		uint64 mantissa : 52;
		uint64 exponent : 11;
		uint64 sign : 1;
	} floatBits;
};

//8-bit Flag union. use value for unsigned 8-bit access, or flag# for specific flags
union flagU8 {
	uint8 value;	//raw numeric representation
	struct {
		bool flag1:1;
		bool flag2:1;
		bool flag3:1;
		bool flag4:1;
		bool flag5:1;
		bool flag6:1;
		bool flag7:1;
		bool flag8:1;
	};
};


//Flag class for easy bitwise flags. WIP: add math operators, fix bugs
//8-bit ver
class Flag8 {	//refactor: rename to distinguish from union version
public:
	union{
		uint8 value;	//raw numeric representation
		struct {
			bool flag1:1;
			bool flag2:1;
			bool flag3:1;
			bool flag4:1;
			bool flag5:1;
			bool flag6:1;
			bool flag7:1;
			bool flag8:1;
		};
	};
	
	Flag8():
		value(0)
	{}
	
	Flag8(const uint8&& input):
		value(input)
	{}
	
	Flag8(const uint8& input):
		value(input)
	{}
	
	Flag8(const Flag8& input):
		value(input.value)
	{}
	
	template<typename inType>
	Flag8(const inType& input):
		value((uint8)(input))
	{}
	template<typename inType>
	Flag8(const inType&& input):
		value((uint8)(input))
	{}
	
	Flag8(bool input1, bool input2, bool input3, bool input4, bool input5, bool input6, bool input7, bool input8):
		value(((input1>0)<<7)
			+((input2>0)<<6)
			+((input3>0)<<5)
			+((input4>0)<<4)
			+((input5>0)<<3)
			+((input6>0)<<2)
			+((input7>0)<<1)
			+(input8>0))
	{}
	
	Flag8 operator=(const uint8& input){
		value = input;
		return *this;
	}
	Flag8 operator=(const uint8&& input){
		value = input;
		return *this;
	}
	
	template<typename inType>
	Flag8 operator=(inType input){
		if(sizeof(input)>1)
			value = (uint8)(input);
		else 
			value = *(&input);
		return *this;
	}
	//
	//bool operator[](const int index){
	//	switch(index){
	//		case 0:{return flag1;}
	//		case 1:{return flag2;}
	//		case 2:{return flag3;}
	//		case 3:{return flag4;}
	//		case 4:{return flag5;}
	//		case 5:{return flag6;}
	//		case 6:{return flag7;}
	//		case 7:{return flag8;}
	//		default:
	//			return false;
	//	}
	//	return false;
	//}
	bool operator[](const int& index){
		switch(index){
			case 0:{return flag1;}
			case 1:{return flag2;}
			case 2:{return flag3;}
			case 3:{return flag4;}
			case 4:{return flag5;}
			case 5:{return flag6;}
			case 6:{return flag7;}
			case 7:{return flag8;}
			default:
				return false;
		}
		return false;
	}
	template<typename indexType>
	constexpr bool operator[](const indexType&& index){
		switch(index){
			case 0:{return flag1;}
			case 1:{return flag2;}
			case 2:{return flag3;}
			case 3:{return flag4;}
			case 4:{return flag5;}
			case 5:{return flag6;}
			case 6:{return flag7;}
			case 7:{return flag8;}
			default:
				return false;
		}
		return false;
	}
	bool operator[](const sizet& index){
		switch(index){
			case 0:{return flag1;}
			case 1:{return flag2;}
			case 2:{return flag3;}
			case 3:{return flag4;}
			case 4:{return flag5;}
			case 5:{return flag6;}
			case 6:{return flag7;}
			case 7:{return flag8;}
			default:
				return false;
		}
		return false;
	}
	template<typename indexType>
	bool operator[](const indexType& index){
		switch(index){
			case 0:{return flag1;}
			case 1:{return flag2;}
			case 2:{return flag3;}
			case 3:{return flag4;}
			case 4:{return flag5;}
			case 5:{return flag6;}
			case 6:{return flag7;}
			case 7:{return flag8;}
			default:
				return false;
		}
		return false;
	}
	
	
	template<typename convType>
	operator convType(){return (convType)(value);}
	//operator uint8(){return value;}
	
	//bitwise operators
	Flag8 operator&(uint8 input) const {return value & input;}
	Flag8 operator&=(uint8 input){return value = value & input;}
	
	Flag8 operator|(uint8 input) const {return value | input;}
	Flag8 operator|=(uint8 input){return value = value | input;}
	
	Flag8 operator^(uint8 input) const {return value ^ input;}
	Flag8 operator^=(uint8 input){return value = value ^ input;}
	
	Flag8 operator~() const {return ~value;}
	
	Flag8 operator>>(int shift){return value>>shift;}
	Flag8 operator<<(int shift){return value<<shift;}
	
	//boolean operators
	bool operator!() const {return !value;}
	
	bool operator==(const uint8 comparator) const {return value == comparator;}
	bool operator!=(const uint8 comparator) const {return value != comparator;}
	
	bool operator<(const uint8 comparator) const {return value < comparator;}
	bool operator>(const uint8 comparator) const {return value > comparator;}
	
	bool operator<=(const uint8 comparator) const {return value <= comparator;}
	bool operator>=(const uint8 comparator) const {return value >= comparator;}
	
	bool operator==(const Flag8 comparator) const {return value == comparator.value;}
	bool operator!=(const Flag8 comparator) const {return value != comparator.value;}
	
	bool operator<(const Flag8 comparator) const {return value < comparator.value;}
	bool operator>(const Flag8 comparator) const {return value > comparator.value;}
	
	bool operator<=(const Flag8 comparator) const {return value <= comparator.value;}
	bool operator>=(const Flag8 comparator) const {return value >= comparator.value;}
	
	//math operators
	template<typename opType>
	opType operator+(const opType operand) const {return value+operand;} 
	template<typename opType>
	Flag8 operator+=(const opType operand) {return *this = value+operand;} 
	
	template<typename opType>
	opType operator-(const opType operand) const {return value-operand;} 
	template<typename opType>
	Flag8 operator-=(const opType operand) {return *this = value-operand;} 
	
	template<typename opType>
	opType operator*(const opType operand) const {return value*operand;} 
	template<typename opType>
	Flag8 operator*=(const opType operand) {return *this = value*operand;} 
	
	template<typename opType>
	opType operator/(const opType divisor) const {return value/divisor;} 
	template<typename opType>
	Flag8 operator/=(const opType divisor) {return *this = value/divisor;}
	
	
	//wip: is it faster to use std::hex, or convert it to an int? will need to run tests.
	//	also, can probably combine these into a char[8]
	std::ostream& operator<<(std::ostream& stream){
		char print[8] = {flag8,flag7,flag6,flag5,flag4,flag3,flag2,flag1};
		stream << std::hex;
		stream << flag8;
		stream << flag7;
		stream << flag6;
		stream << flag5;
		stream << flag4;
		stream << flag3;
		stream << flag2;
		stream << flag1;
		stream << std::dec;
		
		return stream;
	}

	std::istream& operator>>(std::istream& stream){
		stream >> value;
		return stream;
	}

};
