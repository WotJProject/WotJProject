#pragma once
//
//#include "tiles.cpp"

#include "WMM.h"


Tile tileSelected = {"▒",tcfgWhite,tcbgBlack,tfDefault}; //selected texture or feature
uint16 brushSize = 0;  //brush size. paintControl() clamps this to (mapSize/2)-1.

const char* cycleBrush[3] = {"Square","Filled","Hollow"};	//displays brush and brush mode on the GUI
uint8 cycleBrushIndex = 1;	//tracks indexing into cycleBrush for the GUI.

const uint8 undoLevels = 29;	//maximum index of undos to store. due to indexing, total number is the value +1
CycleNumber<uint8> undoSaveTarget = {undoLevels,0,undoLevels};	//points to the current undo state file. initialized to maxValue because calls increment it first (may refactor)

BoundNumber<uint8> availableUndos = {0,0,undoLevels};	//how many undos can be performed. incremented every time applyBrush() is called. decremented when an action is undone.
BoundNumber<uint8> availableRedos = {0,0,undoLevels};	//how many redos can be performed. incremented every time an action is undone. zeroed when an action is performed.

//refactor: make this adjustable? need an options menu first...
constexpr double circleFactor = 0.6666666666;    //used for circle brushes. increase to slim/sharpen, decrease to fatten/round
constexpr float lineFactor = 0.550000;	//slims the line for the lineFree brush when drawing at an angle. increase to thicken, decrease to thin.

uint8* fillArea = nullptr;	//used by bucket tool.

uint16 paletteSize = 5;    //max index offset of the cycleable palette, changed in brush selection.
CycleNumber<uint16> cycleIncrement(2,0,paletteSize);    //controls cycling between tiles for cycled brushes. refactor: may deprecate, we only have 2 modes for each brush anyway.
Tile cycleTile[16] = {"█","▓","▒","░"," ","≋",0,0,0,0,0,0,0,0,0,0}; //contains the tiles to cycle through. default is the Basic Brushes set. refactor: use tileset function in initialization

brushEnum brushType = squareShape; //lookup for rendering method of the brush
brushEnum brushPrev = noBrush;	//last brushType used. used with tilePicker and error handling

std::vector<Tile> textBuffer{};	//buffer for the text tool


//resizes the bounds when the brush or canvas is changed
//refactor: rename since we offloaded resize bounds checking to paintControl()
void brushResizeBounds(const brushEnum brushInput){
	//necessary to catch potential issues when interrupting multi-step tools.
	if(brushInput != lineFree && lineData.drawLine)
		lineData.drawLine = 0;
	
	if(brushInput != stampTool && stampData.stampMode)
		stampData.stampMode = 0;
	
	//wip: requires string centering to go off-canvas on negative x
	//if(brushInput == textMode && textBuffer.size()>1){
	//	if(x>mapSizeX-1 + brushSize)
	//		x = mapSizeX-1+brushSize;
	//	else if(x<-brushSize+1)
	//		x = -brushSize+1;
	//}else
	
	if(x>mapSizeX-1)
		x = mapSizeX-1;
	
	if(y>mapSizeY-1)
		y = mapSizeY-1;
	
	return;
}
void brushResizeBounds(){return brushResizeBounds(noBrush);}



//generates a uint8 buffer for contiguous filling based on the center brush tile and brush size.
//used by drawScreen and applyBrush
//refactor: this needs a lot of cleaning up
uint8* fillBucket(Tile* &inputMap, const uint32 fillx, const uint32 filly, const bool invert){
	const uint32 fillHeight = (2*brushSize)+1;
	const uint32 fillSize = sq(fillHeight);

	if(fillArea != nullptr)
		delete fillArea;
	fillArea = new uint8[fillSize];
	
	const Tile target = ptr2D(inputMap,mapSizeX,fillx,filly);
	
	//initialization of fillArea
	for(uint32 i = 0; i<fillSize; i++){fillArea[i] = invert;}
	ptr2D(fillArea, fillHeight, brushSize, brushSize) = 3 * !invert;	//center tile
	
	//match pass
	
	const sint32 xOffset = fillx+brushSize;
	const sint32 yOffset = filly+brushSize;
	
	for(sint32 wy = 0; wy<fillHeight; wy++)
	for(sint32 wx = 0; wx<fillHeight; wx++)
		if(xOffset-wx < mapSizeX && yOffset-wy < mapSizeY 
		&& ptr2D(inputMap, mapSizeX,(xOffset-wx),(yOffset-wy)) == target 
		&& ptr2D(fillArea,fillHeight,wx,wy) == invert)
			ptr2D(fillArea,fillHeight,wx,wy) = !invert;
					
	if(invert){	//exclusive mode
		for(sizet i=0; i<fillSize; i++)
			if(fillArea[i])
				fillArea[i] = 3;
	}else{
		//neighbor pass
		//refactor: look up more Blazingly Fast algorithms
		
		bool noMore = false;
		
		while(!noMore){
			noMore = true;
			
			//do the flags really help here?
			for(sizet i=0; i<fillSize; i++){
				uint8 boundFlags = 0;
				
				enum : uint8{
					topEdge = 0b0001,
					bottomEdge = 0b0010,
					leftEdge = 0b0100,
					rightEdge = 0b1000
				};
				
				boundFlags |= (i<fillHeight) * topEdge;
				boundFlags |= (i>(fillSize-fillHeight)) * bottomEdge;
				boundFlags |= (((i+1)%fillHeight) == 0) * leftEdge;	//i unflipped X axis, so this might be the right edge now.
				boundFlags |= (!i || ((i+1)%fillHeight) == 1) * rightEdge;
				
				if(fillArea[i] == 1){
					if(
					(!(boundFlags & topEdge) && fillArea[i-fillHeight] == 3) || 
					(!(boundFlags & bottomEdge) && fillArea[i+fillHeight] == 3) ||
					(!(boundFlags & leftEdge) && fillArea[i+1] == 3) || 
					(!(boundFlags & rightEdge) && fillArea[i-1] == 3)
					){
						noMore = false;
						fillArea[i] = 3;
					}
				}
			}
		}
	
		//cull islands
		for(sizet i=0; i<fillSize; i++){
			if(fillArea[i] < 3)
				fillArea[i] = 0;
		}
	}
	
	return fillArea;
}
uint8* fillBucket(Tile* &inputMap, const uint32 fillx, const uint32 filly){
	return fillBucket(inputMap,fillx,filly,false);
}

//refactor: dont need the array anymore, consider deprecating and replacing with struct of explicit variables for the gui.
//	for DOD refactor: break this up into brush-specific functions so were not jumping all over the place. should make it easier to read, too.
void selectBrushType(const brushEnum selected){
	brushPrev = brushType;
	
	switch(selected){
		default:
		case squareShape:{
			brushType = squareShape;
			
			cycleBrush[0] = "Square";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 1;
			break;
		}
		case squareHollowShape:{
			brushType = squareHollowShape;
			
			cycleBrush[0] = "Square";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 2;
			break;
		}
		case diamondShape:{
			brushType = diamondShape;
			
			cycleBrush[0] = "Diamond";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 1;
			break;
		}
		case diamondHollowShape:{
			brushType = diamondHollowShape;
			
			cycleBrush[0] = "Diamond";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 2;
			break;
		}
		case circleShape:{
			brushType = circleShape;
			
			cycleBrush[0] = "Circle";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 1;
			
			break;
		}
		case circleHollowShape:{
			brushType = circleHollowShape;
			
			cycleBrush[0] = "Circle";
			cycleBrush[1] = "Filled";
			cycleBrush[2] = "Hollow";
			cycleBrushIndex = 2;
			
			break;
		}
		case lineVertShape:{
			brushType = lineVertShape;
			
			cycleBrush[0] = "Cardinal Line";
			cycleBrush[1] = "Vertical";
			cycleBrush[2] = "Horizontal";
			cycleBrushIndex = 1;
			break;
		}
		case lineHoriShape:{
			brushType = lineHoriShape;
			
			cycleBrush[0] = "Cardinal Line";
			cycleBrush[1] = "Vertical";
			cycleBrush[2] = "Horizontal";
			cycleBrushIndex = 2;
			break;
		}
		case bucketFill:{
			brushType = bucketFill;
			fillBucket(map,x,y,false);
			
			cycleBrush[0] = "Bucket";
			cycleBrush[1] = "Contiguous";
			cycleBrush[2] = "Exclusive";
			cycleBrushIndex = 1;
			break;
		}
		case bucketFillEx:{
			brushType = bucketFillEx;
			fillBucket(map,x,y,true);
			
			cycleBrush[0] = "Bucket";
			cycleBrush[1] = "Contiguous";
			cycleBrush[2] = "Exclusive";
			cycleBrushIndex = 2;
			break;
		}
		case tilePicker:{
			brushPrev = brushType;
			brushType = tilePicker;
			
			cycleBrush[0] = "Picker";
			cycleBrush[1] = "Pick a Tile";	//leaving this null causes the brush to malfunction.
			cycleBrushIndex = 1;
			break;
		}
		case lineFree:{
			brushType = lineFree;
			lineData.drawLine = 0;
			
			cycleBrush[0] = "Free Line";
			cycleBrush[1] = "Place 1st Point";
			cycleBrush[2] = "Place 2nd Point";
			cycleBrushIndex = 1;
			
			break;
		}
		case textMode:{
			brushType = textMode;
			textBuffer.reserve(mapSizeX);
			brushSize = textBuffer.size();
			
			cycleBrush[0] = "Text";
			cycleBrush[1] = "Placement";
			cycleBrush[2] = "Typing...";
			cycleBrushIndex = 1;
			break;
		}
		case stampTool:{
			std::cout << std::flush;
			brushType = stampTool;
			
			///may re-enable this later
			//if(std::filesystem::exists("../maps/history/clipboard.wmap")){
			//	if(stampData.clipboard != nullptr){
			//		delete stampData.clipboard;
			//	}
			//	
			//	if(!loadMap("clipboard"))
			//		stampData.clipboard = nullptr;
			//}
			if(stampData.clipboard){
				stampData.stampMode = 2;
				cycleBrushIndex = 2;
			}else{
				stampData.stampMode = 0;
				cycleBrushIndex = 1;
			}
			
			cycleBrush[0] = "Stamp";
			cycleBrush[1] = "Select Area";
			cycleBrush[2] = "Paste Selection";
			
			break;
		}
	}
	brushResizeBounds(brushType);
	return;
}
//DOD refactor: can probably make this the primary version, and call the tool-specific functions instead of going back and forth
void selectBrushType(keyt selected){
	if(!selected){
		clearScreen;
		terminalHideCursor;
		
		//legacy menu; may deprecate, as its unlikely to be used
		out(lbrFormat,"Choose your brush.","Use the \\ or ? key in paint mode to switch brush modes.",\
		"1. Square",\
		"2. Diamond",\
		"3. Circle",\
		"4. Cardinal Line",\
		"5. Free Line",\
		"6. Bucket Fill",\
		"7. Tile Picker",\
		"8. Text Mode",\
		"9. Copy/Paste Tool",\
		"\nTAB - Return to Main Menu");
		
		//not sure if i like doing it this way, but logic operators still let some undesired values through...
		// not like performance matters on a menu like this anyway
		while(selected!=tabKey 
		&& selected!=oneKey 
		&& selected!=twoKey 
		&& selected!=threeKey 
		&& selected!=fourKey 
		&& selected!=fiveKey 
		&& selected!=sixKey 
		&& selected!=sevenKey 
		&& selected!=eightKey 
		&& selected!=nineKey)
			getKey(false, selected);
	}
	switch(selected){
		default:{
			return;
		}
		case oneKey:{
			return selectBrushType(squareShape);
		}
		case twoKey:{
			return selectBrushType(diamondShape);
		}
		case threeKey:{
			return selectBrushType(circleShape);
		}
		case fourKey:{
			return selectBrushType(lineHoriShape);
		}
		case fiveKey:{
			return selectBrushType(lineFree);
		}
		case sixKey:{
			return selectBrushType(bucketFill);
		}
		case sevenKey:{
			return selectBrushType(tilePicker);
		}
		case eightKey:{
			return selectBrushType(textMode);
		}
		case nineKey:{
			return selectBrushType(stampTool);
		}
		case tabKey:{
			return;
		}
		
	}
	return;
}
void selectBrushType(){keyt k = getKey(false); return selectBrushType(k);}

//applies current brush to the selected area
//DOD refactor: break this up (partially? entirely?) into brush-specific functions to eliminate needless branching and simplify code
void applyBrush(const Tile paint, const uint16 size, const brushEnum shape, const uint32 posx, const uint32 posy){
	if(!shape)
		return;
	
	//non-looping brushes go up here:
	
	else if(shape == tilePicker){
		tileSelected = ptr2D(map,mapSizeX,posx,posy);
		
		if(brushPrev == textMode)
			for(Tile tile : textBuffer)
				tile.copyFormatting(tileSelected);
		
		selectBrushType(brushPrev);
		brushPrev = noBrush;
		return;
	}
	else if(shape == lineFree){
		
		if(!lineData.drawLine){	//initialize on first point
			lineData.x1 = posx;
			lineData.y1 = posy;
			lineData.x2 = posx;
			lineData.y2 = posy;
			
			lineData.left = posx;
			lineData.right = posx;
			lineData.top = posy;
			lineData.bottom = posy;
			
			lineData.drawLine = 1;
			cycleBrushIndex = 2;
			return;
		}else{
			const uint16 lineSize = brushSize+1;
			//must be a float to calculate properly
			const sfloat32 lineWeight = lineSize * lineFactor;
			
			lineData.x2 = posx;
			lineData.y2 = posy;
			
			lineData.run = lineData.x2 - lineData.x1;
			lineData.rise = lineData.y2 - lineData.y1;
			
			if(lineData.x1 < lineData.x2){
				lineData.left = lineData.x1;
				lineData.right = lineData.x2;
			}else{
				lineData.left = lineData.x2;
				lineData.right = lineData.x1;
			}
			
			if(lineData.y1 < lineData.y2){
				lineData.top = lineData.y2;
				lineData.bottom = lineData.y1;
			}else{
				lineData.top = lineData.y1;
				lineData.bottom = lineData.y2;
			}
			
			if(lineData.run && lineData.rise)
				lineData.slope = lineData.rise/lineData.run;
			else
				lineData.slope = 0;
			
			//refactor: look up a less burdensome formula
			for(sint32 painty = 0; painty < mapSizeY; painty++){
				for(sint32 paintx = 0; paintx < mapSizeX; paintx++){
					
					if(!lineData.rise){	//horizontal line
						if(painty < lineData.y2 + lineWeight
						&& painty > lineData.y2 - lineWeight
						&& paintx < lineData.right + lineWeight
						&& paintx > lineData.left - lineWeight)
							ptr2D(map,mapSizeX,paintx,painty) = paint;
							
					}else if(!lineData.run){	//vertical line
						if(paintx < lineData.x2 + lineWeight
						&& paintx > lineData.x2 - lineWeight
						&& painty < lineData.top + lineWeight
						&& painty > lineData.bottom - lineWeight)
							ptr2D(map,mapSizeX,paintx,painty) = paint;
							
					}else{
						if(((painty - paintx*lineData.slope < lineData.y1 - lineData.x1*lineData.slope + lineWeight
						 && painty - paintx*lineData.slope > lineData.y1 - lineData.x1*lineData.slope - lineWeight)
						|| (paintx - painty/lineData.slope < lineData.x1 - lineData.y1/lineData.slope + lineWeight
						 && paintx - painty/lineData.slope > lineData.x1 - lineData.y1/lineData.slope - lineWeight))
							
						&& painty < lineData.top + lineWeight
						&& painty > lineData.bottom - lineWeight
						&& paintx < lineData.right + lineWeight
						&& paintx > lineData.left - lineWeight)
							ptr2D(map,mapSizeX,paintx,painty) = paint;
					}
				}
			}
			lineData.drawLine = 0;
			cycleBrushIndex = 1;
		}
	}
	else if(shape == textMode){
		
		for(uint32 i=0; i<textBuffer.size(); i++)
			if(posx+i < mapSizeX)
				ptr2D(map,mapSizeX,(posx+i),(posy)) = textBuffer[i];
	}
	else if(shape == stampTool){
		
		if(stampData.stampMode == 0){	//initialize with first point
			stampData.top = posy;
			stampData.bottom = posy;
			stampData.left = posx;
			stampData.right = posx;
			
			stampData.a.x = posx;
			stampData.a.y = posy;
			stampData.b.x = posx;
			stampData.b.y = posy;
			
			stampData.clipWidth = 0;
			stampData.clipHeight = 0;
			
			stampData.stampMode = 1;
			return;
		}
		else if(stampData.stampMode == 1){	//record 2nd point and save copied area
			stampData.b.x = posx;
			stampData.b.y = posy;
			
			if(stampData.b.x > stampData.a.x){
				stampData.left = stampData.a.x;
				stampData.right = stampData.b.x;
			}else{
				stampData.left = stampData.b.x;
				stampData.right = stampData.a.x;
			}
			if(stampData.b.y > stampData.a.y){
				stampData.top = stampData.a.y;
				stampData.bottom = stampData.b.y;
			}else{
				stampData.top = stampData.b.y;
				stampData.bottom = stampData.a.y;
			}
			
			stampData.clipWidth = stampData.right-stampData.left+1;
			stampData.clipHeight = stampData.bottom-stampData.top+1;
			
			if(stampData.clipboard)
				delete stampData.clipboard;
			stampData.clipboard = new Tile[stampData.clipWidth * stampData.clipHeight];
			
			for(uint32 iy=0; iy<stampData.clipHeight; iy++)
			for(uint32 ix=0; ix<stampData.clipWidth; ix++)
				ptr2D(stampData.clipboard,stampData.clipWidth,ix,iy) = ptr2D(map,mapSizeX,stampData.left+ix,stampData.top+iy);
			
			clipboardSave();
			stampData.stampMode = 2;
			cycleBrushIndex = 2;
			return;
		}
		else if(stampData.stampMode == 2){	//paste mode
			
			const sint32 mapx = posx - (stampData.clipWidth>>1);
			const sint32 mapy = posy - (stampData.clipHeight>>1);
			sint32 clipx;
			sint32 clipy;
			
			for(sint32 iy = 0; iy<stampData.clipHeight; iy++)
			for(sint32 ix = 0; ix<stampData.clipWidth; ix++){
				clipx = mapx + ix;
				clipy = mapy + iy;
				
				if(clipx < mapSizeX && clipy < mapSizeY && clipx > -1 && clipy > -1)
					ptr2D(map,mapSizeX,clipx,clipy) = ptr2D(stampData.clipboard,stampData.clipWidth, ix, iy);
			}
		}
	}
	
	//looping brushes go down here:
	else{
	
		sint32 truex;
		sint32 truey;

		for(sint32 painty = -size; painty <= size; painty++)
		for(sint32 paintx = -size; paintx <= size; paintx++){
			truex = posx+paintx;
			truey = posy+painty;
			
			switch(shape){
				default:
				case noBrush:{
					return;
				}
				case squareShape:{
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case squareHollowShape:{
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if((paintx==size) || (painty==size) || (paintx==-size) || (painty==-size))
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case diamondShape:{     //+/-1 necessary for accurate size
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if(!(paintx<=(painty-size-1) || paintx>=(painty+size+1) || -paintx<=(painty-size-1) || -paintx>=(painty+size+1)\
						  || painty<=(paintx-size-1) || painty>=(paintx+size+1) || -painty<=(paintx-size-1) || -painty>=(paintx+size+1)))
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case diamondHollowShape:{
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if((paintx==(painty-size) || paintx==(painty+size) || -paintx==(painty-size) || -paintx==(painty+size)\
						  || painty==(paintx-size) || painty==(paintx+size) || -painty==(paintx-size) || -painty==(paintx+size)))
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case circleShape:{
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if (sqrtl((paintx*paintx) + (painty*painty))/circleFactor < size+1)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case circleHollowShape:{
					static sfloat32 pythagFactor;
					pythagFactor = sqrtl((paintx*paintx) + (painty*painty))/circleFactor;
					
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if (pythagFactor < size+1 && pythagFactor > size-circleFactor)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case lineVertShape:{
					if(truey < mapSizeY && truey > -1)
						if(!paintx)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case lineHoriShape:{
					if(truex < mapSizeX && truex > -1)
						if(!painty)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case bucketFill:{
					if(fillArea == nullptr)
						fillBucket(map,posx,posy,false);
					
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if(ptr2D(fillArea,(size*2)+1, size-paintx, size-painty) == 3)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
				case bucketFillEx:{
					if(fillArea == nullptr)
						fillBucket(map,posx,posy,false);
					
					if(truex < mapSizeX && truey < mapSizeY && truex > -1 && truey > -1)
						if(ptr2D(fillArea,(size*2)+1, size-paintx, size-painty) == 3)
							ptr2D(map,mapSizeX,truex,truey) = paint;
					break;
				}
			}
		}
    }
	
	if(historySave()){
		availableUndos++;
		availableRedos = 0;
	}
	clearScreen;
	return;
}

///@WIP. very much wip and outdated code.
//handler for placing activators. no input brings up the menu
int placeActivator(){
    ActivatorMenu:
    clearScreen;
	wip;
	return 0;
	Tile inputCharacter;
	std::fstream listFile;
	sint32 i = 0;	//placeholder for former global.
			//if (!listFile){ //debug - add custom directory punctuation detection!
			//	for (i = 0; inputName[i]; i++){   //intentionally does not redefine i so it keeps its scope
			//		if (customDir){ //set when loading a map with a custom directory
			//		std::string inputListName;  //holds custom list directory
			//		out("\n\nCustom map directory indicated. Please input location of Activator Masterlist .wlst file:\n");
			//		std::cin >> inputListName;
			//		listFile.open((inputListName), std::ios::app); //assumes a custom directory is specified and attempts to open it
			//		break;
			//	}
			//}
			//if (!inputName[i])  //if no indicative punctuation is found, opens the map normally using only the name
			//	listFile.open(("maps\\" + inputName + ".wlst"), std::ios::app); //automatically opens map from its PROPER directory
			//if(!listFile){
			//	out(lbrFormat,"\nERROR: Could not open .wlst file. Please re-check file name or input.");
			//	pause; goto ActivatorMenu;
			//}
		//}
    out(lbrFormat,"Insert activator-WIP:\n",\
            "1. Door/transition",\
            "2. Signage or other basic object",\
            "3. NPC or complex object",\
            "4. Load from file",\
            "5. Delete activator at this location",\
            "\nESC - Cancel");
    /*
    activator file entry format is as follows:
    section header $ 
    activator type - Door, Sign, NPC, etc
    x position
    y position
    activator data
        data strings and paths should be enclosed in {}
    */
    switch(getKey(false)){
        default:
            goto ActivatorMenu;
        case oneKey:{   //  activators\doors
            ActivatorDoor:
            clearScreen;
			
            std::string activatorName;  //display name for the activator
            std::string inputLink;  //holds directory for linked file
            std::fstream fileTest;
            out(lbrFormat,"Input .wmap map file (with extension and relative directory) for door to link to:");   //eg. maps/rooms/bedroom.wmap
            std::cin >> inputLink;
            fileTest.open(inputLink);
            if(!fileTest){
                out(lbrFormat,"\nWARNING: Target file could not be found/opened. Continue?",\
                        "\nENTER - Continue anyway",\
                        "TAB - Return to menu",\
                        "Any other key: Try again.\n\n");
                switch (getKey(false)){    //  activators\doors\notfound
                    default:{
                        fileTest.close();
                        goto ActivatorDoor;
					}
                    case enterKey:
                        break;
                    case tabKey:{
                        fileTest.close();
                        goto ActivatorMenu;
					}
                }
            }
            fileTest.close();
            ActivatorDoorCoords:
            uint32 tx = 0;    //target position X
            uint32 ty = 0;    //target position Y
			
            out(lbrFormat,"Input x,y coordinates on target map:");
            out(lbrFormat,"X: ");
            std::cin >> tx;
            out(lbrFormat,"Y: ");
            std::cin >> ty;
			
            if(!tx || !ty){
                out(lbrFormat,"ERROR: invalid entries, please try again:");
                pause; goto ActivatorDoorCoords;
			}
			
            listFile << "$Door\n";
            listFile << "{";
			
            out(lbrFormat,"Input name to display for the door when inspected:\n");
            getline(std::cin, activatorName);
            getline(std::cin, activatorName);   //double up due to issues with getline and << operators
			
            out(lbrFormat,"Storing string:",activatorName);
            listFile << activatorName;
            out(lbrFormat,"String stored.");
			listFile << "}\n";

            listFile << x; 
            listFile << std::endl;
            listFile << y;
            listFile << std::endl;
            listFile << ("{" + inputLink + "}\n");
            listFile << tx;
            listFile << std::endl;
            listFile << ty;
            listFile << std::endl;
            ptr2D(map,mapSizeX,x,y) = "⌂";    // ⌂    currently only uses this symbol for doors/transitions.
			
            listFile.close();
            break;
        }
        case twoKey:{   //  activators\signs
            std::string activatorName;  //display name/title for the activator
            std::string signData;  //data when sign is accessed by the player

            clearScreen;
            out(lbrFormat,"Initializing... Press Enter to Continue");
            listFile << "$Sign\n";
            listFile << "{";
            //streamClear();	//deprecated
			std::cin.ignore();

            out(lbrFormat,"Input Sign Title:\n");
            getline(std::cin, activatorName);
            // getline(std::cin, activatorName);   //double up due to issues with getline and << operators
            out(lbrFormat,"\nStoring string:",activatorName);
            listFile << activatorName;
            out(lbrFormat,"String stored.\n");
            listFile << "}\n";

            listFile << x << std::endl;
            listFile << y << std::endl;
            
			listFile << "{";
            out(lbrFormat,"\nInput Sign Data:");
            getline(std::cin, signData);
            // getline(std::cin, signData);   //double up due to issues with getline and << operators
            out(lbrFormat,"Storing string:",signData);
            listFile << signData;
            out(lbrFormat,"String stored.");
            listFile << "}\n";
			ptr2D(map,mapSizeX,x,y) = "¶";    // ¶    Symbol for signs (for now)
			
            listFile.close();
            break;
        }
        case threeKey:{ //  activators\NPCs
            wip;
            break;
        }
        case fourKey:{  //  activators\load
            wip;
            break;  //wip
            out("\nInput .wob file or .wlst file (with extension and relative directory).\nWOB files will place single activator, while WLST files will load every activator\n",\
                "and attempt to place them at their designated coordinates.");   //eg. maps/rooms/bedroom.wlst
            
            


            ptr2D(map,mapSizeX,x,y) = inputCharacter;
            break;
        }
        case fiveKey:{  //  activators/delete
            wip;
            break;
        }
        case tabKey:
            break;
    }
    listFile.close();
    return 0;
}
//wip. handler for placing activators. string input loads from file
int placeActivator(std::string input){
	wip; return 0;
    std::ifstream inputFile;
    inputFile.open(input, std::ios::binary);
    if (!inputFile){
        out(lbrFormat,"ERROR: File could not be opened.\n");
        pause; return 1;
    }
    wip;
    return 0;
}


void selectBrushTileset(){
	
	clearScreen;
	terminalHideCursor;
	
	out(lbrFormat,"Choose Brush Tileset:\n",\
	"1.Basic Textures: █▓▒░ ≋",\
	"2.Thin Walls: ├┴",\
	"3.Thick Walls: ╠╩",\
	"4.Blocks: ▙▞▟",\
	"5.Terrain Presets",\
	"6.Color Presets",\
	"\nTAB - Return to Main Menu");
	//"\n9.Raw Input",\
	
	switch(getKey(false)){	//refactor: pass in string alone, without formatting? may have undesired behavior. note: i no longer have any idea what i meant by this.
		default:{
				selectBrushTileset();
				break;
			}
		case oneKey:{
			cycleIncrement = 4;
			paletteSize = 5;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = {"█",tfDefault,tfDefault,tfDefault};
			cycleTile[1] = {"▓",tfDefault,tfDefault,tfDefault};
			cycleTile[2] = {"▒",tfDefault,tfDefault,tfDefault};
			cycleTile[3] = {"░",tfDefault,tfDefault,tfDefault};
			cycleTile[4] = {" ",tfDefault,tfDefault,tfDefault};
			cycleTile[5] = {"≋",tfDefault,tfDefault,tfDefault};
			
			tileSelected = cycleTile[4];
			tileSelected.background = cycleBGcolor[colorsBGindex].background;	//reapply colors
			tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;      
			break;
		}
		case twoKey:{
			cycleIncrement = 0;
			paletteSize = 10;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = {"│",tfDefault,tfDefault,tfDefault};
			cycleTile[1] = {"─",tfDefault,tfDefault,tfDefault};
			cycleTile[2] = {"┌",tfDefault,tfDefault,tfDefault};
			cycleTile[3] = {"┐",tfDefault,tfDefault,tfDefault};
			cycleTile[4] = {"┘",tfDefault,tfDefault,tfDefault};
			cycleTile[5] = {"└",tfDefault,tfDefault,tfDefault};
			cycleTile[6] = {"┤",tfDefault,tfDefault,tfDefault};
			cycleTile[7] = {"├",tfDefault,tfDefault,tfDefault};
			cycleTile[8] = {"┬",tfDefault,tfDefault,tfDefault};
			cycleTile[9] = {"┴",tfDefault,tfDefault,tfDefault};
			cycleTile[10] = {"┼",tfDefault,tfDefault,tfDefault};
			
			tileSelected = cycleTile[0];
			tileSelected.background = cycleBGcolor[colorsBGindex].background;	//reapply colors
			tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;
			break;
			}
		case threeKey:{
			cycleIncrement = 0;
			paletteSize = 10;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = {"║",tfDefault,tfDefault,tfDefault};
			cycleTile[1] = {"═",tfDefault,tfDefault,tfDefault};
			cycleTile[2] = {"╔",tfDefault,tfDefault,tfDefault};
			cycleTile[3] = {"╗",tfDefault,tfDefault,tfDefault};
			cycleTile[4] = {"╝",tfDefault,tfDefault,tfDefault};
			cycleTile[5] = {"╚",tfDefault,tfDefault,tfDefault};
			cycleTile[6] = {"╣",tfDefault,tfDefault,tfDefault};
			cycleTile[7] = {"╠",tfDefault,tfDefault,tfDefault};
			cycleTile[8] = {"╦",tfDefault,tfDefault,tfDefault};
			cycleTile[9] = {"╩",tfDefault,tfDefault,tfDefault};
			cycleTile[10] = {"╬",tfDefault,tfDefault,tfDefault};
			
			tileSelected = cycleTile[0];
			tileSelected.background = cycleBGcolor[colorsBGindex].background;	//reapply colors
			tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;      
			break;
		}
		case fourKey:{
			cycleIncrement = 0;
			paletteSize = 15;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = {"▐",tfDefault,tfDefault,tfDefault};
			cycleTile[1] = {"▌",tfDefault,tfDefault,tfDefault};
			cycleTile[2] = {"▀",tfDefault,tfDefault,tfDefault};
			cycleTile[3] = {"▄",tfDefault,tfDefault,tfDefault};
			cycleTile[4] = {"▙",tfDefault,tfDefault,tfDefault};
			cycleTile[5] = {"▛",tfDefault,tfDefault,tfDefault};
			cycleTile[6] = {"▜",tfDefault,tfDefault,tfDefault};
			cycleTile[7] = {"▟",tfDefault,tfDefault,tfDefault};
			cycleTile[8] = {"▚",tfDefault,tfDefault,tfDefault};
			cycleTile[9] = {"▞",tfDefault,tfDefault,tfDefault};
			cycleTile[10] = {"▖",tfDefault,tfDefault,tfDefault};
			cycleTile[11] = {"▗",tfDefault,tfDefault,tfDefault};
			cycleTile[12] = {"▘",tfDefault,tfDefault,tfDefault};
			cycleTile[13] = {"▝",tfDefault,tfDefault,tfDefault};
			cycleTile[14] = {" ",tfDefault,tfDefault,tfDefault};
			cycleTile[15] = {"█",tfDefault,tfDefault,tfDefault};
			
			tileSelected = cycleTile[0];
			tileSelected.background = cycleBGcolor[colorsBGindex].background;	//reapply colors
			tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;   
			break;
		}
		case fiveKey:{
			cycleIncrement = 0;
			paletteSize = 8;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = stdtiles::grassDeep;
			cycleTile[1] = stdtiles::grassThick;
			cycleTile[2] = stdtiles::grassField;
			cycleTile[3] = stdtiles::grassSparse;
			cycleTile[4] = stdtiles::rock;
			cycleTile[5] = stdtiles::sand;
			cycleTile[6] = stdtiles::water;
			cycleTile[7] = stdtiles::snow;
			cycleTile[8] = stdtiles::empty;
			
			tileSelected = cycleTile[0];
			
			break;
		}
		case sixKey:{
			cycleIncrement = 0;
			paletteSize = 8;
			cycleIncrement.maxValue = paletteSize;
			
			cycleTile[0] = stdtiles::red;
			cycleTile[1] = stdtiles::blue;
			cycleTile[2] = stdtiles::green;
			cycleTile[3] = stdtiles::yellow;
			cycleTile[4] = stdtiles::brown;
			cycleTile[5] = stdtiles::cyan;
			cycleTile[6] = stdtiles::pink;
			cycleTile[7] = stdtiles::purple;
			cycleTile[8] = stdtiles::empty;
			
			tileSelected = cycleTile[0];
			
			break;
		}
		//deprecated: text tool replaces it
		//case nineKey:{  main\brush\raw input
			//clearScreen;
			//wip;
			//break;
			//out(lbrFormat,"WARNING: Putting in multiple visible characters will not work good!\n","Enter desired character:\n");
//			
			//FixedString<4> potato;
			//std::cin >> potato;
			//if(!potato[0])
				//tileSelected = " ";
			//else
				//tileSelected = potato.data;
//			
			//cycleIncrement = 0;
			//paletteSize = 0;
			//cycleIncrement.maxValue = 0;
//			
			//cycleTile[0] = tileSelected;
			//tileSelected.background = cycleBGcolor[colorsBGindex].background;	reapply colors
			//tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;     
			//break;
		//}
		case tabKey:
			return;
	}
	return;
}





