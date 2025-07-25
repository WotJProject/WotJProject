#pragma once
#include "..wotjLib.cpp"




#define chronoduration std::chrono::duration<sfloat64>	//std::chrono duration variable, the result of comparing two time_points
#define chronopoint std::chrono::_V2::steady_clock::time_point	//type macro for chrono variables.
static sfloat64 globalTimeStamp = 0.0;	//global variable for holding chrono data
#define chronoNow std::chrono::_V2::steady_clock::now()	//grabs the current point in time for std::chrono operations.




//standard construct-destruct timer. transfers lifetime to globalTimeStamp upon destruction. note: use heap-allocated variable instead?
//TO USE: declare it using  timerlifetime NAME;  it will automatically store the time when the object leaves scope
struct timerlifetime{
    chronopoint timerBegin;
    chronopoint timerEnd;
    chronoduration timerDuration;
    timerlifetime():
		timerBegin(chronoNow),
		timerEnd(timerBegin),
		timerDuration(0.0)
	{}
    ~timerlifetime(){
        timerEnd = chronoNow;
        chronoduration timerDuration = timerEnd - timerBegin;
        globalTimeStamp = timerDuration.count();
    }
};

//standard timer class. starts timer when constructed, no need to call it manually except to reset it.
class timerbasic{
	// private:
	public:
	chronopoint start;	//starting point of the timer
	chronopoint end;	//when the timer is stopped
	chronoduration duration;	//useless storage container for time operations
	sfloat64 time;	//Total time converted to an actual useful format
	// bool startTimer;	//controls pausing/starting of the timer
	// bool stopTimer;		//stops running timer functions
	// std::thread timerThread;	//thread for running timers in the background. WIP because threads within functions are an unfunny joke

	//default constructor and initializer list
	timerbasic() : 
		end(chronoNow),
		duration(end-end),
		time(0.0),
		// startTimer(1),
		// stopTimer(0),
		start(chronoNow)
	{}

	//returns current timer value
	sfloat64 getTime(){
		end = chronoNow;
		duration = end - start;
		time = duration.count();
		return time;
	}

	//resets timer completely
	void resetTime(){
		time = 0.0;
		end = chronoNow;
		duration = end - end;
		start = chronoNow;
		return;
	}
	/*-------------------------------------------------------WIP ongoing timer functions-------------------------------------------------------*/

	//continuously running timer. starts automatically unless otherwise flagged with a bool
	//to run in the background, youll have to use std::thread yourself
	// chronopoint timerOngoing(bool autoStart){
	// 	start = chronoNow;
	// 	startTimer = autoStart;
	// 	while(!stopTimer)
	// 		while(startTimer){
	// 			end = chronoNow;
	// 		}
	// 	return end;
	// }
	//continuously running timer. starts automatically unless otherwise flagged with a bool.
	//to run in the background, youll have to use std::thread yourself
	// chronopoint timerOngoing(){
	// 	return timerOngoing(true);
	// }


	// //wrapper for running timerOngoing(). can be run with other things, but MUST return a chronopoint type.
	// void run(chronopoint *(&inputFunction)){
	// 	static std::thread timerThread(inputFunction);
	// 	stopTimer = false;
	// 	startTimer = false;
	// }

	// void stop(chronopoint *(&inputFunction)){
	// 	stopTimer = true;
	// 	timerbasic::timerThread.join();
	// }

};