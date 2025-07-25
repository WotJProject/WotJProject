
#include "WMM.h"

#include "maps.cpp"	//contains file i/o
#include "brushes.cpp"
#include "screen.cpp"


const char* Version = "0.99999"; //file version. currently unused.


//all control flow passes through and returns to here.
//ALL continue statements should return to the top while loop.
//refactor: recombine with main()? theres no point in having a function we only call once...
void mainMenu(){
	static bool quit = false;
	
	brushResizeBounds(brushType);	//debug. double-check default values before removing!
	
	while(!quit){
		clearScreen;
		terminalHideCursor;
		
		out(lbrFormat,"           MAIN MENU\n",\
			"\nCurrent Map:",mapName,\
			"\n1. Paint Mode",\
			"2. Select Brush Tileset",\
			"3. Select Brush",\
			"4. Save Map",\
			"5. Load Map",\
			"6. New Map",\
			"7. Canvas Options",\
			"\nTAB = Quit");
		
		switch(getKey(false)){
			default:{
				continue;
			}
			case oneKey:{
			
				if(!map){
					clearScreen;
					out(lbrFormat,"Error: No map loaded. Please import a map or create a new one.");
					pause;
				}else{
					paintControl(true);
				}
				
				break;
			}
			case twoKey:{
				selectBrushTileset();
				break;
			}  
			case threeKey:{
				selectBrushType(0);
				break;
			}
			case fourKey:{	//save map
				if(!map){
					out(lbrFormat,"Error: No map loaded. Please load a map or create a new one.");
					pause;
					continue;
				}
				
				saveMap(map,mapSizeX,mapSizeY,true);
				brushResizeBounds(brushType);
				endLine;
				break;
			}
			case fiveKey:{	//load map
		
				clearScreen;
				if(map){
					out(lbrFormat,"A map is currently loaded.","Continue?","\nENTER - Yes, load new map.","\nTAB - Return to Main Menu");
					switch(getKey(false)){
						default:
						case tabKey:
							continue;
						case carRetKey:
						case enterKey:
							break;
					}
				}
				
				if(!loadMap()){
					out(lbrFormat,"ERROR: Load map failed!");
					pause;
					continue;
				}
				brushResizeBounds(brushType);
				break;
			}
			case sixKey:{	//new map
				if(map){
					out(lbrFormat,"A map is currently loaded.","Continue?","\nENTER - Yes, create new map.","\nTAB - Return to Main Menu");
					switch(getKey(false)){
						default: 
							continue;
						case carRetKey:
						case enterKey:
							break;
						case tabKey:
							continue;
					}
				}
				endLine;
				
				newMap();
				brushResizeBounds(brushType);
				break;
			}
			case sevenKey:{
				canvasEdit(map);
				brushResizeBounds(brushType);
				
				break;
			}
			case tabKey:{
				out(lbrFormat,"\nPress TAB again to quit, or any other button to cancel.");
				if(getKey(false) == tabKey){
					quit = true;
					continue;
				}else{
					break;
				}
			}
		}
	}
	return;
}

//checks for residual undo history and offers to load the latest one
//returns false to immediately quit
bool recoveryCheck(){
	
	//NOTE: may have platform-specific considerations.
	struct stat currentFile;
	struct stat latestFile = {0};
	std::filesystem::path path;
	std::filesystem::path latestPath;
	
	uint8 lastUndos = 0;	//total number of valid undo files found- sets availableUndos
	uint8 lastSaved = 0;
	
	uint8 recoveryIndex = 0;
	for(; recoveryIndex<=undoLevels; recoveryIndex++){
			path = "../maps/history/";
			path += std::to_string(recoveryIndex);
			path += ".wmap";
			
		if(std::filesystem::exists(path)){
			stat(path.c_str(),&currentFile);
			lastUndos++;
			
			if(currentFile.st_mtime >= latestFile.st_mtime){
				latestFile = currentFile;
				latestPath = path;
				
				lastSaved = recoveryIndex;
			}
		}
	}
	
	if(latestFile.st_mtime != 0){
		clearScreen;
		out(lbrFormat,"Uh-oh! The program may not have exited properly last time.","Would you like to continue where you left off?",
			" 1. Yes, attempt to load last action state from undo history.",
			" 2. No, ignore and restart from scratch.",
			"\nTAB - Exit program.\n");
		
		keyt k = 0;
		while(k!=oneKey && k!=twoKey && k!=tabKey)
			getKey(true,k);
		
		if(k == tabKey){
			return false;
		}
		else if(k == oneKey){
			if(!loadMap(latestPath.c_str(),true)){
				out(lbrFormat,"Recovery load operation failed, will now re-initialize from scratch.");
				pause;
				initScreen(true);
				return true;
			}
			
			availableUndos = lastUndos;
			undoSaveTarget = lastSaved;
			mapName = "Unsaved Map";
		}
	}
	
	return true;
}

int main(){
	pause;	//debug
	
	if(!recoveryCheck())
		return 1;
	
	initScreen(false);
	
	mainMenu();	//refactor: not sure if i should just inline it or leave it abstracted like this.
	
	//cleanup
	for(uint8 i=0; i<=undoLevels; i++){
		static std::filesystem::path removePath;
			removePath = "../maps/history/";
			removePath += std::to_string(i);
			removePath += ".wmap";
		
		std::filesystem::remove(removePath);
		
		if(std::filesystem::exists(removePath))
			out(std::clog,noFormat,"\nERROR: ", &removePath, " Failed to delete undo history!","\n Failed path: ",removePath);
	}
	
	return 0;
}

/*

/////////////////////////TO DO - high priority

* fix race condition (in getKey()?) causing key inputs to be printed as text and screwing up the display during laggy times
	-oof
	-might honestly be easier just to learn to write our own stream objects
	-can see it happen at EXACTLY brush size 14 when using the bucket tool on -O3
		-on -O0, it happens much earlier, so definitely its performance-related

* crash after returning from saveMmap????
	-FIXED(partially), apparently its an issue with FixedString. changing to std::string prevents the crash
		-will leave this here until i figure out whats wrong with it
	-it still saves the map successfully, and theres nothing after that except closing the file and returning
		-tried chaning return types, removing references, etc, nothing.

* Clipboard
	-saving and loading files (can be done manually by renaming them)
	-rotate canvas

* clean up naming and globals
	-extern vs recursive #include? going with extern for now
	-fix up naming inconsistencies like map/canvas
	-remove redundant and obsolete globals
		-dependent on DOD refactor
	-consistent typing?
		-signed vs unsigned

* custom tile input
	-broken
	-probably not possible to fill in tileset, just do one character.
	-might not be necessary now that we have the text tool

* big DOD refactor!
	-might have to wait until other high-priority things are addressed
	-reduce runtime branching
		-break up large functions
			-drawScreen() is probably the most performance-critical in this respect
			-specialize functions
				-performance testing for tail call optimization in a switch statement
				-call general code from function to avoid repetitious code?
				-increases abstraction and reduces readability, but probably not by much since most of this code is unlikely to change at this point
			-getKey() should be split up as well, as it is very performance-critical
			-split up functions instead of passing bool parameters
	-group common variables to ensure theyre cached together
		-x and y are probably the biggest ones
		-be careful of wasted memory with bools and smaller variables
	-refactor structs to be more memory efficient
		-test sizes to see if we can tell when memory is wasted
		-mixing types creates wasted space? will need to do more research
			-bools are the most suspect for wasted space, and we use too many as it is.

* WINDOWS SUPPORT!
	-unicode might as well be alien sorcery to the windows terminal
		-since Tiles are read as strings, most should just work once we get this fixed.
		-maybe just set data[0] to \u and offset the char data by one (will need to convert to 4 byte Tile)?
		-preprocessor macro to swap iostream overloads and manually pass \u before printing tile data?
	-the hard part is loading and saving files that will work on both platforms
		-it would probably be easier to just get the windows terminal to use unicode
	-have to create more preprocessor macros to automatically handle the platform differences	
	-getkey needs to be overhauled

///////////////////////////low-priority TO DO

-fix undesirable behavior on text tool using brushSize
	-move outside loops?

-custom tileset?
	-n-size array, or vector?
	-hook into tilePicker?

-copy/paste tool import from file?
	-half done, it does save/load automatically to file
	-maybe option in loadMap menu?
		-and saveMap to copy clipboard to another file

-write own file i/o streams based on C stdio.h
	-ive dealt with this nonsense for the last time!
	-im venting in the todo comment to remind myself that its worth the effort.
		-oh, whats that? deleted function? dont mind if i dont.
		-skip over entire lines of code because std::cin erroneously thinks its got something to say? i say ive written my last cin.ignore().
		-straightup lie about having opened and written to a file? the punishment is deprecation.
			
		-iostream is not even easier syntactically than cstdio, why did they have to make it so difficult?
		-im convinced whoever wrote the iostream library just wanted to watch us suffer

-fill out the rest of the const overloads for Tile

-standardize menu format
	-going with the while loop for now
		-need to change the rest to the while(key) format
	-loadMap() for pre-made menu screens?
		-separate menu Tile*, load one menu at a time(?) and use drawScreen() on the menu?
		-will actually have to fix showUI in drawScreen()

-structure placement tool? (walls&corners)
	-box brushes formula
	-only thin/thick walls? custom?

-increase Tile data block to 4 bytes
	-may help with windows compatibility as well
	-will simplify conversion/construction from int-based inputs
		-type punning
	-wont actually change the size of Tile object in memory since were using alignas
		-will increase file size though

-sanitize user input for map loading

-text tool centering?
	-maybe limit the bounds to the last character on the opposite end of the string

-implement 24-bit color instead of predetermined ascii colors?
	-maybe
	-int enum using byte offsets for defined colors?

-add paintControl controls for canvasEdit()

-half-block pixel imitation trick
	-nuh-uh
	-maybe for future project

-adjust free line formulae so its not as fat when drawing cardinal lines
	-inverse formula?

-options menu
	-line and circle factors
	-other 'constants'
	save to file?


***these will have to wait until work on the game itself gets underway:

1. ACTIVATORS
	-had them working on an older version, just need to update them

3. pathfinding/collision map layer
	-bit array for simplicity?
		-might be fun to have a 2D or 3D bit array....
			-would certainly have a smaller memory footprint
	-byte array to support simple height map?
		-may or may not be simpler than a 3D bit array in implementation

9. file versioning
	-will probably hold on this until everything gets more or less finalized.

- Tile rewrite
	-pre-generated string
		-test if single-digits will work with ascii formatting if they begin with 0
		-contain all print data in string, with esc codes

- cell-based maps
	-break up into smaller squares
	-cells are stored in individual contiguous arrays
	-should help performance somewhat
	-was going to do this anyway for worldgen

*/


