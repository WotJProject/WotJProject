/*
This is the main library.

link this file to access all of wotjcore
*/
#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <thread>


//platform library, change as needed. must be defined first, as other libraries depend on it.
#include "wotjLinGCC.cpp"	//linux
//#include ".wotjWinMSVC.cpp"	//windows (WIP)

#define lbr "\n"
//const std::ostream& lbr(std::ostream& stream){return stream << "\n";}	// ~15% slower than the macro version. kept for reference.
#define endLine std::cout << std::endl

#define wip std::cout << "\n\nOops! Not finished!" << std::endl; pause
#define zzz throw std::cout << "\nThis is where you left off!\n"	//the compiler error is a feature, not a bug


/*
im going to leave this here to remind myself not to trust code just because it works. also that i cant do math.

#define ptr2D(xAxis,yAxis,xOffset,yOffset) *(xAxis + xOffset*yAxis + yOffset)
#define ptr3D(xAxis,yAxis,zAxis,xOffset,yOffset,zOffset) *(xAxis + xOffset*yAxis + yOffset*zAxis + zOffset)

also, consider making a bounds-checked version using ternaries?
	-it doesnt have to be readable if we never have to modify it... right?
*/

//actual fully-tested versions. see pseudocode below.
#define ptr2D(address,width,x,y) *((address)+(x) + ((width)*(y)))	//accesses linear array as 2D array/matrix
#define ptr3D(address,width,area,x,y,z) *((address)+(x) + ((width)*(y)) + ((area)*(z)))	//area is number of rows*columns. name is negotiable.
#define ptr4D(address,width,area,volume,x,y,z,t) *((address)+(x) + ((width)*(y)) + ((area)*(z)) + ((volume)*(t)))	//volume is area*layers, area is rows*columns.
/*
PSEUDOCODE:
//these are measured in indices, not bytes! actual byte size/offset is determined by the address type!
int width;
int height;
int x;
int y;

//these SHOULD ALL be the same-size type, unless you like segfaults!
char matrix[width * height];
char putData;
char getData;

//changing target data
ptr2D(matrix, width, x, y) = putData;

//returning target data
getData = ptr2D(matrix, width, x, y);

*/



#include "wotjOutput.cpp"
#include "wotjMath.cpp"
//#include ".wotjStructures.cpp"	//needs a rewrite
//#include ".wotjFiles.cpp"
#include "wotjTime.cpp"
//#include ".wotjLibGL.cpp"	//wip pending Vulkan development
#include "wotjFixedString.cpp"
#include "wotjNumbers.cpp"
//#include "wotjWIP.cpp"




