#pragma once
#include "WMM.h"

#include <fstream>
#include <iostream>
#include <filesystem>

uint32 mapSizeX = 0; //maximum horizontal size of map
uint32 mapSizeY = 0; //maximum vertical size of map

Tile* map = nullptr;	//primary map buffer.

std::string mapName = "Default Map";

//saves the map to file
//DOD refactor: separate into menu (user input name) version and automatic (known name) version; the former will call the latter, but the latter can be standalone.
bool saveMap(const char* name, Tile*& input, const uint32 sizeX, const uint32 sizeY, const bool print){
	if(!input){
		out(std::clog,lbrFormat,"ERROR: No map found or invalid address.");
		return false;
	}
	
	//making these static causes first-run bug
	std::ofstream saveFile;
	///FixedString<250> saveName;	//causes function to crash on return. need to find out why.
	std::string saveNameStr;
	
	//i guess std::string doesnt support being passed a nullptr
	if(name)
		saveNameStr = name;
		//saveName = name;
	else{
		clearScreen;
		out(lbrFormat, "Please include extension and relative directory if not default.",\
				"Please save to existing folders, to avoid potential issues. Press TAB to cancel.",\
				"\nMap Name: ");
		//std::cin >> saveName;
		std::cin >> saveNameStr;
		std::cin.ignore();
		
		//sanitizePlainText(saveName.data,saveName.readLength(),true);
		sanitizePlainText(saveNameStr,saveNameStr.size(),true);
		
		//if(saveName[0] == tabKey || !saveName[0])
		if(saveNameStr[0] == tabKey || !saveNameStr[0])
			return false;
	}
	
	sint32 strIndex;	//saves place in string.
	//for (strIndex=0; saveName[strIndex]; strIndex++){
	for (strIndex=0; saveNameStr[strIndex]; strIndex++){
		
		//explicit directory
		//if (saveName[strIndex] == filePathSymbol || (saveName[strIndex] == '.' && (!saveName[strIndex+4] || !saveName[strIndex+5]))){ //checks for punctuation that may indicate a file path
		if (saveNameStr[strIndex] == filePathSymbol || (saveNameStr[strIndex] == '.' && (!saveNameStr[strIndex+4] || !saveNameStr[strIndex+5]))){ //checks for punctuation that may indicate a file path
			if(print)
				//out(lbrFormat,"Explicit directory detected.","Readying file:",saveName);
				out(lbrFormat,"Explicit directory detected.","Readying file:",saveNameStr);
			
			//if(std::filesystem::exists(saveName.data)){
			if(std::filesystem::exists(saveNameStr)){
				out(lbrFormat,"WARNING: File already exists. Continue?","1. Yes","Any other key: No");
				keyt k = getKey(true);
				if(k != oneKey)
					return false;
			}else{
				std::string path = saveNameStr;
				
				sizet i=saveNameStr.size();
				for(; path[i] != '/'; i--){}
				
				if(i){
					path[i] = '\0';
					std::filesystem::create_directory(path);
				}
			}
			//saveFile.open(saveName.data, std::ios::out | std::ios::binary);
			saveFile.open(saveNameStr, std::ios::out | std::ios::binary);
			
			break;
		}
	}
	
	//implicit directory
	//if (strIndex == saveName.getLength()){
	if (strIndex == saveNameStr.size()){
		
		//must be done as separate operations, because otherwise it does pointer arithmetic instead of concatenation.
		//refactor: might be a better workaround out there somewhere, though the performance impact is unnoticeable.
		std::string mapPath = "..";
		mapPath += filePathSymbol;
		mapPath += "maps";
		
		std::filesystem::create_directory(mapPath);
		
		mapPath += filePathSymbol; 
		//mapPath += saveName.data;
		mapPath += saveNameStr;
		mapPath += ".wmap";
		if(print)
			out(std::cout,lineBreakFormat,"Implicit directory detected; assuming default directory.","Readying file:",mapPath);
		
		if(std::filesystem::exists(mapPath)){
			out(lbrFormat,"WARNING: File already exists. Continue?","1. Yes","Any other key: No");
			keyt k = getKey(true);
			if(k != oneKey)
				return false;
		}
		saveFile.open(mapPath, std::ios::out | std::ios::binary);
	}
	
	//file must be opened by this point!
	
	if(!saveFile || saveFile.fail() || saveFile.bad()){
		out(std::clog,lbrFormat,"\nERROR: File could not be opened. The name/directory may be incorrect,",\
				"or I may not have permission to write to this location.",\
				"\nPress Enter to try again, or any other key to cancel Save operation.");
		saveFile.close();
		
		if(getKey(false) == enterKey)
			return saveMap(name,input,sizeX,sizeY,print);
		else
			return false;
	}
	saveFile.seekp(std::ios::beg);
	
	if(print)
		out(lineBreakFormat,"Saving to file...");
	
	saveFile << sizeX << '\n';
	saveFile << sizeY << '\n';
	
	std::noskipws(saveFile);
	
	for(uint32 i = 0; i<sizeX*sizeY; i++){
		saveFile << input[i];
		if(saveFile.eof() || saveFile.fail() || saveFile.bad())
			out(std::clog,pairFormat,"WARNING:",&input," Unexpected interruption in file writing operation at index:",i);
	}
	
	saveFile.close();
	//mapName = saveName.data;
	mapName = saveNameStr;
	
	//if(print)
	//	out(lineBreakFormat,"Map Saved. Returning to menu.");
	//endLine;
	
	//std::cin.ignore();
		out(lineBreakFormat,"Map Saved. Returning to menu.");
	endLine;
	return true;	///wip;	//crashes on return when using FixedString for saveName, dont know why
}
bool saveMap(Tile*& input, const uint32 sizeX, const uint32 sizeY){return saveMap(nullptr, input, sizeX, sizeY, false);}
bool saveMap(Tile*& input, const uint32 sizeX, const uint32 sizeY, const bool print){return saveMap(nullptr, input, sizeX, sizeY, print);}
bool saveMap(const char* name){return saveMap(name,map,mapSizeX,mapSizeY,false);}	//save primary map as specified name

//save clipboard data to file
bool clipboardSave(){
	std::filesystem::create_directory("../maps/history");
	
	std::ofstream saveFile;
	saveFile.open("../maps/history/clipboard.wmap", std::ios::out | std::ios::binary);
	
	if(!saveFile || saveFile.fail() || saveFile.bad() || !std::filesystem::exists("../maps/history/clipboard.wmap")){
		out(std::cerr,lbrFormat,"ERROR: Failed to save to clipboard file!");
		return false;
	}
	saveFile.seekp(std::ios::beg);
	
	saveFile << stampData.clipWidth << '\n';
	saveFile << stampData.clipHeight << '\n';
	
	std::noskipws(saveFile);
	for(uint32 i = 0; i< stampData.clipWidth * stampData.clipHeight; i++){
		saveFile << stampData.clipboard[i];
		
		if(saveFile.bad() || saveFile.fail() || saveFile.eof())
			out(std::clog,lbrFormat,"WARNING: Clipboard save operation failed at ", i, "in position: ",saveFile.tellp(), "!");
	}
	
	saveFile.close();
	return true;
}

//save undo data to file
bool historySave(){
	std::filesystem::create_directory("../maps/history");
	
	std::ofstream saveFile;
	undoSaveTarget++;
	
	std::string undoPath = "../maps/history/";
	undoPath += std::to_string(undoSaveTarget.data) + ".wmap";
	saveFile.open(undoPath, std::ios::out | std::ios::binary);
	
	if(!saveFile || saveFile.fail() || saveFile.bad() || !std::filesystem::exists(undoPath)){
		out(std::cerr,lbrFormat,"ERROR: Failed to save to undo history file!");
		undoSaveTarget--;
		return false;
	}
	saveFile.seekp(std::ios::beg);
	
	saveFile << mapSizeX << '\n';
	saveFile << mapSizeY << '\n';
	
	std::noskipws(saveFile);
	for(uint32 i = 0; i<mapSizeX*mapSizeY; i++)
		saveFile << map[i];
	
	saveFile.close();
	return true;
}


Tile* loadMap(const char* loadName, Tile*& destination, const bool print){
	std::string inputName;
	std::ifstream mapFile;
	
	if(!loadName){
			
		out(lineBreakFormat,"Please include relative directory and extension if not in ../maps/. Otherwise, just include the name.","Press TAB to cancel.","\nMap name: ");
		std::cin >> inputName;
		
		if(inputName[0] == tabKey || inputName[0] == escKey || !inputName[0])
			return nullptr;
		
	}else{
		inputName = loadName;
	}
	
	static sint32 i;
	
	for (i=0; inputName[i]; i++){   //i needs to keep its scope for implicit directory checking.
		
		//explicit directory
		if (inputName[i] == filePathSymbol || (inputName[i] == '.' && (!inputName[i+4] || !inputName[i+5]))){
			out(std::cout,lbrFormat,"Explicit directory detected.","Readying file:",inputName);
			
			if(!std::filesystem::exists(inputName)){
				out(std::clog,lbrFormat,"ERROR: File not found. Please try again.");
				return loadMap(loadName, destination, print);
			}
			
			mapFile.open(inputName, std::ios::in | std::ios::binary);
			
			break;
		}
	}
	
	//implicit directory
	if (i == inputName.length()){
		
		//must be done as separate operations, because otherwise it does pointer arithmetic instead of concatenation.
		//refactor: might be a better workaround out there somewhere...
		std::string mapPath = "..";
		mapPath += filePathSymbol;
		mapPath += "maps";
		mapPath += filePathSymbol; 
		mapPath += inputName + ".wmap";
		
		out(std::cout,lineBreakFormat,"Implicit directory detected; assuming default directory.","Readying file:",mapPath);
		
		if(!std::filesystem::exists(mapPath)){
			out(std::clog,lbrFormat,"ERROR: File not found. Please try again.");
			return loadMap(loadName, destination, print);
		}
		
		mapFile.open(mapPath, std::ios::in | std::ios::binary);
	}
	
	
	
	if(!mapFile || mapFile.bad() || mapFile.fail()){
		out("\nERROR: ", &inputName ," Could not open file. Please re-check file name or input, and ensure it is not open elsewhere.");
		out("\n input:",inputName);
		
		pause;
		return loadMap(loadName, destination, print);
	}else{
		mapName = inputName;
	}
	
	
	//map file must be opened by this point!
	
	mapFile.unsetf(std::ios_base::skipws);
	mapFile.seekg(std::ios::beg);
	
	uint32 outSizeX = 0;
	uint32 outSizeY = 0;
	
	mapFile >> outSizeX;
	mapFile.seekg(1,std::ios::cur);
	
	mapFile >> outSizeY;
	mapFile.seekg(1,std::ios::cur);
	
	const uint32 newSize = outSizeX * outSizeY;
	
	if(newSize>maxS32 || outSizeX<5 || outSizeY<5){
		out("\nERROR: ", &outSizeX, ": map has bad size! Cancelling load operation.","\nX: ",outSizeX, "\nY: ",outSizeY);
		pause;
		return nullptr;
		
	}else if(destination){
		delete destination;
		destination = nullptr;
	}
	destination = new Tile[newSize];
	
	for(uint32 i=0; i<newSize; i++){
		mapFile >> destination[i];
		if(mapFile.eof() || mapFile.fail() || mapFile.bad())
			out(pairFormat,"WARNING: Map loading stopped unexpectedly!","Debug information:","File position:",mapFile.tellg(),"Map buffer position:",i,"Map Width:",outSizeX,"Map Height:",outSizeY);
	}
	
	mapFile.close();
	
	
#if 0
	zzz;
	static std::ifstream listFile;	//refactor: moving to local static
	//static  std::vector masterList; //storage for list of activators on current map. wip.

	//wip wip wip
	//MasterListMenu: //load map/activators
	
	out(lineBreakFormat,"\nWARNING: Activators are a work-in-progress! Select NO for now!","Load activator Masterlist?","1. Yes (no but later)","2. No","Any other key: Also no.");
	if (getKey(false) == oneKey){
		throw "Hey! I told you NOT to proceed!";
		wip;    //wip
		return nullptr;
		//goto MasterListMenu;
		for (i = 0; inputName[i]; i++){
			if (inputName[i] == '\\' || (inputName[i] == '.' && (!inputName[i+4] || !inputName[i+5]))){
				std::string inputListName;
				
				out(lineBreakFormat,"\nCustom map directory indicated. Please input location of Activator Masterlist:");
				std::cin >> inputListName;
				
				listFile.open((inputListName), std::ios::in);
				break;
			}
		}
		if (!inputName[i])
			listFile.open(("maps\\"+ inputName + "\\" + inputName + ".wlst"), std::ios::in);
			
		if(!listFile){
			out(lineBreakFormat,"ERROR: Could not open file. Please re-check file name or input.");
			pause;
			//goto MasterListMenu;
		}
		listFile.close();
	}
#endif
	
	mapSizeX = outSizeX;
	mapSizeY = outSizeY;
	
	x = mapSizeX >>1;
	y = mapSizeY >>1;
	
	undoSaveTarget = undoSaveTarget.maxValue;	//the saveMap call below increments it to 0
	availableUndos = 0;
	availableRedos = 0;
	historySave();
	
	if(!loadName){	//for some reason the input stream passes extra information in this situation (i suspect that streams may be dumb), so one ignore isnt enough
		std::cin.clear();
		std::cin.ignore();
	}
	
	clearScreen;
	
	
	
	return destination;
}
Tile* loadMap(){return loadMap(nullptr,map,true);}
Tile* loadMap(const char* name, Tile*& dest){return loadMap(name, dest, false);}
Tile* loadMap(const char* name, const bool print){return loadMap(name, map, print);}
Tile* loadMap(const char* name){return loadMap(name, map, false);}

//loads clipboard data from file into stampData buffer
bool clipboardLoad(){
	std::ifstream mapFile;
	//clearScreen;	//why did this fix a segfault?????????
	
	const char* copyPath = "../maps/history/clipboard.wmap";
	if(!std::filesystem::exists(copyPath))
		return false;
	
	mapFile.open(copyPath, std::ios::out | std::ios::binary);
	if(!std::filesystem::exists(copyPath) || mapFile.bad() || mapFile.fail() || !mapFile){
		out(std::cerr,lbrFormat,"ERROR: Clipboard file could not be loaded!");
		stampData.stampMode = 0;
		
		mapFile.close();
		return false;
	}else{
		stampData.stampMode = 3;
	}
	
	mapFile.unsetf(std::ios_base::skipws);
	mapFile.seekg(std::ios::beg);
	
	mapFile >> stampData.clipWidth;
	mapFile.seekg(1,std::ios::cur);
	
	mapFile >> stampData.clipHeight;
	mapFile.seekg(1,std::ios::cur);
	
	const uint32 newSize = stampData.clipWidth * stampData.clipHeight;
	
	if(newSize > maxS32){
		out(std::clog,noFormat,"\nERROR: ", &mapFile, ": clipboard has bad size! Cancelling load operation and clearing data.","\nX: ",stampData.clipWidth, "\nY: ",stampData.clipHeight);
		stampData.stampMode = 0;
		stampData.clipHeight = 0;
		stampData.clipWidth = 0;
		
		if(stampData.clipboard != nullptr){
			delete stampData.clipboard;
			stampData.clipboard = nullptr;
		}
		mapFile.close();
		return false;
	} 
	
	if(stampData.clipboard != nullptr){
		delete stampData.clipboard;
		stampData.clipboard = nullptr;
	}
	stampData.clipboard = new Tile[newSize];
	
	for(uint32 i = 0; i < newSize; i++){
		mapFile >> stampData.clipboard[i];
		
		if(mapFile.eof() || mapFile.fail() || mapFile.bad())
			out(std::clog,pairFormat,"WARNING: Map loading stopped unexpectedly!","Debug information:",
				"File position:",mapFile.tellg(),
				"Map buffer position:",i,
				"Map Width:",stampData.clipWidth,
				"Map Height:",stampData.clipHeight);
	}
	
	//wip: needs final testing
	stampData.top = y - (stampData.clipHeight>>1);
	stampData.left = x - (stampData.clipWidth>>1);
	stampData.right = x + (stampData.clipWidth>>1);
	stampData.bottom = y + (stampData.clipHeight>>1);
	
	mapFile.close();
	return true;
}

//loads undo data to primary map
bool historyLoad(){
	std::ifstream mapFile;
	
	std::string undoPath = "../maps/history/";
	undoPath += std::to_string(undoSaveTarget.data);
	undoPath += ".wmap";
	
	mapFile.open(undoPath, std::ios::out | std::ios::binary);
	if(!mapFile || mapFile.bad() || mapFile.fail() || !std::filesystem::exists(undoPath)){
		out(std::clog,lbrFormat,"ERROR: Undo history could not be loaded!");
		return false;
	}
	
	mapFile.unsetf(std::ios_base::skipws);
	mapFile.seekg(std::ios::beg);
	
	uint32 outSizeX = 0;
	uint32 outSizeY = 0;
	
	mapFile >> outSizeX;
	mapFile.seekg(1,std::ios::cur);
	
	mapFile >> outSizeY;
	mapFile.seekg(1,std::ios::cur);
	
	const uint32 newSize = outSizeX*outSizeY;
	
	if(newSize>maxS32 || outSizeX<5 || outSizeY<5){
		out(std::clog,noFormat,"\nERROR: ", &mapFile, ": map has bad size! Cancelling load operation.","\nX: ",outSizeX, "\nY: ",outSizeY);
		
		mapFile.close();
		return false;
	}
	
	if(map){
		delete map;
		map = nullptr;
	}
	map = new Tile[newSize];
	
	mapSizeX = outSizeX;
	mapSizeY = outSizeY;
	
	for(uint32 i = 0; i < newSize; i++){
		mapFile >> map[i];
		if(mapFile.eof() || mapFile.fail() || mapFile.bad()){
			out(std::cerr,pairFormat,"WARNING: History loading stopped unexpectedly!","Debug information:",
				"Map path:",undoPath,
				"File position:",mapFile.tellg(),
				"Map buffer position:",i,
				"Map Width:",mapSizeX,
				"Map Height:",mapSizeY);
			break;
		}
	
	}
	
	mapFile.close();
	return true;
}

Tile* newMap(){
	Tile inputTile;
	
	uint32 sizeX = 0;
	uint32 sizeY = 0;
	
	clearScreen;
	out("\nEnter desired map bounds without commas.\n",\
	"\nFor reference, a square map of only 1000x1000 units will occupy around ", (sizeof(Tile)), "MB, and",\
	"\nlarger maps may take a few moments to generate, load, or save.",\
	"\n\nMap Horizontal size: ");
	
	GetNewMapSize:
	
	std::cin >> sizeX;
	if(sizeX < 5){
		out(lineBreakFormat,"ERROR: Invalid input. Please try again.\n");
		
		pause;
		goto GetNewMapSize;
	}
	
	out(lbrFormat,"\nMap vertical size: ");
	
	std::cin >> sizeY;
	if(sizeY < 5 || maxU64/sizeY < sizeX){
		out(lineBreakFormat,"ERROR: Invalid input. Please try again.\n");
		
		pause;
		goto GetNewMapSize;
	}
		
	if(map){
		delete map;
		map = nullptr;
	}
	
	mapName = "Unsaved Map";
	mapSizeX = sizeX;
	mapSizeY = sizeY;
	
	out(lineBreakFormat,"Initializing map...");
	map = new Tile[mapSizeX*mapSizeY];
	
	for(uint32 i = 0; i < mapSizeX*mapSizeY; i++)
		map[i] = stdtiles::empty;
	
	bool exitMenu = false;
	while(!exitMenu){
		clearScreen;
		out(lineBreakFormat,"Map initialization finished.","mapSizeX:",mapSizeX,"mapSizeY:",mapSizeY,\
		"\nSelect base texture:",\
		"1.Solid Black",\
		"2.Solid White",\
		"3.Light gradient",\
		"4.Medium gradient",\
		"5.Dark gradient",\
		"6.Water",\
		"7.Use tile from Paint Mode: ",\
		"\nTAB - Return to Main Menu (new map was created, but will be empty by default)");
		
		//print tile from paint mode for option 7
		terminalMoveCursor(15,29);
		out(tileSelected);
		terminalMoveCursor(0,0);
		terminalHideCursor;
		
		switch(getKey(false)){
			default:
				continue;
			case tabKey:{
				x = mapSizeX >>1;
				y = mapSizeY >>1;
				historySave();
				
				return map;
			}
			case oneKey:{
				inputTile = stdtiles::empty;
				break;
			}
			case twoKey:{
				inputTile = stdtiles::snow;
				break;
			}
			case threeKey:{
				inputTile = "▓";
				break;
			}
			case fourKey:{
				inputTile = "▒";
				break;
			}
			case fiveKey:{
				inputTile = "░";
				break;
			}
			case sixKey:{
				inputTile = "≋";
				break;
			}
			case sevenKey:{				
				inputTile = tileSelected;
				break;
			}
		}
		exitMenu = true;
	}
	for(uint32 i = 0; i<mapSizeX*mapSizeY; i++)
		map[i] = inputTile;
	
	x = mapSizeX >>1;
	y = mapSizeY >>1;
	
	clearScreen;
	historySave();
	return map;
}

//can be passed in canvasEdit() to bypass menu
enum canvasAutorun : keyt {
	none = 0,
	canvasFlip = oneKey,
	canvasRotate = twoKey,
	canvasResize = threeKey
};

//wip: add paintmode controls for some modes
//for high-level manipulation of the map buffer. NOT passing in a value for canvasAutorun will display the menu.
bool canvasEdit(Tile*& inputMap, canvasAutorun inputCommand, keyt inputValue){
	clearScreen;
	
	if(!map){
		out(lbrFormat,"ERROR: No map loaded!","Load a map, or create a new one.");
		pause;
		return false;
	}
	
	keyt k = inputCommand;
	Tile* dummy = nullptr;	//temporary holding for copied canvas data
	
	while(k != tabKey){
		while(k != tabKey && k != threeKey && k != twoKey && k != oneKey){
			clearScreen;
			out(lbrFormat,"CANVAS EDITING MENU:","1. Flip Canvas","2. Rotate Canvas","3. Resize Canvas","\nTAB - Return to Main Menu");
			getKey(false,k);
		}
		
		switch(k){
			default:
				continue;
			case oneKey:{	//flip
				if(inputCommand)
					k = inputValue;
				else
					k = 0;
				
				switch(k){
					default:{
						while(k != oneKey && k != twoKey && k != tabKey){
							endLine;
							out(lbrFormat,"Flip Canvas:","1. Vertically (top-bottom switch)","2. Horizontally (left-right switch)", "\nTAB - Previous menu");
							getKey(false,k);
							clearScreen;
						}
						inputValue = k;
						inputCommand = canvasFlip;
						k = canvasFlip;
						continue;
						
					}
					case oneKey:{	//vertical 
						dummy = new Tile[mapSizeX * mapSizeY];
						
						for(uint32 iy=0; iy<mapSizeY; iy++)
						for(uint32 ix=0; ix<mapSizeX; ix++)
							ptr2D(dummy,mapSizeX,ix,iy) = ptr2D(map,mapSizeX,ix,(mapSizeY-1-iy));
						
						break;
					}
					case twoKey:{	//horizontal flip
						dummy = new Tile[mapSizeX * mapSizeY];
						
						for(uint32 iy = 0; iy<mapSizeY; iy++)
						for(uint32 ix = 0; ix<mapSizeX; ix++)
							ptr2D(dummy,mapSizeX,ix,iy) = ptr2D(map,mapSizeX,(mapSizeX-1-ix),iy);
						
						break;
					}
					case tabKey:{
						inputCommand = none;
						k = 0;
						continue;
					}
				}
				
				if(!inputCommand)
					out(lbr,"Operation completed. Returning...");
				
				delete map;
				map = dummy;
				
				if(historySave()){
					availableUndos++;
					availableRedos = 0;
				}
				return true;
			}
			case twoKey:{	//rotate
				if(inputCommand)
					k = inputValue;
				else
					k = 0;
				
				switch(k){
					default:{
						while(k!=tabKey && k!=oneKey && k!=twoKey && k!=threeKey){
							endLine;
							out(lbrFormat,"Rotate Canvas:","1. Clockwise","2. 180°","3. Counter-Clockwise","\nTAB - Previous menu");
							getKey(false,k);
							clearScreen;
						}
						
						inputValue = k;
						inputCommand = canvasRotate;
						k = canvasRotate;
						continue;
					}
					case oneKey:{	//cw
						dummy = new Tile[mapSizeX * mapSizeY];
						
						if(mapSizeX != mapSizeY)
							bitSwap(mapSizeX,mapSizeY);
						
						for(sizet iy=0; iy<mapSizeY; iy++)
						for(sizet ix=0; ix<mapSizeX; ix++)
							ptr2D(dummy,mapSizeX,ix,iy) = ptr2D(map,mapSizeY,iy,(mapSizeX-ix-1));
						
						break;
					}
					case twoKey:{	//180
						auto size = mapSizeX*mapSizeY;
						dummy = new Tile[size];
						
						for(sizet i=0; i<size; i++){
							dummy[i] = inputMap[size-1-i];
						}
						
						break;
					}
					case threeKey:{	//ccw
						dummy = new Tile[mapSizeX * mapSizeY];
						
						if(mapSizeX != mapSizeY)
							bitSwap(mapSizeX,mapSizeY);
						
						for(sizet iy=0; iy<mapSizeY; iy++)
						for(sizet ix=0; ix<mapSizeX; ix++)
							ptr2D(dummy,mapSizeX,ix,iy) = ptr2D(map,mapSizeY,(mapSizeY-iy-1),ix);
						
						break;
					}
					case tabKey:{
						inputCommand = none;
						k = 0;
						continue;
					}
				}
				
				delete map;
				map = dummy;
				
				if(historySave()){
					availableUndos++;
					availableRedos = 0;
				}
				return true;
			}
			case threeKey:{	//resize
			//note: consider automation for increasing/decreasing canvas by value(1) on command
				
				BoundNumber<uint32> mapNewX = {0,5,maxU32};
				BoundNumber<uint32> mapNewY = {0,5,maxU32};
				
				endLine;
				out(lbrFormat,"Resize Canvas - expands or shrinks from the bottom and right sides.","Input new Width: (input 0 to cancel):");
				std::cin >> mapNewX;
				out(lbrFormat,"Input new Height: (input 0 to cancel):");
				std::cin >> mapNewY;
				
				if(!mapNewX || !mapNewY){
					inputCommand = canvasResize;
					k = canvasResize;
					continue;
				}
				
				
				//best guess for max stable size of a map. actual limit will depend entirely on system ram and performance constraints.
				if(maxS32/mapNewX < mapNewY){
					out(std::clog,noFormat,"\nERROR: ",&mapNewX, " Values too large! Values MUST have a product less than ",maxS32,"!\n X: ",mapNewX," Y: ",mapNewY);
					pause;
					
					inputCommand = canvasResize;
					k = canvasResize;
					continue;
				}
				else{
					mapNewX.minValue = 5;
					mapNewX.debug(false);
					
					mapNewY.minValue = 5;
					mapNewY.debug(false);
					
					out(pairFormat,"Values accepted.","New map values:","New X:",mapNewX,"New Y:",mapNewY);
				}
				
				dummy = new Tile[mapNewX*mapNewY];
				
				const uint32 largestX = (mapNewX<mapSizeX ? mapSizeX : mapNewX.data);
				const uint32 largestY = (mapNewY<mapSizeY ? mapSizeY : mapNewY.data);
				
				const uint32 smallestX = (mapNewX>mapSizeX ? mapSizeX : mapNewX.data);
				const uint32 smallestY = (mapNewY>mapSizeY ? mapSizeY : mapNewY.data);
				
				for(sizet iy=0; iy<largestY; iy++){
					for(sizet ix=0; ix<largestX; ix++){
						if((ix>=smallestX || iy>=smallestY) && (ix<mapNewX && iy<mapNewY))
							ptr2D(dummy,mapNewX,ix,iy) = stdtiles::red;	//filler
						else if (ix<mapNewX && iy<mapNewY)
							ptr2D(dummy,mapNewX,ix,iy) = ptr2D(inputMap,mapSizeX,ix,iy);
					}
				}
				
				out(lbrFormat,"Map data copied. Cleaning up...");
				
				mapSizeX = mapNewX;
				mapSizeY = mapNewY;
				
				delete map;
				map = dummy;
				
				std::cin.ignore();	//why is std::cin so bad at cleaning up after itself?
				
				out(lbrFormat,"Finished. Returning.");
				
				if(historySave()){
					availableUndos++;
					availableRedos = 0;
				}
				return true;
			}
			case tabKey:{
				return true;
			}
		}
	}
	
	return true;
}
bool canvasEdit(){return canvasEdit(map, none, 0);}
bool canvasEdit(Tile*& inputMap){return canvasEdit(inputMap, none, 0);}
