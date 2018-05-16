/**
 * @brief DumbPlayer AI for battleships
 * @file Deng.cpp
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

#include "Deng.h"


/**
 * @brief Constructor that initializes any inter-round data structures.
 * @param boardSize Indication of the size of the board that is in use.
 *
 * The constructor runs when the AI is instantiated (the object gets created)
 * and is responsible for initializing everything that needs to be initialized
 * before any of the rounds happen. The constructor does not get called 
 * before rounds; newRound() gets called before every round.
 */
Deng::Deng( int boardSize )
    :PlayerV2(boardSize)
{
    // Could do any initialization of inter-round data structures here.
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
Deng::~Deng( ) {
// 	ofstream f;
// 	f.open("shipHeatMap.log");
// 	for(int row=0; row<boardSize; row++) {
// 		for(int col=0; col<boardSize; col++) {
// 			f << shipHeatMap[row][col] << " ";
// 		}
// 		f << endl;
// 	}
// 	f.close();
}

/*
 * Private internal function that initializes a MAX_BOARD_SIZE 2D array of char to water.
 */
void Deng::initializeBoard() {
	for(int row=0; row<boardSize; row++) {
		for(int col=0; col<boardSize; col++) {
			this->attackBoard[row][col] = WATER;
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
Message Deng::getMove() {
// 	lastCol++;
// 	if( lastCol >= boardSize ) {
// 		lastCol = 0;
// 		lastRow++;
// 	}
// 	if( lastRow >= boardSize ) {
// 		lastCol = 0;
// 		lastRow = 0;
// 	}
// 
// 	Message result( SHOT, lastRow, lastCol, "Bang", None, 1 );
// TODO: Store the source so that it does not change
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
void Deng::newRound() {
    /* DumbPlayer is too simple to do any inter-round learning. Smarter players 
     * reinitialize any round-specific data structures here.
     */
    this->lastRow = 0;
    this->lastCol = -1;
    this->numShipsPlaced = 0;

    this->initializeBoard();
	this->initializeScanShootBoard();

	this->sourceRow = -1; // no source at beginning
	this->sourceCol = -1;

	this->roundNum++;
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
Message Deng::placeShip(int length) {
    char shipName[10];
    // Create ship names each time called: Ship0, Ship1, Ship2, ...
    snprintf(shipName, sizeof shipName, "Ship%d", numShipsPlaced);

    // parameters = mesg type (PLACE_SHIP), row, col, a string, direction (Horizontal/Vertical)
    Message response( PLACE_SHIP, numShipsPlaced, 0, shipName, Horizontal, length );
    numShipsPlaced++;

    return response;
}

/**
 * @brief Updates the AI with the results of its shots and where the opponent is shooting.
 * @param msg Message specifying what happened + row/col as appropriate.
 */
void Deng::update(Message msg) {
    switch(msg.getMessageType()) {
	case HIT:
		shipHeatMap[msg.getRow()][msg.getCol()]++;
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
	    break;
	case KILL:
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
	    break;
	case MISS:
	    attackBoard[msg.getRow()][msg.getCol()] = msg.getMessageType();
	    break;
	case WIN:
	    break;
	case LOSE:
	    break;
	case TIE:
	    break;
	case OPPONENT_SHOT:
	    // TODO: get rid of the cout, but replace in your AI with code that does something
	    // useful with the information about where the opponent is shooting.
	    //cout << gotoRowCol(20, 30) << "DumbPl: opponent shot at "<< msg.getRow() << ", " << msg.getCol() << flush;
	    break;
    }
}


/**
 * @return a Message indicating which place to attack, according to scanShootBoard
 */
Message Deng::getScanShoot(){ // to shoot according to scanShootBoard, modifying the shot place, and return a Message
	int scanID = rand() % scanShootCount; // generate a random number between 0 and scanShootCount-1
	for (int i=0; i<boardSize; i++)
		for (int j=0; j<boardSize; j++)
		{
			if (scanShootBoard[i][j] == true) { // if found a spot to be shot
				if (scanID == 0) { // if reaching the ID, return this spot
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
void Deng::initializeScanShootBoard() {
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
void Deng::refreshScanShootBoard() {
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
void Deng::toHit(int& row, int& col)
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
Message Deng::getTargetShoot(int row, int col)
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


bool Deng::isValid(int row, int col)
{
	return row<boardSize &&
		row>=0 &&
		col<boardSize &&
		col>=0;
}
bool Deng::isWater(int row, int col)
{
	return isValid(row,col) &&
		this->attackBoard[row][col] == WATER;
}
bool Deng::isHit(int row, int col)
{
	return isValid(row,col) &&
		this->attackBoard[row][col] == HIT;
}

void Deng::toNext(searchDirection dir, int& row, int& col)
{
	row = getNextRow(dir, row);
	col = getNextCol(dir, col);
}

int Deng::getNextRow(searchDirection dir, int row)
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

int Deng::getNextCol(searchDirection dir, int col)
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

Deng::searchDirection Deng::getAdjDirection(searchDirection d)
{
	return searchDirection (d+1>=4 ? d-3 : d+1);
}
Deng::searchDirection Deng::getOppDirection(searchDirection d)
{
	return searchDirection (d+2>=4 ? d-2 : d+2);
}

/** @brief Get a most shot position from the scanShootBoard
    Intended to be used after shipHeatMap is matured ( after 10 rounds maybe )
    as a replacement of getScanShoot()

    @return a Message object with the most shot position, return 0,0 if position not found
*/
Message Deng::getMaxShoot() {
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
