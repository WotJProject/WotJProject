/*
This file should be complex number types such as Cycle and BoundNumber

*/
#pragma once
#include "..wotjLib.cpp"
#include <iostream>
#include <cmath>

//refactor: struct vs class??? need to rewatch some videos
// make min and max values constexpr? we would need to copy to a new one every time it is changed, but it would provide a potential performance boost, especially on optimization...

//number bound between two values
//set using {data,min,max}
template<typename dataType>
class BoundNumber {
public:
	
	dataType data = 0;
	dataType minValue = 0;
	dataType maxValue = 0;
	
	const dataType range() const {return abs(maxValue-minValue);}

	BoundNumber(){}
	
	BoundNumber(const BoundNumber& input):
		data(input.data),
		minValue(input.minValue),
		maxValue(input.maxValue)
	{}
	
	BoundNumber(const BoundNumber&& input):
		data(input.data),
		minValue(input.minValue),
		maxValue(input.maxValue)
	{}
	
	BoundNumber(const dataType input):
		data(input),
		minValue(input),
		maxValue(input)
	{}
	
	BoundNumber(const dataType input1, const dataType input2, const dataType input3):
		data(input1),
		minValue(input2),
		maxValue(input3)
	{
		debug(false);
	}
	
	///deprecating: these just complicate implementation. if inputs cannot be converted to the same type, itll just cause more problems down the road.
	//template<typename dataType2>
	//BoundNumber(const dataType2 input1, const dataType input2, const dataType input3):
	//	data((dataType)(input1)),
	//	minValue(input2),
	//	maxValue(input3)
	//{
	//	debug(false);
	//}
	////maxValue is MOST LIKELY to be the largest number, so in a 3-type input,
	//// it is favored as the main type.
	//template<typename dataType2, typename dataType3>
	//BoundNumber(const dataType3 input1, const dataType2 input2, const dataType input3):
	//	data((dataType)(input1)),
	//	minValue((dataType)(input2)),
	//	maxValue(input3)
	//{
	//	debug(false);
	//}
	
	//input type must be the same as the object type; this helps ensure it is convertable to the data
	dataType operator=(const dataType input){
		data = input;
		debug(false);
		return data;
	}
	
	template<typename rhsType>
	dataType operator+(const rhsType rhs){
		if(!rhs || !range())
			return data;
		if(rhs>0){
			if(maxValue - data < rhs)
				return maxValue;
			return data + rhs;
		}
		else if(rhs<0){
			if(data - minValue > rhs)
				return minValue;
			else
				return data + rhs;
		}
		return data;
	}
	template<typename rhsType>
	dataType operator+=(const rhsType rhs){return *this = operator+(rhs);}
	dataType operator+(const BoundNumber& rhs){return *this + rhs.data;}
	dataType operator+=(const BoundNumber& rhs){return *this = operator+(rhs.data);}
	
	template<typename rhsType>
	dataType operator-(const rhsType rhs){
		if(!rhs || !range())
			return data;
		if(rhs>0){
			if(data - minValue < rhs)
				return minValue;
			else
				return data - rhs;
		}
		else if(rhs<0){
			if(maxValue - data < -rhs)
				return maxValue;
			else
				return data - rhs;
		}
		return data;
	}
	template<typename rhsType>
	dataType operator-=(const rhsType rhs){return *this = operator-(rhs);}
	dataType operator-(const BoundNumber& rhs){return *this - rhs.data;}
	dataType operator-=(const BoundNumber& rhs){return *this = operator-(rhs.data);}
	
	template<typename rhsType>
	dataType operator*(const rhsType rhs) {
		if(!rhs || !data)
			return 0;
		if(!range() || rhs==1)
			return data;
		if(rhs>0){
			if(maxValue/data < rhs){
				return maxValue;
			}
			else if(minValue/data > rhs){
				return minValue;
			}
			else 
				return data * rhs;
		}
		if(rhs<0){
			if(maxValue/rhs > data){
				return maxValue;
			}
			else if(minValue/rhs < data){
				return minValue;
			}
			else
				return data * rhs;
		}
		return data;
	}
	template<typename rhsType>
	dataType operator*=(const rhsType rhs){return *this = operator*(rhs);}
	dataType operator*(const BoundNumber& rhs){return *this * rhs.data;}
	dataType operator*=(const BoundNumber& rhs){return *this = operator*(rhs.data);}
	
	template<typename rhsType>
	dataType operator/(const rhsType rhs) {
		if(!rhs || !data)
			return *this * 0;
		if(!range() || rhs == 1)
			return data;
		if(rhs>0){
			if(maxValue * data < rhs){
				return maxValue;
			}
			else if(minValue * data > rhs){
				return minValue;
			}
			else 
				return data/rhs;
		}
		if(rhs<0){
			if(maxValue * rhs > data){
				return maxValue;
			}
			else if(minValue * rhs < data){
				return minValue;
			}
			else 
				return data/rhs;
		}
		return operator*(0);
	}
	template<typename rhsType>
	dataType operator/=(const rhsType rhs){return *this = operator/(rhs);}
	dataType operator/(const BoundNumber& rhs){return *this / rhs.data;}
	dataType operator/=(const BoundNumber& rhs){return *this = operator/(rhs.data);}
	
	template<typename rhsType>
	dataType operator%(rhsType rhs){return data % rhs;}	//is it desired behavior to go out of bounds?
	
	dataType operator++(int){return *this = operator+(1);}
	dataType operator--(int){return *this = operator-(1);}
	
	template<typename convType>
	operator convType(){return (convType)data;}
	operator dataType(){return data;}	//not sure why we also need this one
	
	template<typename compType>
	bool operator==(const compType comparator) const {return data == comparator;}
	template<typename compType>
	bool operator!=(const compType comparator) const {return data != comparator;}
	template<typename compType>
	bool operator!() const {return data == 0;}
	
	template<typename compType>
	bool operator<(const compType comparator) const {return data < comparator;}
	template<typename compType>
	bool operator>(const compType comparator) const {return data > comparator;}
	
	template<typename compType>
	bool operator<=(const compType comparator) const {return data <= comparator;}
	template<typename compType>
	bool operator>=(const compType comparator) const {return data >= comparator;}
	
	//class compatible versions
	//WIP: may need additional testing
	
	bool operator==(const BoundNumber& comparator) const {return data == comparator.data;}
	bool operator!=(const BoundNumber& comparator) const {return data != comparator.data;}
	
	bool operator<(const BoundNumber& comparator) const {return data < comparator.data;}
	bool operator>(const BoundNumber& comparator) const {return data > comparator.data;}
	
	bool operator<=(const BoundNumber& comparator) const {return data <= comparator.data;}
	bool operator>=(const BoundNumber& comparator) const {return data >= comparator.data;}
	
	bool debug(const char* message, bool print){
		bool debugNecessary = false;
		
		if(print){
			std::clog << std::endl;
			std::clog << message;
			std::clog << "\ndata: ";
			std::clog << data;
			std::clog << "\nminValue: ";
			std::clog << minValue;
			std::clog << "\nmaxValue: ";
			std::clog << maxValue;
			std::clog << "\nrange: ";
			std::clog << range();
			std::clog << std::endl;
		}
		
		if(minValue == maxValue){
			if(print){
				std::clog << "\nWARNING: ";
				std::clog << &minValue;
				std::clog << " BoundNumber min and max values are the same.";
				std::clog << std::endl;
			}
		}		
		else if(minValue > maxValue){
			debugNecessary = true;
			
			if(print){
				std::clog << "\nDebug: Swapping bounds...";
				std::clog << std::endl;
			}
			dataType dummy = minValue;
			minValue = maxValue;
			maxValue = dummy;
		}
		
		if(data < minValue){
			debugNecessary = true;
			
			if(print){
				std::clog << "\nDebug: ";
				std::clog << &data;
				std::clog << " - Value too low; adjusting...";
				std::clog << std::endl;
			}
			data = minValue;
		}
		
		if(data > maxValue){
			debugNecessary = true;
			
			if(print){
				std::clog << "\nDebug: ";
				std::clog << &data;
				std::clog << " - Value too high; adjusting...";
				std::clog << std::endl;
			}
			data = maxValue;
		}
		
		if(debugNecessary){
			static bool firstrun = true;	//prevents infinite loop
			debugNecessary = false;
			
			if(firstrun){
				firstrun = false;
				debugNecessary = debug("Automatic corrections made. New information:",print);
				firstrun = true;
			}
		}
		return debugNecessary;
	}
	bool debug(){return debug("Debug information:", true);}
	bool debug(const bool print){return debug("", print);}
	bool debug(const char* message){return debug(message, true);}
	
};

//must be kept with struct/class
template<typename inType>
std::ostream& operator<<(std::ostream& stream, const BoundNumber<inType>& input){
	stream << input.data;
	return stream;
}
template<typename inType>
std::istream& operator>>(std::istream& stream, BoundNumber<inType>& input){
	stream >> input.data;
	
	input.debug(false);
	return stream;
}

//number that will cycle within a range of values.
//set using {data,min,max}. can use normal assignment operators, as well, but you must declare the type on initialization.
template<typename dataType>
class CycleNumber {
public:
	
	dataType data;
	dataType minValue;
	dataType maxValue;
	
	const dataType range() {return maxValue-minValue+1;}	//here, the range counts EVERY number, so 0-10 is a range of 11
	const dataType headRoom() {return maxValue-data;}	//distance from data to maxValue
	const dataType legRoom() {return data-minValue;}	//distance from data to maxValue

	CycleNumber(){}
	
	CycleNumber(const dataType input):
		data(input),
		minValue(input),
		maxValue(input)
	{}
	
	CycleNumber(const CycleNumber& input):
		data(input.data),
		minValue(input.minValue),
		maxValue(input.maxValue)
	{}
	
	CycleNumber(const CycleNumber&& input):
		data(input.data),
		minValue(input.minValue),
		maxValue(input.maxValue)
	{}
	
	CycleNumber(dataType input1, dataType input2, dataType input3):
		data(input1),
		minValue(input2),
		maxValue(input3)
	{debug(false);}
	
	///deprecating- they are forced to convert anyway, so there is no reason to provide explicit constructors
	//template<typename dataType2>
	//CycleNumber(dataType2 input1, dataType input2, dataType input3):
	//	data((dataType)(input1)),
	//	minValue(input2),
	//	maxValue(input3)
	//{debug(false);}
	//
	////maxValue is MOST LIKELY to be the largest number, so in a 3-type input,
	//// it is favored as the main type.
	//template<typename dataType2, typename dataType3>
	//CycleNumber(dataType2 input1, dataType3 input2, dataType input3):
	//	data((dataType)(input1)),
	//	minValue((dataType)(input2)),
	//	maxValue(input3)
	//{debug(false);}
	
	dataType operator=(const dataType input){
		data = input;
		debug(false);
		return data;
	}
	
	//WIP: needs final testing.
	
	template<typename rhsType>
	dataType operator+(rhsType rhs){	//find a way to make const?
		if(!range()){
			debug(false);
			return data;
		}
		if(rhs > 0){
			if(rhs >= range())
				rhs -= range()*(rhs/range());
			if(headRoom() < rhs)
				//return (data + rhs) - (range()+1); //original formula, had an error with unsigned values.
				return minValue + (rhs - headRoom()-1);	//needs final testing
			return data + rhs;
		}
		else if(rhs < 0)
			return operator-(-rhs);
		return data;
	}
	template<typename rhsType>
	dataType operator+=(rhsType rhs){return *this = operator+(rhs);}
	dataType operator+(CycleNumber& rhs){return operator+(rhs.data);}
	dataType operator+=(CycleNumber& rhs){return *this = operator+(rhs.data);}
	
	template<typename rhsType>
	dataType operator-(rhsType rhs){	//refactor: find a way to make const?
		if(!range()){
			debug(false);
			return minValue;
		}
		if(rhs>0){
			if(rhs >= range())
				rhs -= range()*(rhs/range());
			if(legRoom() < rhs){
				//return (data - input) + (range()+1);	//wip: possible unsigned value error, see operator+()
				return maxValue - (rhs - legRoom()-1);	// possible solution. needs testing to confirm fix.
			}
			else
				return data - rhs;
		}
		else if(rhs<0){
			return operator+(-rhs);
		}
		return data;
	}
	template<typename rhsType>
	dataType operator-=(rhsType rhs){return *this = operator-(rhs);}
	dataType operator-(CycleNumber& rhs){return operator-(rhs.data);}
	dataType operator-=(CycleNumber& rhs){return *this = operator-(rhs.data);}
	
	template<typename rhsType>
	dataType operator*(const rhsType rhs){
		if(!rhs || !data)
			return 0;
		if(!range() || rhs==1)
			return data;
		
		//wip: had to compromise between compute-intensive formula and loops, or using extra memory for local variables.
		// need to come up with a more efficient formula to eliminate the loops altogether.
		int i=0;
		for(; data*rhs + range()*i > maxValue; i--){}
		for(; data*rhs + range()*i < minValue; i++){}
		
		return rhs*data + i*range();
	}
	template<typename rhsType>
	dataType operator*=(const rhsType rhs){return *this = operator*(rhs);}
	dataType operator*(const CycleNumber& rhs){return operator*(rhs.data);}
	dataType operator*=(const CycleNumber& rhs){return *this = operator*(rhs.data);}
	
	//wip: cannot decide if it should divide based on range, instead of absolute value. maybe make separate member function?
	//uses standard division, and applies cycling rules.
	// ex.: 15, 10, 20; 15/2 would be 7.5, which would cycle back around to 18.5
	template<typename rhsType>
	dataType operator/(const rhsType rhs){
		if(!rhs || !data)
			return 0;
		if(!range() || rhs==1)
			return data;
		if(rhs < 1 && rhs > -1)
			return *this * (1/rhs);
		
		//wip: need a better formula to replace this
		int i=0;
		for(; data/rhs + range()*i > maxValue; i--){}
		for(; data/rhs + range()*i < minValue; i++){}
		
		return data/rhs + i*range();
	}
	template<typename rhsType>
	dataType operator/=(const rhsType rhs){return *this = operator/(rhs);}
	dataType operator/(const CycleNumber& rhs){return operator/(rhs.data);}
	dataType operator/=(const CycleNumber& rhs){return *this = operator/(rhs.data);}
	
	
	dataType operator++(int){return *this = operator+(1);}
	dataType operator--(int){return *this = operator-(1);}
	
	dataType operator%(dataType input){return data % input;}	//cannot decide if this is desired behavior or not
	
	template<typename convType>
	operator convType(){return (convType)(data);}
	operator dataType(){return data;}
	
	
	//wip: test performance vs dataType versions
	template<typename compType>
	bool operator==(const compType comparator) const {return data == comparator;}
	template<typename compType>
	bool operator!=(const compType comparator) const {return data != comparator;}
	bool operator!() const {return data == 0;}
	
	template<typename compType>
	bool operator<(const compType comparator) const {return data < comparator;}
	template<typename compType>
	bool operator>(const compType comparator) const {return data > comparator;}
	
	template<typename compType>
	bool operator<=(const compType comparator) const {return data <= comparator;}
	template<typename compType>
	bool operator>=(const compType comparator) const {return data >= comparator;}
	
	
	bool operator==(const CycleNumber& comparator) const {return data == comparator.data;}
	bool operator!=(const CycleNumber& comparator) const {return data != comparator.data;}
	
	bool operator<(const CycleNumber& comparator) const {return data < comparator.data;}
	bool operator>(const CycleNumber& comparator) const {return data > comparator.data;}
	
	bool operator<=(const CycleNumber& comparator) const {return data <= comparator.data;}
	bool operator>=(const CycleNumber& comparator) const {return data >= comparator.data;}
	
	
	//wip: update correction formulas from operators
	// also make stream-selectable version
	bool debug(const char * message, bool print){
		bool debugNecessary = false;
		static dataType dummy;
		
		if(print){
			std::clog << std::endl;
			std::clog << message;
			std::clog << "\ndata address: ";
			std::clog << &data;
			std::clog << "\ndata: ";
			std::clog << +data;
			std::clog << "\nminValue: ";
			std::clog << +minValue;
			std::clog << "\nmaxValue: ";
			std::clog << +maxValue;
			std::clog << "\nRange: ";
			std::clog << range();
			std::clog << std::endl;
		}
		if(minValue == maxValue){
			if(print){
				std::clog << "\nWARNING: ";
				std::clog << &minValue;
				std::clog << " - CycleNumber min and max values are the same.";
				std::clog << std::endl;
			}
		}else if(minValue > maxValue){
			debugNecessary = true;
			
			if(print){
				std::clog << "\nDebug: Swapping bounds...";
				std::clog << std::endl;
			}
			
			bitSwap(maxValue,minValue);
		}
		//if(data < minValue){
		//	debugNecessary = true;
		//	if(print){
		//		std::clog << "\nDebug: ";
		//		std::clog << &data;
		//		std::clog << " - Value too low; adjusting...";
		//		std::clog << std::endl;
		//	}
		//	data += (range()*((minValue-data)/range())+1);
		//}
		if(data > maxValue || data < minValue){
			debugNecessary = true;
			if(print){
				std::clog << "\nDebug: ";
				std::clog << &data;
				std::clog << " - Value out of bounds; adjusting...";
				std::clog << std::endl;
			}
			dummy = data;
			data = 0;
			data = operator+(dummy);
			
			//data -= (range()*((data-maxValue)/range())+1);
		}
		if(debugNecessary){
			static bool firstrun = true;
			debugNecessary = false;
			
			if(firstrun){
				firstrun = false;	//stops this from entering an infinite loop
				debugNecessary = debug("Automatic corrections made. New information:",print);
				firstrun = true;
			}
		}
		return debugNecessary;
	}
	bool debug(){return debug("Debug information:", true);}
	bool debug(const bool print){return debug("",print);}
	bool debug(const char* message){return debug(message, true);}
};
//must be kept with struct/class
template<typename dataType>
std::ostream& operator<<(std::ostream& stream, CycleNumber<dataType>& input){
	stream << input.data;
	return stream;
}
template<typename dataType>
std::istream& operator>>(std::istream& stream, CycleNumber<dataType>& input){
	stream >> input.data;
	input.debug(false);
	return stream;
}
