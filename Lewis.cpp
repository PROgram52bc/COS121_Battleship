/**
 * @brief Lewis AI for battleships
 * @file Lewis.cpp
 * @author Stefan Brandle, Jonathan Geisler
 * @date September, 2004 Updated 2015 for multi-round play.
 *
 * This Battleships AI is very simple and does nothing beyond playing
 * a legal game. However, that makes it a good starting point for writing
 * a more sophisticated AI.
 *
 * The constructor
 */

#include <iostream>
#include <cstdio>
#include <fstream>
#include <vector>
#include "Lewis.h"


/**
 * @brief Constructor that initializes any inter-round data structures.
 * @param boardSize Indication of the size of the board that is in use.
 *
 * The constructor runs when the AI is instantiated (the object gets created)
 * and is responsible for initializing everything that needs to be initialized
 * before any of the rounds happen. The constructor does not get called 
 * before rounds; newRound() gets called before every round.
 */
Lewis::Lewis( int boardSize )
	:PlayerV2(boardSize)
{
    // Could do any initialization of inter-round data structures here.
	/* Below transplanted from Deng.cpp */
	for(int row=0; row<boardSize; row++) {
		for(int col=0; col<boardSize; col++) {
			this->shipHeatMap[row][col] = 0;
		}
	}
	this -> roundNum = 0;
}

/**
 * @brief Destructor placeholder.
 * If your code does anything that requires cleanup when the object is
 * destroyed, do it here in the destructor.
 */
Lewis::~Lewis( ) {}

/*
 * Private internal function that initializes a MAX_BOARD_SIZE 2D array of char to water.
 */
void Lewis::initializeBoard() {
	for(int row=0; row<boardSize; row++) {
		for(int col=0; col<boardSize; col++) {
			//printf("setting up row %d col %d for boardSize %d\n",row,col,boardSize);
			this->attackBoard[row][col] = WATER;
			this->board[row][col] = WATER;
		}
	}
}
/**
 * @brief Specifies the AI's shot choice and returns the information to the caller.
 * @return Message The most important parts of the returned message are 
 * the row and column values. 
 *
 * See the Message class documentation for more information on the 
 * Message constructor.
 */
Message Lewis::getMove() {	
	/* Below transplanted from Deng */
 	Message result( SHOT, -1, -1, "", None, 1 ); // default result
	if (!isHit(sourceRow,sourceCol)) // if the source is no longer a HIT, reset source to none
	{
		sourceRow = -1;
		sourceCol = -1;
	}
	if (sourceRow == -1 && sourceCol == -1) // if no existing source
		toHit(sourceRow, sourceCol); // try setting the source to existing HIT
	if (sourceRow == -1 && sourceCol == -1) // if no source, scan shoot or max shoot
	{
		refreshScanShootBoard();
		if (roundNum>=10) { // if already 10+ rounds 
			result = getMaxShoot();
		}
		else { // if not yet 10 rounds
			result = this->getScanShoot();
		}
	}
	else // if found a HIT
	{
		result = this->getTargetShoot(sourceRow, sourceCol);
	}

	lastRow = result.getRow();
	lastCol = result.getCol();
	return result;
}
/**
 * @brief Tells the AI that a new round is beginning.
 * The AI show reinitialize any intra-round data structures.
 */
void Lewis::newRound() {
    /* DumbPlayer is too simple to do any inter-round learning. Smarter players 
     * reinitialize any round-specific data structures here.
     */
    this->lastRow = 0;
    this->lastCol = -1;
    this->numShipsPlaced = 0;
    this->initializeBoard();
	this->shotCount = 0;
	/* Below transplanted from Deng */
	this->initializeScanShootBoard();

	this->sourceRow = -1; // no source at beginning
	this->sourceCol = -1;
	this->roundNum += 1;
	//Print the heat map to a file
	ofstream logFile;
	logFile.open("heatMap.txt");
	for(int row = 0; row < boardSize; row++){
		for(int col = 0; col < boardSize; col++){
			logFile << "." << oppShot[row][col];
		}
		logFile << std::endl;
	}
	logFile << std::endl;
	logFile.close();
}
/**
 * @brief Gets the AI's ship placement choice. This is then returned to the caller.
 * @param length The length of the ship to be placed.
 * @return Message The most important parts of the returned message are 
 * the direction, row, and column values. 
 *
 * The parameters returned via the message are:
 * 1. the operation: must be PLACE_SHIP 
 * 2. ship top row value
 * 3. ship top col value
 * 4. a string for the ship name
 * 5. direction Horizontal/Vertical (see defines.h)
 * 6. ship length (should match the length passed to placeShip)
 */
Lewis::position Lewis::placeCorner(int length){
	position newPosition;
	int row = 0;
	int col = 0;
	newPosition.row = 0;
	newPosition.col = 0;
	newPosition.orientation = 1;
	//orientation: 0 = up, 1 = right, 2 = down, 3 = left
	int orientation = 1;
	//Decide which of 8 corner placements we want to select
	bool isLegal = true;
	int positionSwitch = (rand() % 10);
	switch(positionSwitch){
		case 0:
			row = 1;
			col = 0;
			orientation = 2;
			break;
		case 1:
			row = 0;
			col = 1;
			orientation = 1;
			break;
		case 2:
			row = 0;
			col = boardSize - 2;
			orientation = 3;
			break;
		case 3:
			row = 1;
			col = boardSize - 1;
			orientation = 2;
			break;
		case 4:
			row = boardSize - 2;
			col = 0;
			orientation = 0;
			break;
		case 5:
			row = boardSize - 1;
			col = 1;
			orientation = 1;
			break;
		case 6:
			row = boardSize - 1;
			col = boardSize - 2;
			orientation = 3;
			break;
		case 7:
			row = boardSize - 2;
			col = boardSize - 1;
			orientation = 0;
			break;
		case 8:
			row = boardSize - 1;
			col = boardSize - 1;
			orientation = 3;
			break;
		case 9:
			row = boardSize - 1;
			col = boardSize - 1;
			orientation = 0;
			break;
		case 10:
			row = boardSize - 1;
			col = 0;
			orientation = 0;
			break;
		case 11:
			row = boardSize - 1;
			col = 0;
			orientation = 1;
			break;
	}
	//based on orientation, check to see if it is a valid move.
	switch(orientation){
		//Up
		case 0:
			for(int i = 0; i < length; i++){
				if(this->board[row-i][col]==SHIP || row-i < 0){
					isLegal = false;
				}
			}
			break;
		//Right
		case 1:
			for(int i = 0; i < length; i++){
				if(this->board[row][col+i]==SHIP || col+i >= boardSize){
					isLegal = false;
				}
			}
			break;
		//Down
		case 2:
			for(int i = 0; i < length; i++){
				if(this->board[row+i][col]==SHIP || row+i >= boardSize){
					isLegal = false;
				}
			}
			break;
		//Left
		case 3:
			for(int i = 0; i < length; i++){
				if(this->board[row][col-i]==SHIP || col-i < 0){
					isLegal = false;
				}
			}
			break;
	}
	if (isLegal==true){
		newPosition.row = row;
		newPosition.col = col;
		//Convert up and left orientations to format readable by placeShip
		switch(orientation){
			case 0:
				newPosition.row = row - (length - 1);
				newPosition.orientation = Vertical;
				break;
			case 1:
				newPosition.orientation = Horizontal;
				break;
			case 2:
				newPosition.orientation = Vertical;
				break;
			case 3:
				newPosition.col = col - (length - 1);
				newPosition.orientation = Horizontal;
				break;
		}
	}else{
		newPosition.row = 0;
		newPosition.col = 0;
		newPosition.orientation = Horizontal;
	}
	return newPosition;

}
int Lewis::getDirection(int row, int col, int length){
	//return 1 for horizontal, 2 for vertical
	long horizontalHeat = 0;
	long verticalHeat = 0;
	int finalAnswer = 0;
	//Accumulate total heat for vertical and horizontal options
	for(int i = 0; i < length; i++){
		horizontalHeat += (oppShot[row][col+i]*oppShot[row][col+i]);
		verticalHeat += (oppShot[row+i][col]*oppShot[row+i][col]);
	}
	if (horizontalHeat < verticalHeat){
		finalAnswer = 2;
	}else {
		finalAnswer = 1;
	}
	return finalAnswer;
}

Lewis::position Lewis::placeHeatMap(int length){
	position newPosition;
	bool canHorizontal = true;
	bool canVertical = true;
	int leastHits = -1;
	//Iterate through every space on oppShot to determine least hit spot
	for(int row = 0; row < boardSize; row++){
		for(int col = 0; col < boardSize; col++){
			//Check to see if the current spot has been hit less
			if (this->oppShot[row][col] < leastHits || leastHits < 0){
				bool isHorizontal = true;
				bool isVertical = true;
				//Check to see if at this spot the ship can be placed horizontally or vertically
				for(int i = 0; i < length; i++){
					if(this->board[row+i][col] == SHIP || row+i>(boardSize-1)){
						isVertical = false;
					}
					if(this->board[row][col+i] == SHIP || col+i>(boardSize-1)){
						isHorizontal = false;
					}
				}
				if(isVertical == true || isHorizontal == true){
					newPosition.row = row;
					newPosition.col = col;
					leastHits = this->oppShot[row][col];
					canHorizontal = isHorizontal;
					canVertical = isVertical;
				}
			}
		}
	}
	//If both canVertical and canHorizontal are true, randomly determine direction
	if (canVertical == true && canHorizontal == true){
		if(getDirection(newPosition.row,newPosition.col,length)==1){
			newPosition.orientation = Horizontal;
		}else{
			newPosition.orientation = Vertical;
		}
		/*if(rand() % 2 == 0){
			newPosition.orientation = Vertical;
		}else{
			newPosition.orientation = Horizontal;
		}*/
	}else if (canVertical == true){
		newPosition.orientation = Vertical;
	}else if (canHorizontal == true){
		newPosition.orientation = Horizontal;
	}
	return newPosition;
}
Message Lewis::placeShip(int length) {


	//Goal: Place ships towards the outside squares where less players will guess
	bool isLegal = false;
	int newRow = 0;
	int newCol = 0;
	bool isVertical = false;
	std::fstream logFile;
	logFile.open("shipMap.txt",ios::app);
	//Randomly select a row and col to start
	if (numShipsPlaced<1){
		position newPos;
		newPos = placeCorner(length);
		newRow = newPos.row;
		newCol = newPos.col;
		if(newPos.orientation == Vertical){
			isVertical = true;
		}else {
			isVertical = false;
		}
	}else if (roundNum < 5){
		while(isLegal == false){
			//Setup a random assignment
			newRow = rand() % boardSize;
			newCol = rand() % boardSize;
			if (rand() % 2 == 0){
				isVertical = true;	
			}else{
				isVertical = false;
			}
			//Check if assignment is valid
			//for vertical ships
			if(isVertical == true){
				if(newRow + length - 1 < boardSize){
					isLegal = true;
				}
				//Check to see if we run into another ship or if we hit a deadzone
				for(int i = 0; i < length; i++){
					if(this->board[newRow+i][newCol] == SHIP){
						isLegal = false;
					}
				}
			}else{//For horizontal ships
				if(newCol + length - 1 < boardSize){
					isLegal = true;
				}
				//Check to see if we run into another ship
				for(int i = 0; i < length; i++){
					if(this->board[newRow][newCol+i] == SHIP){
						isLegal = false;
					}
				}
			}
		}
	}else{
		position newPos;
		newPos = placeHeatMap(length);
		newRow = newPos.row;
		newCol = newPos.col;
		if(newPos.orientation == Vertical){
			isVertical = true;
		}else {
			isVertical = false;
		}
	}
	char shipName[10];
    // Create ship names each time called: Ship0, Ship1, Ship2, ...
    snprintf(shipName, sizeof shipName, "Ship%d", numShipsPlaced);

    // parameters = mesg type (PLACE_SHIP), row, col, a string, direction (Horizontal/Vertical)
    //The next line is the original placement message from DumbPlayer. 
	Direction orientation;
	if (isVertical == true){
		orientation = Vertical;
	}else{
		orientation = Horizontal;
	}
	Message response( PLACE_SHIP, newRow, newCol, shipName, orientation, length );
	//Once we've placed the ship, we need to keep track of its' location internally
	for(int i = 0; i < length; i++){
		if (isVertical==true){
		this->board[newRow+i][newCol] = SHIP;
		}else{
		this->board[newRow][newCol+i] = SHIP;
		}
	}
	//print ships
	for(int row = 0; row < boardSize; row++){
		for(int col = 0; col < boardSize; col++){
			logFile << this->board[row][col];
		}
		logFile << std::endl;
	}
	logFile << std::endl;
	logFile.close();
	numShipsPlaced++;

    return response;
}

/**
 * @brief Updates the AI with the results of its shots and where the opponent is shooting.
 * @param msg Message specifying what happened + row/col as appropriate.
 */
void Lewis::update(Message msg) {
    switch(msg.getMessageType()) {
	case HIT:
		/* Below transplanted from Deng */
		shipHeatMap[msg.getRow()][msg.getCol()]++;
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
		break;
	case KILL:
		/* Below transplanted from Deng */
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
		break;
	case MISS:
	    board[msg.getRow()][msg.getCol()] = msg.getMessageType();
		/* Below transplanted from Deng */
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
	    break;
	case WIN:
	    break;
	case LOSE:
	    break;
	case TIE:
	    break;
	case OPPONENT_SHOT:
	    this->oppShot[msg.getRow()][msg.getCol()] += (boardSize*boardSize)-shotCount;
		this->shotCount++;	
		// TODO: get rid of the cout, but replace in your AI with code that does something
	    // useful with the information about where the opponent is shooting.
	    //cout << gotoRowCol(20, 30) << "DumbPl: opponent shot at "<< msg.getRow() << ", " << msg.getCol() << flush;
	    break;
    }
}
/* Below transplanted from Deng */

/**
 * @return a Message indicating which place to attack, according to scanShootBoard
 */
Message Lewis::getScanShoot(){ // to shoot according to scanShootBoard, modifying the shot place, and return a Message
	int scanID = rand() % scanShootCount; // generate a random number between 0 and scanShootCount-1
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
		{
			if (scanShootBoard[i][j] == true) { // if found a spot to be shot
				if (scanID == 0) { // if reaching the ID, return this spot
					scanShootBoard[i][j] = false;
					scanShootCount--;
					return Message( SHOT, i, j, "Scan Shooting", None, 1);
				}
				else // if not yet reached
				{
					scanID--;
				}
			}
		}
	return Message( SHOT, 0, 0, "RUNOUT", None, 1);
}


/**
 * @brief Initialize scanShootBoard to be shooted in a scanning pattern
 */
void Lewis::initializeScanShootBoard() {
	// initial coordinates to false
//	ofstream f;
//	f.open("scanBoardLog.txt");
	scanShootCount = 0;
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
			scanShootBoard[i][j] = false;

	int row = 0;
	int col = 1;
	int startCol = 2; // the column number to start at each row, vary between 0-2
	while (row < boardSize) // as long as the row number is valid, keep going down
	{
		do {
			scanShootBoard[row][col] = true; // switch the position to true
			scanShootCount++; // increment the counting
			col+=3; // move to the next col position
		}
		while (col < boardSize);  // whenever the col is inside the board
		col=startCol; // go to the startCol
		startCol==2 ? startCol=0 : startCol++; // update startCol
		row++; // update the rol number
	}
	
// debug: print the board
//	for (int i=0; i<boardSize; i++) {
//		for (int j=0; j<boardSize; j++)
//		{
//			f << int(scanShootBoard[i][j]) << " ";
//		}
//		f << endl;
//	}
//	f << "Writing end.\n";
//	f.close();
}

/*
* @brief refresh scanShootBoard according to the condition in the actual board
*/
void Lewis::refreshScanShootBoard() {
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
		{
			if (this->scanShootBoard[i][j] == true &&
					this->attackBoard[i][j] != WATER) // if in scanShootBoard valid, but in board invalid
			{
				this->scanShootBoard[i][j] = false; // update it to invalid
				this->scanShootCount--; // decrement the count
			}
		}
}


/**
 * @brief Modify the parameter to the first found HIT in the board, set to -1, -1 if not found
 */
void Lewis::toHit(int& row, int& col)
{
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
		{
			if (attackBoard[i][j] == HIT)
			{
				row = i;
				col = j;
				return;
			}
		}
	row = -1;
	col = -1;
	return;
}


/**
 * @brief return a place to shoot, given the source HIT
 * @param row of the source HIT
 * @param col of the source HIT
 *
 * Requirements:
 * 1. The given coordinate is a HIT ( not a KILL )
 * 2. The gameboard is in a valid state ( since the ship at source HIT is not killed, a possible shot will be found if searching through all four directions.
 */
Message Lewis::getTargetShoot(int row, int col)
{
	searchDirection currDir = up; // initial search direction
	bool directionsValidity[4] = {true, true, true, true}; // set all directions to valid
	int currRow = row;
	int currCol = col;
	while(1)
	{
		if (directionsValidity[currDir] == false) // if the current direction is invalid
		{
			currDir = getAdjDirection(currDir); // go to adjacent direction
			continue;
		}
		if (isWater(getNextRow(currDir,currRow),getNextCol(currDir,currCol))) // if the next coord is water, shoot!
		{
			toNext(currDir,currRow,currCol); // go to next coord along direction
			return Message( SHOT, currRow, currCol, "Target Shooting", None, 1);
		}
		else if (isHit(getNextRow(currDir,currRow),getNextCol(currDir,currCol))) // if next coord is shot, go again to the next
		{
			toNext(currDir,currRow,currCol); // go to next coord along direction
			continue;
		}
		else // if neither water nor shot ( meaning the same ship can't extend to that spot )
		{
			directionsValidity[currDir] = false; // set current direction to invalid
			if (currRow == row && currCol == col) // if this is the source HIT, search priority is adjacent direction
			{
				currDir = getAdjDirection(currDir);
				continue;
			}
			else // if there are shots after source already, search priority is opposite
			{
				currDir = getOppDirection(currDir);
				currRow = row;
				currCol = col;
				continue;
			}
		}
	}
}


bool Lewis::isValid(int row, int col)
{
	return row<boardSize &&
		row>=0 &&
		col<boardSize &&
		col>=0;
}
bool Lewis::isWater(int row, int col)
{
	return isValid(row,col) &&
		this->attackBoard[row][col] == WATER;
}
bool Lewis::isHit(int row, int col)
{
	return isValid(row,col) &&
		this->attackBoard[row][col] == HIT;
}

void Lewis::toNext(searchDirection dir, int& row, int& col)
{
	row = getNextRow(dir, row);
	col = getNextCol(dir, col);
}

int Lewis::getNextRow(searchDirection dir, int row)
{
	switch (dir) {
		case up:
			return row-1;
		case down:
			return row+1;
		default:
			return row;
	}
}

int Lewis::getNextCol(searchDirection dir, int col)
{
	switch (dir) {
		case left:
			return col-1;
		case right:
			return col+1;
		default:
			return col;
	}
}

Lewis::searchDirection Lewis::getAdjDirection(searchDirection d)
{
	return searchDirection (d+1>=4 ? d-3 : d+1);
}
Lewis::searchDirection Lewis::getOppDirection(searchDirection d)
{
	return searchDirection (d+2>=4 ? d-2 : d+2);
}

/** @brief Get a most shot position from the scanShootBoard
    Intended to be used after shipHeatMap is matured ( after 10 rounds maybe )
    as a replacement of getScanShoot()

    @return a Message object with the most shot position, return 0,0 if position not found
*/
Message Lewis::getMaxShoot() {
	int maxRow = 0;
	int maxCol = 0;
	int maxHeat = -1;
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
		{
			if (scanShootBoard[i][j] == true) // if spot available
			{
				if (shipHeatMap[i][j] > maxHeat) // if that position has greater heat
				{
					maxRow = i;
					maxCol = j;
					maxHeat = shipHeatMap[i][j];
				}
			}
		}
	return Message( SHOT, maxRow, maxCol, "Max Shooting", None, 1);
}
