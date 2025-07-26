#include "WMM.h"
#pragma once

/*
color switching:

index the vector using colorsFGindex and colorsBGindex:
colorSet[colorsFGindex];

this way, its all set up to automatically calculate the bounds,
 so we dont have to keep track during incrementation.
*/
//for indexing colorSet
CycleNumber<uint8> colorsFGindex(13, 0, (colorSetFG.size()-1));
CycleNumber<uint8> colorsBGindex(0, 0, (colorSetBG.size()-1));

//tracks selected colors.
Tile cycleFGcolor[colorSetFG.size()];
Tile cycleBGcolor[colorSetBG.size()];

uint32 x = 0;    //brush horizontal position
uint32 y = 0;    //brush vertical position

constexpr uint32 screenWidth = 135;	//default screen size. actual screen size is viewXRadius and viewYRadius.
constexpr uint32 screenHeight = 45;	//

//these need to be signed for certain operations!
sint32 viewXRadius = screenWidth >>1; //vertical radius to render in the viewport
sint32 viewYRadius = (screenHeight >>1) - 6; //horizontal radius to render in the viewport, accounting for GUI

//any commands that need to be run to format the screen can be put here
bool initScreen(const bool force){	
	static bool initialized = false;    //whether to run first-run code
	if(initialized && !force)
		return true;
	else if(force)
		initialized = false;
		
	
#ifdef wotj_WINDOWS
	//zzz;	//wip: windows terminal hates unicode and fun and probably kittens too.
	
	HANDLE currentBuffer;
	HANDLE screenHandle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	
	consoleInitialize();	//also enables VT mode.
	
    SetConsoleTitle("Wanderer Mapmaker");
	
#elif defined(wotj_LINUX)	//wip platform detection
	static struct termios oldSettings;
	
	tcgetattr(0, &oldSettings);
	struct termios newSettings = oldSettings;
	
	newSettings.c_lflag &= ~(ICANON | ECHO | ISIG);	//canonical mode, print input characters, input signals, and new-line echo all turned off.
	newSettings.c_lflag &= (IEXTEN);	//extended special characters is turned on
	//newSettings.c_iflag &= ~(ICRNL);	//makes enter and ctrl+m return 13 instead of 10, disables new-line returns
	newSettings.c_iflag &= ~(IXON);	//turns off additional Ctrl commands
	newSettings.c_cc[VTIME] = 0;
	newSettings.c_cc[VMIN] = 1;
	
	tcsetattr(0,TCSAFLUSH, &newSettings);
	atexit([](){tcsetattr(0, TCSAFLUSH, &oldSettings);});
	
	out("\033]0;Wanderer Mapmaker\007");
#endif
	
    terminalHideCursor;
	
	
	keyt detectInput;
	while (detectInput != enterKey && detectInput != carRetKey){
		clearScreen;
		terminalResetCursor;
		terminalHideCursor;
		
		static Tile fill = "▒";
		const Tile border = {"■", tcfgBlue, tcbgBlack};
		
		for(int hig = 0; hig<screenHeight; hig++){
			for(int wid = 0; wid< screenWidth; wid++){
				if(!wid || !hig || wid==screenWidth-1 || hig==screenHeight-1){
					std::cout << border;
				}else{
					fill.background = colorSetBG[colorsFGindex + hig];
					fill.foreground = colorSetFG[colorsFGindex + wid];
					std::cout << fill;
				}
			}
			terminalClearFormat(std::cout);
			out(lbr);
		}
		
		terminalMoveCursor(6,5);
		terminalApplyFormat(std::cout, tcfgGreen);
		out("Press ENTER when finished.");
		terminalMoveCursor(7,5);
		out("Press any other key to refresh box.");
		
		terminalMoveCursor(8,5);
		terminalApplyFormat(std::cout, tcfgYellow);
		out("The border should be small blue squares, and the center filled with rainbow plaid.");
		terminalMoveCursor(9,5);
		out("If this is not what you see, change your terminal settings.");
		
		terminalMoveCursor(10,5);
		terminalApplyFormat(std::cout, tfBold, tcfgCyan);
		out("You WILL be able to increase screen size while in draw mode, but this is the minimum size.");
		terminalClearFormat(std::cout);
		
		//this needs to draw last to make sure its in the user's view when the window is too small
		terminalMoveCursor(5,5);
		terminalApplyFormat(std::cout, tfBold, tcbgDrkBlue, tcfgYellow, tfUnderline);
		out("RESIZE OR ZOOM WINDOW UNTIL BOX FITS WITHOUT WRAPPING");
		terminalClearFormat(std::cout);
		
		getKey(false, detectInput);
	}
	
	//for drawScreen GUI
	//refactor: move to drawScreen?
	for(uint8 i = 0; i<colorSetFG.size(); i++){
		cycleFGcolor[i] = "█";
		cycleFGcolor[i].foreground = colorSetFG[i];
	}
	for(uint8 i = 0; i<colorSetBG.size(); i++){
		cycleBGcolor[i] = " ";
		cycleBGcolor[i].background = colorSetBG[i];
	}
	
	//initialize default map
	if(!loadMap("chernocolor")){
		mapSizeX = 100;
		mapSizeY = 100;
		map = new Tile[mapSizeX * mapSizeY];
		
		x = 50;
		y = 50;
	
		for(uint32 i = 0; i < mapSizeX*mapSizeY; i++){
			if(!(i%mapSizeX))	//left edge (and also indexing test)
				map[i] = stdtiles::brown;
			else if(!((i+1)%mapSizeX))	//right edge
				map[i] = stdtiles::yellow;
			else if(i>(mapSizeX*(mapSizeY-1)))	//bottom row
				map[i] = stdtiles::blue;
			else if(i<mapSizeX)	//top row
				map[i] = stdtiles::red;
			else
				map[i] = (i % 128 ? stdtiles::water : stdtiles::grassDeep);
		}
	
		//initialize undo state
		undoSaveTarget = undoSaveTarget.maxValue;
		availableUndos = 0;
		availableRedos = 0;
		//note: keep an eye on this, there may be situations where we want to initialize these when also loading a map
	}
	
	if(!historySave()){	//establish initial undo state
		out(std::cerr,noFormat,"ERROR: Failed to initialize undo state!");
		throw "Initialization failed. Closing application.";	//refactor: perhaps try to disable undo/redo instead of crashing.
	}
	
	if(std::filesystem::exists("../maps/history/clipboard.wmap"))
		if(clipboardLoad()){
			stampData.stampMode = 2;
		}
	
	initialized = true;
	clearScreen;
    return initialized;
}
 bool initScreen(){return initScreen(false);}



//draws the current screen using the player position x and y.
//refactor: this function has grown way too big and complex, it needs a rewrite.
void drawScreen(Tile*& inputMap, const uint32 posx, const uint32 posy, const bool drawBrush, const bool showUI){
	static uint16 refresh = 0;
	if(!(refresh++%5) && drawBrush && brushSize>12)	//wip: above 12 seems to be where performance begins to fall, but this may vary by machine.
		clearScreen;
	
    terminalResetCursor;
    terminalHideCursor;

    for(sint32 drawy = -viewYRadius; drawy<viewYRadius && drawBrush; drawy++){
		out("\n ");	//render line padding
		for(sint32 drawx = -viewXRadius; drawx<viewXRadius && drawBrush; drawx++){
				
			static sint32 truex;
			static sint32 truey;
			
			truex = (posx+drawx);	//translates drawing coordinate to map coordinate
			truey = (posy+drawy);	//translates drawing coordinate to map coordinate
	
			if (truex>=mapSizeX || truey>=mapSizeY)
                std::cout << " "; 
            
			else if(brushType == lineFree){
				if(!lineData.drawLine){
					if(!drawx && !drawy)
						std::cout << tileSelected;
					else
						std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
				}
				else{
					
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
						
					//if i convert this to an int ahead of time, it doesnt draw properly even if the result is the same.
					//god bless floating point
					const sfloat32 lineWeight = (brushSize+1) * lineFactor;
					
					//refactor: look up better algorithms for line drawing...
					if(!lineData.slope && !lineData.rise)	//horizontal line
						if(truey < lineData.y2 + lineWeight
						&& truey > lineData.y2 - lineWeight
						&& truex < lineData.right + lineWeight
						&& truex > lineData.left - lineWeight)
							std::cout << tileSelected;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
							
					else if(!lineData.slope && !lineData.run)	//vertical line
						if(truex < lineData.x2 + lineWeight 
						&& truex > lineData.x2 - lineWeight
						&& truey < lineData.top + lineWeight
						&& truey > lineData.bottom - lineWeight)
							std::cout << tileSelected;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
					else if(((truey - truex*lineData.slope < lineData.y1 - lineData.x1*lineData.slope + lineWeight
						 && truey - truex*lineData.slope > lineData.y1 - lineData.x1*lineData.slope - lineWeight)
						|| (truex - truey/lineData.slope < lineData.x1 - lineData.y1/lineData.slope + lineWeight
						 && truex - truey/lineData.slope > lineData.x1 - lineData.y1/lineData.slope - lineWeight))
							
						&& truex < lineData.right + lineWeight
						&& truey < lineData.top + lineWeight
						&& truex > lineData.left - lineWeight
						&& truey > lineData.bottom - lineWeight)
							std::cout << tileSelected;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
							
				}
			}
			
			else if(brushType == stampTool){
					static const Tile stampCursor = {"C", tcfgBlack,tcbgWhite};
					static const Tile stampCursorAlt = {"C", tcfgWhite,tcbgRed};
				
				if(stampData.stampMode == 0){	//place first point
					if(!drawx && !drawy)
						std::cout << stampCursor;
					else
						std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
				}
				else if(stampData.stampMode == 1){	//place 2nd point
					
					stampData.b.x = posx;
					stampData.b.y = posy;
					
					if(posx < stampData.a.x){
						stampData.right = stampData.a.x;
						stampData.left = posx;
					}else{
						stampData.right = posx;
						stampData.left = stampData.a.x;
					}
					
					if(posy < stampData.a.y){
						stampData.bottom = stampData.a.y;
						stampData.top = posy;
					}else{
						stampData.bottom = posy;
						stampData.top = stampData.a.y;
					}
					
					//make this more readable somehow? its gonna be a mess no matter what
					if(((truey == stampData.top 
					|| truey == stampData.bottom) 
						&& (truex >= stampData.left 
						&& truex <= stampData.right)
					|| ((truex == stampData.left
					|| truex == stampData.right)
						&& truey >= stampData.top
						&& truey <= stampData.bottom))){
							
						if((truex & 1) ^ (truey & 1))
							std::cout << stampCursor;
						else
							std::cout << stampCursorAlt;
					}else
						std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						
				}else if(stampData.stampMode == 2){	//paste mode
					if(!stampData.clipboard){
						std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
					}else{
						const sint32 xoffset = -(stampData.clipWidth>>1)-1;
						const sint32 yoffset = -(stampData.clipHeight>>1)-1;
						
						if(drawy > yoffset
						&& drawx > xoffset
						&& drawy < -yoffset - !(stampData.clipHeight & 1)
						&& drawx < -xoffset - !(stampData.clipWidth & 1)){
							std::cout << ptr2D(stampData.clipboard, stampData.clipWidth, drawx-xoffset-1, drawy-yoffset-1);
						}else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
					}
				}
			}
			
			//MUST be below non-looping tools, otherwise they wont get drawn outside brush area
            else if (!drawBrush || drawy>brushSize || drawx>brushSize || drawx<-brushSize || drawy<-brushSize)
					std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
			
            else if(truex<mapSizeX && truey<mapSizeY)
                switch(brushType){
                    default:
					case noBrush:{
						std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						break;
					}
                    case squareShape:{
                        std::cout << tileSelected;
                        break;
                    }
                    case diamondShape:{
                        if(!(drawx<=(drawy-brushSize-1) || drawx>=(drawy+brushSize+1) || -drawx<=(drawy-brushSize-1) || -drawx>=(drawy+brushSize+1)\
                          || drawy<=(drawx-brushSize-1) || drawy>=(drawx+brushSize+1) || -drawy<=(drawx-brushSize-1) || -drawy>=(drawx+brushSize+1)))
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
                    case diamondHollowShape:{
                        if((drawx==(drawy-brushSize) || drawx==(drawy+brushSize) || -drawx==(drawy-brushSize) || -drawx==(drawy+brushSize)\
                          || drawy==(drawx-brushSize) || drawy==(drawx+brushSize) || -drawy==(drawx-brushSize) || -drawy==(drawx+brushSize)))
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
                    case circleShape:{
                        if (sqrtl((drawx*drawx) + (drawy*drawy))/circleFactor < brushSize+1) //circleFactor refines circle shape.
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
                    case circleHollowShape:{
                        static sfloat32 pythagFactor;
                        pythagFactor = sqrtl((drawx*drawx) + (drawy*drawy))/circleFactor;
						
                        if (pythagFactor < brushSize+1 && pythagFactor > brushSize-circleFactor)	//circleFactor refines circle shape.
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
					case squareHollowShape:{
                        if((drawx==brushSize) || (drawy==brushSize) || (drawx==-brushSize) || (drawy==-brushSize))
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
                    case lineVertShape:{
                        if(!drawx)
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
                    case lineHoriShape:{
                        if(!drawy)
                            std::cout << tileSelected;
                        else
                            std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
                        break;
                    }
					case bucketFill:{
						
						if(!fillBucket(inputMap,posx,posy,false) || !fillArea){	//generate fill map based on brush size
							clearScreen;
							out(lineBreakFormat,"ERROR: Failed to generate bucket map!");
							pause;
							
							if(fillArea)
								delete fillArea;
							fillArea = nullptr;
							
							brushType = squareShape;
							break;
						}
						
						if(ptr2D(fillArea,((2*brushSize)+1),brushSize-drawx,brushSize-drawy) == 3)
							std::cout << tileSelected;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						break;
					}
					case bucketFillEx:{
						
						if(!fillBucket(inputMap,posx,posy,true) || !fillArea){	//generate fill map based on brush size
							clearScreen;
							out(std::cerr, lineBreakFormat,"ERROR: Failed to generate bucket map!");
							pause;
							
							if(fillArea)
								delete fillArea;
							fillArea = nullptr;
							
							brushType = squareShape;
							break;
						}
						
						if(ptr2D(fillArea,((2*brushSize)+1),brushSize-drawx,brushSize-drawy) == 3)
							std::cout << tileSelected;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						break;
					}
					case tilePicker:{
						static const Tile picker = {"☼",tcfgWhite,tcbgBlack};
						
						if(!drawx && !drawy)
							std::cout << picker;
						else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						
						break;
					}
					case textMode:{
						brushResizeBounds(textMode);
						static const Tile textCursor("T",tcfgBlack,tcbgWhite);
						
						if(textBuffer.size() > 0 && drawx < textBuffer.size() && drawx > -1 && !drawy){
							textBuffer[drawx].copyFormatting(tileSelected);
							std::cout << textBuffer[drawx];
						}else if(!textBuffer.size()){
							std::cout << textCursor;
						}else
							std::cout << ptr2D(inputMap,mapSizeX,truex,truey);
						break;
					}
                }
        }
    }
	endLine;
    if(showUI && drawBrush){
		static const Tile gui_rightArrow = {"▶",tcfgRed,tfDefault,tfDefault};
		static const Tile gui_leftArrow = {"◀",tcfgRed,tfDefault,tfDefault};
		static Tile tileSample;
		
		if(brushType == textMode){
			tileSample = "A";
			tileSample.copyFormatting(tileSelected);	//refactor: combine the ternary below with this?
		}else
			tileSample = tileSelected;
		/*
		|Controls: ~	X:## Y:## 	Screen Width: ### 	Screen Height: ### 					
		|Brush: Cardinal Line █		Brush Size: ## 	Brush Mode: Horizontal	
		|FG:█ █ █ █ █ █ █ █  BG:█ █ █ █ █ █ █ █	Texture:█ █ █ █ █ █ █ █
		
		*/
		out("|Controls: ~\tX:",posx," Y:",posy," \tScreen Width: ",(2*viewXRadius+1)," Height: ",(2*viewYRadius+1),lbr,\
			"|Brush: ",cycleBrush[0]," ",tileSample,"\tBrush Size: ",brushSize," \tBrush Mode: ",cycleBrush[cycleBrushIndex],"      \n|\n",\
			"|FG:",cycleFGcolor[colorsFGindex-2],cycleFGcolor[colorsFGindex-1],"▶",cycleFGcolor[colorsFGindex],"◀",cycleFGcolor[colorsFGindex+1],cycleFGcolor[colorsFGindex+2],\
			"  BG:",cycleFGcolor[colorsBGindex-2],cycleFGcolor[colorsBGindex-1],"▶",cycleFGcolor[colorsBGindex],"◀",cycleFGcolor[colorsBGindex+1],cycleFGcolor[colorsBGindex+2],\
			"  Texture:",cycleTile[cycleIncrement-2],cycleTile[cycleIncrement-1],gui_rightArrow,cycleTile[cycleIncrement],gui_leftArrow,cycleTile[cycleIncrement+1],cycleTile[cycleIncrement+2],lbr);
		
    }else if(showUI)
            out("|Controls: ~\tX:",posx," Y:",posy," \tScreen Width: ",viewXRadius," Height: ",viewYRadius,lbr,"                                                                  \n                                                                                                        \n                                                                                                        \n                                                                                                        \n");
    else
        out("                                                                                                        \n                                                                                                        \n                                                                                                             ");
		//refactor: use a forloop to fill the screenwidth with blank space?
    return;
}
//draws the current screen using the player position x and y. if neither ShowUI or drawBrush are included, it's assumed that neither are necessary.
 void drawScreen(Tile* &inputMap, const uint32 posx, const uint32 posy){
    return drawScreen(inputMap, posx, posy, false, false);
}
/*
draws the current screen using the player position x and y.
brush controls and display are disabled if showUI is false.
*/
 void drawScreen(Tile*& inputMap, const uint32 posx, const uint32 posy, const bool showUI){
    return drawScreen(inputMap, posx, posy, showUI, showUI);
}


//controller for viewing the map. true enables drawing and applying the brush. false just enables camera controls.
void paintControl(const bool enablePaintControls){
	bool showBrush = true;
	keyt playerInput = 0;
	
    while(playerInput != tabKey){
		drawScreen(map,x,y,enablePaintControls,enablePaintControls);
        getKey(false, playerInput);
		
        switch(playerInput){
            default:
                continue;
			case oneKey:
			case twoKey:
			case threeKey:
			case fourKey:
			case fiveKey:
			case sixKey:
			case sevenKey:
			case eightKey:
			case nineKey:{
				selectBrushType(playerInput);
				break;
			}
            case carRetKey:
            case enterKey:{
                if(enablePaintControls){
                    applyBrush(tileSelected,brushSize,brushType,x,y);
				}
                continue;
			}
            case downKey:{	//refactor: consider making bounds signed to remove an else branch
				if(y<mapSizeY-1)
					y++;
				else
					y = mapSizeY-1;
                break;
            }
            case upKey:{
				if(y>=mapSizeY)
					y = 0;
				else if(y)
					y--;
                break;
            }
            case leftKey:{
				if(x>=mapSizeX)
					x = 0;
				else if(x)
					x--;
                break;
            }
            case rightKey:{
				if(x<mapSizeX-1)
					x++;
				else
					x = mapSizeX-1;
                break;
            }
			case '=':
            case '+':{
                if(enablePaintControls){
					
					if(brushType == textMode){
						brushSize = textBuffer.size();
						break;
					}else
						brushSize++;
					
					if(mapSizeY>mapSizeX && brushSize > (mapSizeY >>1))
						brushSize = mapSizeY >>1;
					else if(brushSize > (mapSizeX >>1))
						brushSize = mapSizeX >>1;
					
					brushResizeBounds(brushType);
                }
                break;
            }
			case '_':
            case '-':{
                if(enablePaintControls){
                    if(!brushSize)
                        break;
					
					if(brushType == textMode){
						brushSize = textBuffer.size();
						break;
					}else
						brushSize--;
					
					if(mapSizeY>mapSizeX && brushSize > (mapSizeY >>1))
						brushSize = mapSizeY >>1;
					else if(brushSize > (mapSizeX >>1))
						brushSize = mapSizeX >>1;
					
					brushResizeBounds(brushType);
                }
                break;
            }
			case '|':
            case '\\':{
                if(enablePaintControls)
                    showBrush = !showBrush;
                else
                    showBrush = false;
                clearScreen;
                break;
            }
			//all tools must handle this input
            case '/':
            case questionKey:{
                switch (brushType){
                    default:
					case noBrush:{
						break;
					}
                    case squareShape:{
                        selectBrushType(squareHollowShape);
						cycleBrushIndex = 2;
                        break;
                    }
                    case lineHoriShape:{
                        selectBrushType(lineVertShape);
						cycleBrushIndex = 1;
						brushResizeBounds(brushType);
						break;
                    }
                    case lineVertShape:{
                        selectBrushType(lineHoriShape);
						cycleBrushIndex = 2;
						brushResizeBounds(brushType);
                        break;
                    }
					case squareHollowShape:{
                        selectBrushType(squareShape);
						cycleBrushIndex = 1;
                        break;
                    }
                    case diamondShape:{
                        selectBrushType(diamondHollowShape);
						cycleBrushIndex = 2;
                        break;
                    }
                    case diamondHollowShape:{
                        selectBrushType(diamondShape);
						cycleBrushIndex = 1;
                        break;
                    }
                    case circleShape:{
                        selectBrushType(circleHollowShape);
						cycleBrushIndex = 2;
                        break;
                    }
                    case circleHollowShape:{
                        selectBrushType(circleShape);
						cycleBrushIndex = 1;
                        break;
                    }
					case bucketFill:{
						selectBrushType(bucketFillEx);
						cycleBrushIndex = 2;
						fillBucket(map,x,y,true);
						break;
					}
					case bucketFillEx:{
						selectBrushType(bucketFill);
						cycleBrushIndex = 1;
						fillBucket(map,x,y,false);
						break;
					}
					case tilePicker:{
						selectBrushType(brushPrev);
						brushPrev = noBrush;
						brushResizeBounds(brushType);
						break;
					}
					case lineFree:{
						if(lineData.drawLine){
							lineData.drawLine = false;
							cycleBrushIndex = 1;
						}
						break;
					}
					case textMode:{
						cycleBrushIndex = 2;
						Tile input = tileSelected;
						
						keyt k = 0;
						while(k != enterKey){
							drawScreen(map,x,y,enablePaintControls,true);
							getKey(false,k);
							
							if(k == enterKey || k == carRetKey || k == '\n'){
								k = enterKey;
								continue;
							}
							
							switch(k){
								default:{
									if(textBuffer.size() < mapSizeX){
										sanitizePlainText(k, 1, false);
										input = k;
										
										textBuffer.emplace_back(input);
										
										brushSize = textBuffer.size();
										brushResizeBounds(textMode);
									}
									break;
								}
								case downKey:
								case upKey:
								case leftKey:
								case rightKey:
								case escKey:
								case pgdnKey:
								case pgupKey:
								case homeKey:
								case endKey:
								case '\t':
								case insKey:{
									continue;
								}
								case delKey:{
									textBuffer.clear();
									
									brushSize = 0;
									brushResizeBounds(textMode);
									continue;
								}
								case backspaceKey:{
									if(textBuffer.size() > 0){
										textBuffer.resize(textBuffer.size()-1);
										
										brushSize = textBuffer.size();
										brushResizeBounds(textMode);
									}
									continue;
								}
							}
						}
						cycleBrushIndex = 1;
						
						break;
					}
					case stampTool:{
						if(stampData.stampMode != 2 && stampData.clipboard != nullptr){
							stampData.stampMode = 2;
							cycleBrushIndex = 2;
						}else{
							stampData.stampMode = 0;
							cycleBrushIndex = 1;
						}
						break;
					}
                }
                break;
            }
			case '{':
			case '[':{
				tileSelected.foreground = cycleFGcolor[colorsFGindex--].foreground;
				break;
			}
			case '}':
			case ']':{
				tileSelected.foreground = cycleFGcolor[colorsFGindex++].foreground;
				break;
			}
			case ':':
			case ';':{
				tileSelected.background = cycleBGcolor[colorsBGindex--].background;
				break;
			}
			case '\'':
			case '\"':{
				tileSelected.background = cycleBGcolor[colorsBGindex++].background;
				break;
			}
			case pgupKey:{
				viewXRadius++;
				viewYRadius++;
				
				if(viewXRadius<(screenWidth >>1))
					viewXRadius = (screenWidth >>1);
				if(viewYRadius<(screenHeight >>1))
					viewYRadius = (screenHeight >>1);
				
				clearScreen;
				break;
            }
            case pgdnKey:{
				if(viewXRadius<(screenWidth >>1))
					viewXRadius--;
				if(viewYRadius<(screenHeight >>1))
					viewYRadius--;
				
				clearScreen;
				break;
            }
            case insKey:{
				wip; break;
				placeActivator();	//wip
				break;
            }
            case ',':
            case '<':{
                tileSelected = cycleTile[cycleIncrement--];
				
				if(cycleTile[cycleIncrement].isFormatted() == false){
					tileSelected.background = cycleBGcolor[colorsBGindex].background;
					tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;
				}
                break;
            }
            case '.':
            case '>':{
                tileSelected = cycleTile[cycleIncrement++];
				
				if(cycleTile[cycleIncrement].isFormatted() == false){
					tileSelected.background = cycleBGcolor[colorsBGindex].background;
					tileSelected.foreground = cycleFGcolor[colorsFGindex].foreground;    
				}
                break;
            }
            case tabKey:{
                return;
                break;
            }
			case '~':
			case '`':{
				clearScreen;
				terminalApplyFormat(std::cout,tfBold,tfUnderline);
					out("\nCONTROLS:\n");
				
				terminalApplyFormat(std::cout,tfDefault,tfBold); out("Select brush:");
						out(numberedFormat,"Square Brush","Diamond Brush","Circle Brush","Cardinal Line Brush","Free Line Brush","Bucket Fill","Tile Picker","Text Tool","Copy/Paste Tool");
					
				terminalApplyFormat(std::cout,tfBold);
					out("\nInsert\t\t"); terminalClearFormat();   out("place Activator (WIP).");
				
				terminalApplyFormat(std::cout,tfBold,tcbgDrkGrey);
					 terminalApplyFormat(std::cout,tcbgDrkGrey); out("\n<>\t\t"); out("change brush texture."); terminalClearFormat();
				
				terminalApplyFormat(std::cout,tfBold);
					out("\n/\t\t"); terminalClearFormat();  out("change brush mode.");
				
				terminalApplyFormat(std::cout,tfBold,tcbgDrkGrey);
					 terminalApplyFormat(std::cout,tcbgDrkGrey); out("\n[]\t\t"); out("change brush foreground color."); terminalClearFormat();
				
				terminalApplyFormat(std::cout,tfBold);
					out("\n;\"\t\t"); terminalClearFormat();  out("change brush background color.");
				
				terminalApplyFormat(std::cout,tfBold,tcbgDrkGrey);
					terminalApplyFormat(std::cout,tcbgDrkGrey); out("\n+-\t\t"); out("change brush size."); terminalClearFormat();
				
				terminalApplyFormat(std::cout,tfBold);
					out("\npgUp/pgDn\t"); terminalClearFormat();  out("change screen size.");
				
				terminalApplyFormat(std::cout,tfBold,tcbgDrkGrey);
					terminalApplyFormat(std::cout,tcbgDrkGrey); out("\nTAB\t\t"); out("view main menu."); terminalClearFormat();
				
				terminalApplyFormat(std::cout,tfBold);
					out("\n\\\t\t"); terminalClearFormat();  out("hide brush.");
				
				terminalApplyFormat(std::cout,tfBold,tcbgDrkGrey);
					terminalApplyFormat(std::cout,tcbgDrkGrey); out("\nCtrl+Z\t\t"); out("Undo"); terminalClearFormat();
				
				terminalApplyFormat(std::cout,tfBold);
					out("\nCtrl+Y\t\t"); terminalClearFormat();  out("Redo");
					
					terminalApplyFormat(std::cout,tfBold,tfUnderline); out("\n\nText Tool Usage:\n"); terminalClearFormat();
					out(" Press / to change to typing mode, press ENTER to return to placement mode.\n In typing mode, pressing DELETE will clear the string, and BACKSPACE works as expected.\n Pressing ENTER outside typing mode will paste the text.");
				
				endLine;
				pause;
				break;
			}
			case redoKey:{
				if(availableRedos){
					availableUndos++;
					availableRedos--;
					undoSaveTarget++;
					
					if(!historyLoad()){	//resets variables if map loading fails; helps ensure the indices stay in step with each other.
						availableRedos = 0;
						availableUndos = 0;
						undoSaveTarget = undoSaveTarget.maxValue;
					}else{
						brushResizeBounds(brushType);	//needed for canvas operations
					}
				}
				break;
			}
			case undoKey:{
				if(availableUndos){
					availableUndos--;
					availableRedos++;
					undoSaveTarget--;
					
					if(!historyLoad()){	//resets variables if map loading fails; helps ensure the indices stay in step with each other.
						availableUndos = 0;
						availableRedos = 0;
						undoSaveTarget = undoSaveTarget.maxValue;
					}else{
						brushResizeBounds(brushType);	//needed for canvas operations
					}
				}
				break;
			}
        }
		brushResizeBounds(brushType);
        drawScreen(map,x,y,showBrush,true);  //always shows the UI. only shows full UI if painting is enabled.
    }
    return;
}





