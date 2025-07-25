#pragma once
#include <cmath>





#define oneThird (1.0/3.0)
#define twoThirds (2.0/3.0)
#define fourThirds (oneThird+1)
#define oneQuarter 0.2500000000000000
#define oneEighth 0.1250000000000000
#define oneHalf 0.5000000000000000
#define threeHalfs 1.5000000000000000

//need to test to see how it stacks up against stdlib versions
template<typename inputType>
inputType exponent(inputType input, const int exp){
	const inputType factor = input;
	if(!exp)
		return 1;
	if(exp == 1)
		return input;
		
	else if(exp>0)
		for(int i = 0; i<exp; i++)
			input*=factor;
			
	else if(exp<0){
		for(int i = 0; i>exp; i--)
			input*=factor;
		input=(1/input);
	}
	
	return input;
}
template<typename inputType>
inputType exponent(inputType&& input, const int exp){
	const inputType factor = input;
	if(!exp)
		return 1;
	if(exp == 1)
		return input;
		
	else if(exp>0)
		for(int i = 0; i<exp; i++)
			input*=factor;
			
	else if(exp<0){
		for(int i = 0; i>exp; i--)
			input*=factor;
		input=(1/input);
	}
	
	return input;
}

//template<typename inputType>
//inputType sq(inputType input){
//	return input * input;
//}
template<typename inputType>
constexpr inputType sq(inputType&& input){
	return input * input;
}
template<typename inputType>
inputType sq(const inputType& input){
	return input * input;
}


