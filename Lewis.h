/**
 * @author Stefan Brandle, Jonathan Geisler
 * @date September, 2004
 *
 * Please type in your name[s] here:
 *
 */

#ifndef LEWIS_H		// Double inclusion protection
#define LEWIS_H

using namespace std;

#include "PlayerV2.h"
#include "Message.h"
#include "defines.h"
#include <fstream>

// DumbPlayer inherits from/extends PlayerV2

class Lewis: public PlayerV2 {
	public:
		Lewis( int boardSize );
		~Lewis();
		void newRound() override;
		Message placeShip(int length) override;
		Message getMove() override;
		void update(Message msg) override;

	private:
		void initializeBoard();
		int lastRow;
		int lastCol;
		int numShipsPlaced;
		char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
		//oppShot keeps track of how many times the opponent has shot at each spot
		int oppShot[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
		//position struct to more easily store row col positions
		struct position {
			int row;
			int col;
			int orientation;
		};
		//placeCorner is used to place one ship near a board corner
		position placeCorner(int length);
		//placeCorner is used to place one ship at the least hit (and available) spot
		position placeHeatMap(int length);
		//getDirection uses the heat map to decide which direction (horizontal or vertical) to place the ship
		int getDirection(int row, int col, int length);
		//shotCount tracks number of opponent hits each round
		int shotCount;
		/* below transplanted from Deng */
		char attackBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
		/* Scan Shoot */
		bool scanShootBoard[MAX_BOARD_SIZE][MAX_BOARD_SIZE]; // to store whether the position is to be shooted by scanShoot()
		Message getScanShoot(); // to shoot according to scanShootBoard, modifying the shot place, and return a Message
		int scanShootCount; // to track how many block are available to be scan shooted
		void initializeScanShootBoard();
		void refreshScanShootBoard();
		/* Target Shoot */
		int sourceRow;
		int sourceCol;
		void toHit(int& row, int& col);
		enum searchDirection { up=0, right=1, down=2, left=3 };
		Message getTargetShoot(int rol, int col);
		bool isValid(int row, int col);
		bool isWater(int row, int col);
		bool isHit(int row, int col);
		int getNextRow(searchDirection dir, int row);
		int getNextCol(searchDirection dir, int col);
		void toNext(searchDirection, int& row, int& col);
		searchDirection getAdjDirection(searchDirection);
		searchDirection getOppDirection(searchDirection);
		/* Learning Ship Placement: Max shoot */
		int roundNum;
		int shipHeatMap[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
		Message getMaxShoot();
};

#endif
