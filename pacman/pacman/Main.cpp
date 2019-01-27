#include "GLUT.H"
#include <math.h>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <set>


#include "Point2D.h"
#include "pacmanPawn.h"
#include "Monster.h"
#include "ComparePointsByDist.h"


using namespace std;

const int MSIZE = 600;
const int W = MSIZE; // window width
const int H = MSIZE; // window height

const int SPACE = 1;
const int WALL = 2;
const int COIN = 3;
const int UNREACHABLE = 4;
const int CENTER = 5;
const int TARGET_COIN = 6;
const int PACMAN_VISITED = 7;

/*******************************************************************/
/*        ALWAYS WORK WITH MULTIPLICATION OF CELL_SIZE IN THE MAZE
OTHERWISE IT WON'T DRAW IT!!!!!!!!!!

And The origin : x -> from left to right,
y -> from bottom to top.
*/
/*******************************************************************/


const double SQSIZE = 2.0 / MSIZE;
int maze[MSIZE][MSIZE];
const int CELL_SIZE = 5;
const int SPACE_SIZE = CELL_SIZE * 5;
const double step = 0.1;
int stepCounter = 0;

enum wallSide { Top, Bottom, Left, Right };    //the position of the wall

											   //gray queue for BFS algorithm
vector<Point2D*> gray_coins;    //for bfs ditrebution of the coins
vector<Point2D*> coins_locations;

bool coinsBfs = false;
Point2D* parent_forCoins[MSIZE][MSIZE];
int coinCounter = 0;
const int COINS_STEP = 5 * CELL_SIZE;


vector<Point2D*> gray_target;
//Point2D* startPoint, *targetPoint;

int startPointForCoins_x = 3 * CELL_SIZE;
int startPointForCoins_y = 3 * CELL_SIZE;

const int MAX_COINS = pow((MSIZE - 2 * startPointForCoins_x) / COINS_STEP, 2);


Point2D* pacmanStartPoint = new Point2D(startPointForCoins_x, startPointForCoins_y);
PacmanPawn pacman(pacmanStartPoint, 2 * CELL_SIZE);
Point2D* nearestCoinToPacman;
vector<Point2D*> nearestCoinsVector;
Point2D* lastCoin;

bool startPacmanCoinSearch = false;
priority_queue <Point2D*, vector<Point2D*>, ComparePointsByDist> pacmanRoute;
//vector<Point2D*> pacmanPath;
Point2D* pacmanPathParents[MSIZE][MSIZE];
Point2D* lastPointForPath;
priority_queue <Point2D*, vector<Point2D*>, ComparePointsByDist> pacmanPath;
bool checkVertical = true;
bool checkHorizontal = true;
int rowToCheck;
int colToCheck;

Monster monster1(vector<double>({ 0, 1, 1 }), new Point2D(300, 300), 2 * CELL_SIZE);



void thickenWallInMaze(wallSide side, int row = -1, int col = -1)
{

	for (int j = 0; j < CELL_SIZE; j++)
	{
		switch (side)
		{
		case Top:
			maze[(row == -1) ? j : (row - j)][col] = WALL;
			break;
		case Bottom:
			maze[(row == -1) ? j : (row + j)][col] = WALL;
			break;
		case Left:
			maze[row][(col == -1) ? j : (col + j)] = WALL;
			break;
		case Right:
			maze[row][(col == -1) ? j : (col - j)] = WALL;
			break;
		}
	}
}

void setUnreachableZone(int bottomRow, int topRow, int leftCol, int rightCol, int unreachableParam = UNREACHABLE)
{
	for (int i = bottomRow; i < topRow; i++)
		for (int j = leftCol; j < rightCol; j++)
			maze[i][j] = unreachableParam;

}

void setupPerimeter()
{
	int i, k;

	//calclate the second y coordinate in maze of the horizontal line from bottom up
	int middleDoubleSpaceCells = (((MSIZE / 3) - SPACE_SIZE) / 2);
	middleDoubleSpaceCells -= middleDoubleSpaceCells % CELL_SIZE;    //need to be multiplication of CEEL_SIZE

	for (i = 0; i < MSIZE; i++)
	{
		if (i < MSIZE / 3 || i >(MSIZE / 3) * 2)    //only first third and last third are walls
		{
			thickenWallInMaze(Left, i); // left walls
			thickenWallInMaze(Right, i, MSIZE - CELL_SIZE); // right bound walls of the maze
		}
		else
		{    //draw the left and right middle sections first lines
			if ((i == MSIZE / 3) || (i == 2 * MSIZE / 3))
			{
				//for the lower edge , it's positive, for the top edge is negative
				middleDoubleSpaceCells = (i == MSIZE / 3) ? middleDoubleSpaceCells : (middleDoubleSpaceCells * (-1));

				for (k = 0; k <= MSIZE / 6; k++)
				{
					if (middleDoubleSpaceCells < 0)
					{
						//top bounds of the middle section ,left and right sides
						thickenWallInMaze(Bottom, i, k); //top left
						thickenWallInMaze(Bottom, i, MSIZE - k - 1); //top right
						thickenWallInMaze(Bottom, i + middleDoubleSpaceCells, k); //bottom left
						thickenWallInMaze(Bottom, i + middleDoubleSpaceCells, MSIZE - k - 1); //bottom right

					}
					else
					{
						//bottom bounds of the middle section ,left and right sides
						thickenWallInMaze(Top, i, k); //bottom  left
						thickenWallInMaze(Top, i, MSIZE - k - 1); //bottom right
						thickenWallInMaze(Top, i + middleDoubleSpaceCells, k); //top left
						thickenWallInMaze(Top, i + middleDoubleSpaceCells, MSIZE - k - 1); //top right
					}
				}
				if (i == MSIZE / 3)
				{
					setUnreachableZone(i + 1, i + middleDoubleSpaceCells, 0, MSIZE / 6);
					setUnreachableZone(i + 1, i + middleDoubleSpaceCells, MSIZE - (MSIZE / 6), MSIZE);
				}
				else
				{
					setUnreachableZone(i + middleDoubleSpaceCells + 1, i, 0, MSIZE / 6);
					setUnreachableZone(i + middleDoubleSpaceCells + 1, i, MSIZE - (MSIZE / 6) + 1, MSIZE);
				}


			}//set the bottom and top vertical walls in the middle section for left and right
			else if ((i > MSIZE / 3) && (i < MSIZE / 3 + middleDoubleSpaceCells + 2 * CELL_SIZE))
			{
				thickenWallInMaze(Left, i - CELL_SIZE, MSIZE / 6); // left
				thickenWallInMaze(Right, i - CELL_SIZE, MSIZE - MSIZE / 6); // right
			}

			else if ((i < 2 * MSIZE / 3) && (i > 2 * MSIZE / 3 - middleDoubleSpaceCells - 2 * CELL_SIZE))//set the top vertical walls in the middle section for left and right
			{
				thickenWallInMaze(Left, i + CELL_SIZE, MSIZE / 6); // left
				thickenWallInMaze(Right, i + CELL_SIZE, MSIZE - MSIZE / 6); // right
			}
		}
		thickenWallInMaze(Bottom, -1, i); //bottom wall
		thickenWallInMaze(Top, MSIZE - CELL_SIZE, i); // top bound walls of the maze
	}
}

void setupCenterSquare()
{
	int i, j;

	for (i = (MSIZE / 2) - SPACE_SIZE; i < (MSIZE / 2) + SPACE_SIZE + CELL_SIZE; i++)
	{
		//left wall
		thickenWallInMaze(Right, i, (MSIZE / 2) - (3 * SPACE_SIZE));
		//right wall
		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE));

		if ((i == (MSIZE / 2) - SPACE_SIZE) || (i == (MSIZE / 2) + SPACE_SIZE))
		{
			for (j = (MSIZE / 2) - (3 * SPACE_SIZE); j < (MSIZE / 2) + (3 * SPACE_SIZE); j++)
			{
				if (i == (MSIZE / 2) + SPACE_SIZE)
				{
					if (j < (MSIZE / 2) - SPACE_SIZE || j >(MSIZE / 2) + SPACE_SIZE)
						thickenWallInMaze(Bottom, i, j);
				}
				else
					thickenWallInMaze(Top, i, j);
			}
		}
	}

	setUnreachableZone((MSIZE / 2) - SPACE_SIZE + 1, (MSIZE / 2) + SPACE_SIZE,
		(MSIZE / 2) - (3 * SPACE_SIZE) + 1, (MSIZE / 2) + (3 * SPACE_SIZE), CENTER);
}


void setupCenterWalls()
{
	int i, j;
	int middleDoubleSpaceCells = (((MSIZE / 3) - SPACE_SIZE) / 2);
	middleDoubleSpaceCells -= middleDoubleSpaceCells % CELL_SIZE;    //need to be multiplication of CEEL_SIZE

																	 // botom section
	for (i = MSIZE / 3; i < MSIZE / 3 + middleDoubleSpaceCells + CELL_SIZE; i++)
	{
		//left wall
		thickenWallInMaze(Left, i, (MSIZE / 2) - (5 * SPACE_SIZE));
		thickenWallInMaze(Right, i, (MSIZE / 2) - (6 * SPACE_SIZE));

		//right wall
		thickenWallInMaze(Right, i, (MSIZE / 2) + (5 * SPACE_SIZE));
		thickenWallInMaze(Left, i, (MSIZE / 2) + (6 * SPACE_SIZE));
	}

	//bottom left
	setUnreachableZone(MSIZE / 3 + 1, MSIZE / 3 + middleDoubleSpaceCells,
		(MSIZE / 2) - (6 * SPACE_SIZE) + 1, (MSIZE / 2) - (5 * SPACE_SIZE));
	//bottom right
	setUnreachableZone(MSIZE / 3 + 1, MSIZE / 3 + middleDoubleSpaceCells,
		(MSIZE / 2) + (5 * SPACE_SIZE) + 1, (MSIZE / 2) + (6 * SPACE_SIZE));


	// top section
	for (i = 2 * MSIZE / 3 - middleDoubleSpaceCells; i < 2 * MSIZE / 3 + CELL_SIZE; i++)
	{
		//left wall
		thickenWallInMaze(Left, i, (MSIZE / 2) - (5 * SPACE_SIZE));
		thickenWallInMaze(Right, i, (MSIZE / 2) - (6 * SPACE_SIZE));

		//right wall
		thickenWallInMaze(Right, i, (MSIZE / 2) + (5 * SPACE_SIZE));
		thickenWallInMaze(Left, i, (MSIZE / 2) + (6 * SPACE_SIZE));
	}

	//top left
	setUnreachableZone(2 * MSIZE / 3 - middleDoubleSpaceCells + 1, 2 * MSIZE / 3 + CELL_SIZE,
		(MSIZE / 2) - (6 * SPACE_SIZE) + 1, (MSIZE / 2) - (5 * SPACE_SIZE));
	//top right
	setUnreachableZone(2 * MSIZE / 3 - middleDoubleSpaceCells + 1, 2 * MSIZE / 3 + CELL_SIZE,
		(MSIZE / 2) + (5 * SPACE_SIZE) + 1, (MSIZE / 2) + (6 * SPACE_SIZE));


	// left vertical lines
	for (j = (MSIZE / 2) - (5 * SPACE_SIZE); j > (MSIZE / 2) - (6 * SPACE_SIZE) - CELL_SIZE; j--)
	{
		// top
		thickenWallInMaze(Top, 2 * MSIZE / 3 - middleDoubleSpaceCells, j);
		thickenWallInMaze(Bottom, 2 * MSIZE / 3, j);

		// bottom
		thickenWallInMaze(Top, MSIZE / 3, j);
		thickenWallInMaze(Bottom, MSIZE / 3 + middleDoubleSpaceCells, j);
	}

	// right vertical lines
	for (j = (MSIZE / 2) + (5 * SPACE_SIZE); j < (MSIZE / 2) + (6 * SPACE_SIZE) + CELL_SIZE; j++)
	{
		// top
		thickenWallInMaze(Top, 2 * MSIZE / 3 - middleDoubleSpaceCells, j);    //bottom line
		thickenWallInMaze(Bottom, 2 * MSIZE / 3, j);    //top line

														// bottom
		thickenWallInMaze(Top, MSIZE / 3, j);    //bottom line
		thickenWallInMaze(Bottom, MSIZE / 3 + middleDoubleSpaceCells, j);    //top line
	}

}

void setupTopSection()
{
	int i, j;

	// left +
	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE);

		if ((i <= MSIZE - 2 * SPACE_SIZE && i > MSIZE - 3 * SPACE_SIZE - CELL_SIZE) || (i < MSIZE - 4 * SPACE_SIZE && i > MSIZE - 6 * SPACE_SIZE - CELL_SIZE))
			thickenWallInMaze(Right, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE - SPACE_SIZE);
	}

	for (j = (MSIZE / 2) - (5 * SPACE_SIZE); j >= (MSIZE / 2) - (6 * SPACE_SIZE); j--)
	{
		thickenWallInMaze(Bottom, MSIZE - 2 * SPACE_SIZE, j);
		thickenWallInMaze(Top, MSIZE - 6 * SPACE_SIZE, j);
	}

	for (j = 2 * SPACE_SIZE; j < 6 * SPACE_SIZE + CELL_SIZE; j++)
	{
		thickenWallInMaze(Bottom, MSIZE - 3 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, MSIZE - 4 * SPACE_SIZE, j);
	}

	for (i = MSIZE - 3 * SPACE_SIZE - CELL_SIZE; i > MSIZE - 4 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 2 * SPACE_SIZE);
	}

	setUnreachableZone(MSIZE - 6 * SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE,
		(MSIZE / 2) - (6 * SPACE_SIZE) + 1, (MSIZE / 2) - (5 * SPACE_SIZE));

	setUnreachableZone(MSIZE - 4 * SPACE_SIZE + 1, MSIZE - 3 * SPACE_SIZE - CELL_SIZE,
		2 * SPACE_SIZE + 1, 6 * SPACE_SIZE + CELL_SIZE);

	// right +
	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE);

		if ((i <= MSIZE - 2 * SPACE_SIZE && i > MSIZE - 3 * SPACE_SIZE - CELL_SIZE) || (i < MSIZE - 4 * SPACE_SIZE && i > MSIZE - 6 * SPACE_SIZE - CELL_SIZE))
			thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE + SPACE_SIZE);
	}

	for (j = (MSIZE / 2) + (5 * SPACE_SIZE); j <= (MSIZE / 2) + (6 * SPACE_SIZE); j++)
	{
		thickenWallInMaze(Bottom, MSIZE - 2 * SPACE_SIZE, j);
		thickenWallInMaze(Top, MSIZE - 6 * SPACE_SIZE, j);
	}

	for (j = MSIZE - 2 * SPACE_SIZE; j > MSIZE - 6 * SPACE_SIZE - CELL_SIZE; j--)
	{
		thickenWallInMaze(Bottom, MSIZE - 3 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, MSIZE - 4 * SPACE_SIZE, j);
	}

	for (i = MSIZE - 3 * SPACE_SIZE - CELL_SIZE; i > MSIZE - 4 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Left, i, MSIZE - 2 * SPACE_SIZE);
	}

	setUnreachableZone(MSIZE - 6 * SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE,
		(MSIZE / 2) + (5 * SPACE_SIZE) + 1, (MSIZE / 2) + (6 * SPACE_SIZE));

	setUnreachableZone(MSIZE - 4 * SPACE_SIZE + 1, MSIZE - 3 * SPACE_SIZE - CELL_SIZE,
		MSIZE - 6 * SPACE_SIZE - CELL_SIZE + 1, MSIZE - 2 * SPACE_SIZE);

}

void setupBottomSection()
{
	int i, j;

	// right rectangle
	for (j = MSIZE - 2 * SPACE_SIZE; j > MSIZE - 7 * SPACE_SIZE; j--)
	{
		thickenWallInMaze(Bottom, 7 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, 6 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 7 * SPACE_SIZE - CELL_SIZE; i > 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, MSIZE - 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE - 7 * SPACE_SIZE);
	}

	setUnreachableZone(6 * SPACE_SIZE + CELL_SIZE + 1, 7 * SPACE_SIZE - CELL_SIZE, MSIZE - 7 * SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE);


	// draw right +
	for (j = MSIZE - 2 * SPACE_SIZE; j > MSIZE - 7 * SPACE_SIZE; j--)
	{
		if ((j > MSIZE - 4 * SPACE_SIZE && j <= MSIZE - 2 * SPACE_SIZE) || ((j > MSIZE - 7 * SPACE_SIZE && j < MSIZE - 5 * SPACE_SIZE)))
		{
			thickenWallInMaze(Bottom, 4 * SPACE_SIZE - CELL_SIZE, j);
			thickenWallInMaze(Top, 3 * SPACE_SIZE + CELL_SIZE, j);
		}
	}

	for (i = 4 * SPACE_SIZE - CELL_SIZE; i > 3 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, MSIZE - 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE - 7 * SPACE_SIZE);
	}

	for (i = 4 * SPACE_SIZE - CELL_SIZE; i < 5 * SPACE_SIZE; i++)
	{
		thickenWallInMaze(Right, i, MSIZE - 4 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE - 5 * SPACE_SIZE);
	}

	for (i = 2 * SPACE_SIZE; i <= 3 * SPACE_SIZE + CELL_SIZE; i++)
	{
		thickenWallInMaze(Right, i, MSIZE - 4 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE - 5 * SPACE_SIZE);
	}

	for (j = MSIZE - 4 * SPACE_SIZE; j >= MSIZE - 5 * SPACE_SIZE; j--)
	{
		thickenWallInMaze(Bottom, 5 * SPACE_SIZE, j);
		thickenWallInMaze(Top, 2 * SPACE_SIZE, j);
	}

	setUnreachableZone(3 * SPACE_SIZE + CELL_SIZE + 1, 4 * SPACE_SIZE - CELL_SIZE, MSIZE - 7 * SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE);
	setUnreachableZone(2 * SPACE_SIZE + 1, 5 * SPACE_SIZE, MSIZE - 5 * SPACE_SIZE + 1, MSIZE - 4 * SPACE_SIZE);

	// left rectangle
	for (j = 2 * SPACE_SIZE; j < 7 * SPACE_SIZE; j++)
	{
		thickenWallInMaze(Bottom, 4 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, 3 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 4 * SPACE_SIZE - CELL_SIZE; i > 3 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, 7 * SPACE_SIZE);
	}

	setUnreachableZone(3 * SPACE_SIZE + CELL_SIZE + 1, 4 * SPACE_SIZE - CELL_SIZE, 2 * SPACE_SIZE + 1, 7 * SPACE_SIZE);


	// draw left -
	for (j = 2 * SPACE_SIZE; j < 7 * SPACE_SIZE; j++)
	{
		thickenWallInMaze(Bottom, 7 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, 6 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 7 * SPACE_SIZE - CELL_SIZE; i > 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, 7 * SPACE_SIZE);
	}

	setUnreachableZone(6 * SPACE_SIZE + CELL_SIZE + 1, 7 * SPACE_SIZE - CELL_SIZE, 2 * SPACE_SIZE + 1, 7 * SPACE_SIZE);
}

void setupInitials()
{
	int i, j;

	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 8 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE));
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) + CELL_SIZE);

		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE));
		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) - CELL_SIZE);

		for (j = MSIZE / 2 - 3 * SPACE_SIZE; j < MSIZE / 2 + 3 * SPACE_SIZE; j++)
		{
			if (i == MSIZE - 5 * SPACE_SIZE)
			{
				thickenWallInMaze(Bottom, i, j);
				thickenWallInMaze(Bottom, i + CELL_SIZE, j);
				thickenWallInMaze(Bottom, i - CELL_SIZE, j);
			}

		}
	}

	for (i = 2 * SPACE_SIZE; i <= 8 * SPACE_SIZE; i++)
	{
		if (i < 5 * SPACE_SIZE)
		{
			thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) - CELL_SIZE);
			thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) - 2 * CELL_SIZE);
		}
		else if (i < 8 * SPACE_SIZE)
		{
			thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE));
			thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) + CELL_SIZE);
		}

		for (j = MSIZE / 2 - 3 * SPACE_SIZE; j < MSIZE / 2 + 3 * SPACE_SIZE; j++)
		{
			if (i == 2 * SPACE_SIZE || i == 5 * SPACE_SIZE || i == 8 * SPACE_SIZE)
			{
				thickenWallInMaze(Bottom, i, j);
				thickenWallInMaze(Bottom, i - CELL_SIZE, j);
			}
		}
	}
}


void setupMaze()
{
	int i, j;

	//clean up the maze
	for (i = 0; i < MSIZE; i++/*= CELL_SIZE*/)
		for (j = 0; j < MSIZE; j++/*= CELL_SIZE*/)
			maze[i][j] = SPACE;

	setupPerimeter();
	setupCenterSquare();
	setupInitials();
	setupCenterWalls();
	setupBottomSection();
	setupTopSection();
}

/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/

bool checkPointCloseToWallVerticalUp(int row, int col, int range = 2 * CELL_SIZE)
{
	col -= range;

	for (int i = row; i < row + range && i < MSIZE; i++)
		for (int j = col; j < col + 2 * range && j < MSIZE; j++)
			if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
				return false;


	/*for (int i = row; i < row + 2 * range && i < MSIZE; i++)
	if (maze[i][col] == WALL || maze[i][col] == UNREACHABLE || maze[i][col] == CENTER)
	return false;*/

	return true;
}

bool checkPointCloseToWallVertical(int row, int col, int range = 2 * CELL_SIZE)
{
	if (row - range <= 0)
		return false;

	row -= range;
	for (int i = row; i < row + 2 * range && i < MSIZE; i++)
		if (maze[i][col] == WALL || maze[i][col] == UNREACHABLE || maze[i][col] == CENTER)
			return false;

	return true;
}

bool checkPointCloseToWallHorizontalRight(int row, int col, int range = 2 * CELL_SIZE)
{
	row -= range;

	for (int i = row; i < row + 2 * range && i < MSIZE; i++)
		for (int j = col; j < col + range && j < MSIZE; j++)
			if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
				return false;


	/*for (int j = col; j < col + 2 * range && j < MSIZE; j++)
	if (maze[row][j] == WALL || maze[row][j] == UNREACHABLE || maze[row][j] == CENTER)
	return false;*/

	return true;
}

bool checkPointCloseToWallHorizontal(int row, int col, int range = 2 * CELL_SIZE)
{
	if (col - range <= 0)
		return false;

	col -= range;
	for (int j = col; j < col + 2 * range && j < MSIZE; j++)
		if (maze[row][j] == WALL || maze[row][j] == UNREACHABLE || maze[row][j] == CENTER)
			return false;



	return true;
}

bool checkPointCloseToWall(int row, int col)
{
	int range = 2 * CELL_SIZE;

	//don't place coin in that case
	if (row - range <= 0 || col - range <= 0)
		return false;

	row -= range;
	col -= range;

	for (int i = row; i < row + 2 * range && i < MSIZE; i++)
		for (int j = col; j < col + 2 * range && j < MSIZE; j++)
			if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
				return false;


	return true;
}

void storeCurrentPointInParrentArray(int row, int col, Point2D* parentPoint, Point2D* parentArray[][MSIZE], vector<Point2D*> &grayArray)
{
	Point2D* ptAddToGray = new Point2D(col, row);
	parentArray[row][col] = parentPoint;
	grayArray.push_back(ptAddToGray);
	coins_locations.push_back(ptAddToGray);
}

bool setPointAsGray(int mazeRow, int mazeCol, Point2D*& parentPoint, Point2D* parentArray[][MSIZE], vector<Point2D*> &grayVector)
{
	if ((maze[mazeRow][mazeCol] == SPACE) && checkPointCloseToWall(mazeRow, mazeCol))
	{
		maze[mazeRow][mazeCol] = COIN;    //add it to gray
		coinCounter++;
		storeCurrentPointInParrentArray(mazeRow, mazeCol, parentPoint, parentArray, grayVector);
		return true;
	}

	return false;
}

void bfsIteration()
{
	Point2D* pt;
	int mazeRow, mazeCol;

	if (gray_coins.empty() || coinCounter >= MAX_COINS)    //grey is the edges that didn't visited yet
		coinsBfs = false;    //there is no path to the target

	else
	{
		pt = gray_coins[0];    //this will be the parent
		gray_coins.erase(gray_coins.begin());    //deque

		mazeRow = pt->getY();
		mazeCol = pt->getX();

		//paint pt VISITED
		if ((mazeRow == MSIZE - (startPointForCoins_y)) && (mazeCol == MSIZE - (startPointForCoins_x)))    //found target
		{
			storeCurrentPointInParrentArray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			return;
		}

		else
		{
			if (maze[mazeRow][mazeCol] == SPACE)
				maze[mazeRow][mazeCol] = COIN;    //y is i, x is j


			mazeRow = pt->getY() + COINS_STEP;
			setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeRow = pt->getY() - COINS_STEP;
			setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeRow = pt->getY();
			mazeCol = pt->getX() + COINS_STEP;
			setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeCol = pt->getX() - COINS_STEP;
			setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);


		}
	}
}

void setupCoins()
{
	while (coinsBfs)
		bfsIteration();
}

bool checkForWallsBetweenCoinAndPacman(Point2D* nextCoin)
{
	int pacman_x = pacman.getPacmanLocation()->getX();
	int pacman_y = pacman.getPacmanLocation()->getY();

	int nextCoin_x = nextCoin->getX();
	int nextCoin_y = nextCoin->getY();

	if (pacman_y <= nextCoin_y) // coin top
	{
		for (int i = pacman_y; i <= nextCoin_y; i++)
		{
			if (pacman_x <= nextCoin_x) // coin right
			{
				for (int j = pacman_x; j <= nextCoin_x; j++)
					if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
						return false;
			}
			else // coin left
			{
				for (int j = pacman_x; j >= nextCoin_x; j--)
					if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
						return false;
			}
		}
	}
	else // coin top
	{
		for (int i = pacman_y; i >= nextCoin_y; i--)
		{
			if (pacman_x <= nextCoin_x) // coin right
			{
				for (int j = pacman_x; j <= nextCoin_x; j++)
					if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
						return false;
			}
			else  // coin left
			{
				for (int j = pacman_x; j >= nextCoin_x; j--)
					if (maze[i][j] == WALL || maze[i][j] == UNREACHABLE || maze[i][j] == CENTER)
						return false;
			}
		}
	}

	return true;
}

//method wiil return true if found a coin in the vector, otherwise it returns false
bool findNearestCoin()
{
	cout << "in findNearestCoin" << endl;

	if (coins_locations.empty())
	{
		nearestCoinToPacman = nullptr;
		return false;
	}

	if (nearestCoinToPacman)
		lastCoin = new Point2D(*nearestCoinToPacman);

	if (nearestCoinToPacman && maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] == TARGET_COIN)
		maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] = COIN;

	/*if (nearestCoinsVector.empty())
	{*/
	random_shuffle(coins_locations.begin(), coins_locations.end());
		Point2D* pacmanLocation = pacman.getPacmanLocation();
		nearestCoinToPacman = coins_locations[0];
		double minDistance = MSIZE * 2/*pacmanLocation->getDistanceFromTarget(nearestCoinToPacman)*/;
		double tmpDist = 0.0;
		Point2D* tmpCoinLoc;
		int indexToDelete = 0;

		for (int i = 1; i < coins_locations.size(); i++)
		{
			tmpCoinLoc = coins_locations[i];
			tmpDist = pacmanLocation->getDistanceFromPoint(tmpCoinLoc);
			if (!checkForWallsBetweenCoinAndPacman(tmpCoinLoc))
				tmpDist += 20 * SPACE_SIZE;

			if ((tmpDist < minDistance) /*&& ((lastCoin) && (*tmpCoinLoc != *lastCoin))*/)
			{
				minDistance = tmpDist;
				nearestCoinToPacman = tmpCoinLoc;
				indexToDelete = i;
			}
		}

		cout << "the first coin in: x = " << nearestCoinToPacman->getX() << ", y = " << nearestCoinToPacman->getY() << endl;
		//target coin is set, erase it from the coins vector so it won't be target again
		//coins_locations.erase(coins_locations.begin() + indexToDelete);
		/*if (maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] == COIN)
			maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] = TARGET_COIN;*/

		/*if (!pacmanPath.empty())
		{
		int size = pacmanPath.size();
		for (int i = 0; i < size; i++)
		pacmanPath.pop();
		}*/

		/*for (int i = 1; i < coins_locations.size(); i++)
		{
			if (nearestCoinToPacman->getDistanceFromPoint(coins_locations[i]) == minDistance
				&& nearestCoinToPacman != coins_locations[i])
				nearestCoinsVector.push_back(coins_locations[i]);
		}
	}
	else
	{
		nearestCoinToPacman = nearestCoinsVector[0];
		nearestCoinsVector.erase(nearestCoinsVector.begin());
	}*/
	if (maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] == COIN)
		maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] = TARGET_COIN;


	return true;

}

void init()
{
	srand(time(0));
	setupMaze();

	//coins init
	gray_coins.push_back(new Point2D(startPointForCoins_x, startPointForCoins_y));
	coinsBfs = true;
	setupCoins();

	//pacman init
	startPacmanCoinSearch = true;
	pacmanStartPoint->set_f(pacmanStartPoint, 0);
	pacmanRoute.push(pacmanStartPoint);

	findNearestCoin();

	glClearColor(0.7, 0.7, 0.7, 0);
	glOrtho(0, MSIZE, 0, MSIZE, 0, MSIZE);    //regular origin!!! y from bottom to up, x from left to right

}



void storeCurrentPointForAstar(int row, int col, Point2D*& parentPoint, Point2D*& targetPoint)
{
	Point2D* ptAddToGray = new Point2D(col, row);
	ptAddToGray->set_f(targetPoint, parentPoint->get_g() + 1);

	//if(find(pacmanPath.begin(), pacmanPath.end(), parentPoint) == pacmanPath.end())
	parentPoint->set_f(pacman.getPacmanLocation(), parentPoint->getX() + parentPoint->getY());
	pacmanPath.push(parentPoint);
	/*if(*parentPoint != *nearestCoinToPacman*/
	//pacmanPathParents[row][col] = parentPoint;

//	if (maze[row][col] != PACMAN_VISITED)
		pacmanRoute.push(ptAddToGray);
}

void setPointAsGrayForAStar(int mazeRow, int mazeCol, Point2D*& parentPoint, Point2D*& targetPoint)
{
	//if(checkPointCloseToWall(mazeRow, mazeCol))
	if (maze[mazeRow][mazeCol] == TARGET_COIN || maze[mazeRow][mazeCol] == COIN || maze[mazeRow][mazeCol] == SPACE /*|| maze[mazeRow][mazeCol] == PACMAN_VISITED*/)    //found target
		storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint, targetPoint);
}

/*
*isIncrease - set to true, if you want to increase the parameter, set to false to decrease
*paramRow - set to true, if you want to change the row's value, set to false to change the column's value
*/
//bool findNearestPointToMoveAwayFromWall(int mazeRow, int mazeCol, bool isIncrease, bool paramRow, Point2D* parentPoint)
//{
//    for (int i = 0; i < pacman.getScale(); i++)
//    {
//        mazeRow = (isIncrease ? (paramRow ? (mazeRow + 1) : mazeRow) : (paramRow ? (mazeRow - 1) : mazeRow));
//        mazeCol = (isIncrease ? (paramRow ? (mazeCol + 1) : mazeCol) : (paramRow ? (mazeCol - 1) : mazeCol));
//
//        if (checkPointCloseToWall(mazeRow, mazeCol))
//        {
//            setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
//            return true;
//        }
//    }
//
//    return false;
//}

void addStepPathToPacman(Point2D* parentPoint)
{
	int mazeRow = parentPoint->getY();
	int mazeCol = parentPoint->getX();

	Point2D* tmpParent = new Point2D(*parentPoint);

	for (int i = 1; i < pacman.getScale(); i++)
	{
		if (maze[mazeRow][mazeCol] == TARGET_COIN || maze[mazeRow][mazeCol] == COIN || maze[mazeRow][mazeCol] == SPACE /*|| maze[mazeRow][mazeCol] == PACMAN_VISITED*/)    //found target
		{
			tmpParent->set_f(pacman.getPacmanLocation(), tmpParent->getX() + tmpParent->getY());
			pacmanPath.push(tmpParent);
		}
	}
}

bool checkRouteVertical(Point2D* parentPoint)
{
	int mazeRow = parentPoint->getY();
	int mazeCol = parentPoint->getX();

	if (!checkVertical)
	{
		if (checkPointCloseToWall(rowToCheck, mazeCol))
		{
			setPointAsGrayForAStar(rowToCheck, mazeCol, parentPoint, nearestCoinToPacman);
			checkVertical = true;
			return true;
		}
		else
			return false;

	}

	if (checkPointCloseToWall(mazeRow, mazeCol))
	{

		//check up
		mazeRow = parentPoint->getY() + 1;
		setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);

		//check down
		mazeRow = parentPoint->getY() - 1;
		setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);

		return true;
	}
	else //stuck from up or down
	{
		if (!checkPointCloseToWallVerticalUp(mazeRow, mazeCol))    //stuck from up
		{
			/*for (int i = 0; i < pacman.getScale() / 2; i++)
			{*/
				mazeRow--;
				if (maze[mazeRow][mazeCol] != PACMAN_VISITED)
				{
					if (checkPointCloseToWallVerticalUp(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
				}
				else
				{
					checkVertical = false;
					checkHorizontal = true;
					rowToCheck = mazeRow + 1;

					if (checkPointCloseToWallHorizontalRight(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
					else
					{
						mazeCol--;
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;

					}
				//	setPointAsGrayForAStar(mazeRow, mazeCol - 1, parentPoint, nearestCoinToPacman);
				//	setPointAsGrayForAStar(mazeRow, mazeCol + 1, parentPoint, nearestCoinToPacman);
					return false;
				}
			}
		//}
		else    //stuck from down
		{
			/*for (int i = 0; i < pacman.getScale() / 2; i++)
			{*/
				mazeRow++;
				if (maze[mazeRow][mazeCol] != PACMAN_VISITED)
				{
					if (checkPointCloseToWallVerticalUp(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
				}
				else
				{
					checkVertical = false;
					checkHorizontal = true;
					rowToCheck = mazeRow - 1;

					if (checkPointCloseToWallHorizontalRight(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
					else
					{
						mazeCol--;
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;

					}
				//	setPointAsGrayForAStar(mazeRow, mazeCol - 1, parentPoint, nearestCoinToPacman);
				//	setPointAsGrayForAStar(mazeRow, mazeCol + 1, parentPoint, nearestCoinToPacman);
					return false;
				}
			}
		}
	//}
	return false;
}

bool checkRouteHorizontal(Point2D* parentPoint)
{
	int mazeRow = parentPoint->getY();
	int mazeCol = parentPoint->getX();

	if (!checkHorizontal)
	{
		if (checkPointCloseToWall(mazeRow, colToCheck))
		{
			setPointAsGrayForAStar(mazeRow, colToCheck, parentPoint, nearestCoinToPacman);
			checkHorizontal = true;
			return true;
		}
		else
			return false;

	}

	if (checkPointCloseToWall(mazeRow, mazeCol))
	{
		//check right
		//mazeRow = parentPoint->getY();
		mazeCol = parentPoint->getX() + 1;
		setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);

		//check left
		mazeCol = parentPoint->getX() - 1;
		setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);

		return true;
	}
	else
	{
		if (!checkPointCloseToWallHorizontalRight(mazeRow, mazeCol))    //stuck from right
		{
			/*for (int i = 0; i < pacman.getScale() / 2; i++)
			{*/
				mazeCol--;
				if (maze[mazeRow][mazeCol] != PACMAN_VISITED)
				{
					if (checkPointCloseToWallHorizontalRight(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
				}
				else
				{
					checkVertical = true;
					checkHorizontal = false;
					colToCheck = mazeCol + 1;

					if (checkPointCloseToWallVerticalUp(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
					else
					{
						mazeRow--;
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;

					}
			//		setPointAsGrayForAStar(mazeRow - 1, mazeCol, parentPoint, nearestCoinToPacman);
			//		setPointAsGrayForAStar(mazeRow + 1, mazeCol, parentPoint, nearestCoinToPacman);
					return false;
				}
			}
		//}
		else    //stuck from left
		{
			/*for (int i = 0; i < pacman.getScale() / 2; i++)
			{*/
				mazeCol++;
				if (maze[mazeRow][mazeCol] != PACMAN_VISITED)
				{
					if (checkPointCloseToWallHorizontalRight(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
				}
				else
				{
					checkVertical = true;
					checkHorizontal = false;
					colToCheck = mazeCol - 1;

					if (checkPointCloseToWallVerticalUp(mazeRow, mazeCol))
					{
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;
					}
					else
					{
						mazeRow--;
						setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
						return true;

					}
				//	setPointAsGrayForAStar(mazeRow - 1, mazeCol, parentPoint, nearestCoinToPacman);
				//	setPointAsGrayForAStar(mazeRow + 1, mazeCol, parentPoint, nearestCoinToPacman);
					return false;
				}
			}
		}

	//}
	return false;
}

void createPacmanPathVector()
{
	lastPointForPath = nearestCoinToPacman;
	while (lastPointForPath != NULL && pacmanPathParents[lastPointForPath->getY()][lastPointForPath->getX()] != pacman.getPacmanLocation())
	{
		//if (*lastPointForPath != *nearestCoinToPacman)
		lastPointForPath->set_f(pacman.getPacmanLocation(), 0);
			pacmanPath.push(lastPointForPath);
		lastPointForPath = pacmanPathParents[lastPointForPath->getY()][lastPointForPath->getX()];
	}
}

void a_starIteration()
{
	Point2D* pt;
	int mazeRow, mazeCol;
	cout << "in astar, is pacmanRoute empty: " << pacmanRoute.empty() << "size of vector: " << pacmanRoute.size() << ", is not nearest coin: " << !nearestCoinToPacman << endl;

	if (pacmanRoute.empty())    //grey is the edges that didn't visited yet OR //couldn't find coin
	{
		startPacmanCoinSearch = false;    //there is no path to the target
										  //    createPacmanPathVector();
	}

	else
	{
		pt = pacmanRoute.top();    //this will be the parent
		pacmanRoute.pop();
		lastPointForPath = pt;

		mazeRow = pt->getY();
		mazeCol = pt->getX();

		//paint pt VISITED
		if (maze[mazeRow][mazeCol] == TARGET_COIN)    //found target
		{
			storeCurrentPointForAstar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			startPacmanCoinSearch = false;
			/*vector<Point2D*>::iterator it = find(coins_locations.begin(), coins_locations.end(), nearestCoinToPacman);
			coins_locations.erase(it);*/

		//	createPacmanPathVector();
		}

		else //if( maze[mazeRow][mazeCol] = PACMAN_VISITED) findNearestCoin();
		{
			maze[mazeRow][mazeCol] = PACMAN_VISITED;    //y is i, x is j

			int nearestCoin_x = nearestCoinToPacman->getX();
			int nearestCoin_y = nearestCoinToPacman->getY();

			Point2D* points[4] = { new Point2D(mazeCol, mazeRow + 1),
				new Point2D(mazeCol, mazeRow - 1),
				new Point2D(mazeCol + 1, mazeRow),
				new Point2D(mazeCol - 1, mazeRow)
			};

			/*double minVerticalDist = fmin(points[0]->getDistanceFromPoint(nearestCoinToPacman), points[1]->getDistanceFromPoint(nearestCoinToPacman));
			double minHorizontalDist = fmin(points[2]->getDistanceFromPoint(nearestCoinToPacman), points[3]->getDistanceFromPoint(nearestCoinToPacman));
			if (minVerticalDist < minHorizontalDist)
			{
			if (!checkRouteVertical(pt))
			checkRouteHorizontal(pt);
			}
			else
			{
			if (!checkRouteHorizontal(pt))
			checkRouteVertical(pt);
			}*/

			if (mazeCol != nearestCoin_x && mazeRow != nearestCoin_y)
			{
				if (checkVertical)
				{
					if (!checkRouteVertical(pt))
						checkRouteHorizontal(pt);
				}
				else
				{
					checkRouteHorizontal(pt);
					checkRouteVertical(pt);
				}
			}
			else if (mazeCol != nearestCoin_x)
			{
				if (checkHorizontal)
				{
					if (!checkRouteHorizontal(pt))
						checkRouteVertical(pt);
				}
				else
				{
					checkRouteVertical(pt);
					checkRouteHorizontal(pt);
				}
			}
			else if (mazeRow != nearestCoin_y)    // in case that pacman and the coin isn't on the same row (y value)
			{
				if (checkVertical)
				{
					if (!checkRouteVertical(pt))
						checkRouteHorizontal(pt);
				}
				else
				{
					checkRouteHorizontal(pt);
					checkRouteVertical(pt);
				}
			}
		}
	}
}

/*
rowDif = 1, to increase row,
rowDif = -1, to decrease row,
rowDif = 0, in that case the column value will change,
same goes for the colDif
*/
void createPacmanPathVector(int mazeRow, int mazeCol, int rowDif, int colDif, Point2D* parentPoint)
{
	Point2D* tmpParent = new Point2D(mazeCol, mazeRow);

	int i;
	int row, col;

	for (i = 1; i < pacman.getScale() - 1 && (*tmpParent != *nearestCoinToPacman); i++)
	{
		row = mazeRow + i * rowDif;
		col = mazeCol + i * colDif;
		tmpParent = new Point2D(col, row);

		if (checkPointCloseToWall(row, col))
		{
			if (*tmpParent == *nearestCoinToPacman)
				startPacmanCoinSearch = false;
			tmpParent->set_f(pacman.getPacmanLocation(), tmpParent->getDistanceFromPoint(nearestCoinToPacman)/*tmpParent->getX() + tmpParent->getY()*/);
			pacmanPath.push(tmpParent);
			//setPointAsGrayForAStar(mazeRow, mazeCol, tmpParent, nearestCoinToPacman);
		}
		else
			return;
	}
	//for pacmanPath
	if (startPacmanCoinSearch)
	{
		row = mazeRow + i * rowDif;
		col = mazeCol + i * colDif;
		tmpParent = new Point2D(col, row);
		setPointAsGrayForAStar(row, col, tmpParent, nearestCoinToPacman);
	}

}

//void a_starIteration()
//{
//    Point2D* pt;
//    int mazeRow, mazeCol;
//    cout << "in astar, is pacmanRoute empty: " << pacmanRoute.empty() << "size of vector: " << pacmanRoute.size() << ", is not nearest coin: " << !nearestCoinToPacman << endl;
//
//    if (pacmanRoute.empty())    //grey is the edges that didn't visited yet OR //couldn't find coin
//        startPacmanCoinSearch = false;    //there is no path to the target
//
//    else
//    {
//        pt = pacmanRoute.top();    //this will be the parent
//        pacmanRoute.pop();
//
//        mazeRow = pt->getY();
//        mazeCol = pt->getX();
//
//        //paint pt VISITED
//        if (maze[mazeRow][mazeCol] == TARGET_COIN)    //found target
//        {
//            storeCurrentPointForAstar(mazeRow, mazeCol, pt, nearestCoinToPacman);
//            startPacmanCoinSearch = false;
//
//        }
//
//        else
//        {
//            maze[mazeRow][mazeCol] = PACMAN_VISITED;    //y is i, x is j
//
//            int nearestCoin_x = nearestCoinToPacman->getX();
//            int nearestCoin_y = nearestCoinToPacman->getY();
//
//            //check up
//            //createPacmanPathVector(mazeRow, mazeCol, 1, 0, pt);
//            mazeRow = pt->getY() + 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
//
//            //check down
//            //createPacmanPathVector(mazeRow, mazeCol, -1, 0, pt);
//            mazeRow = pt->getY() - 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
//
//            //check right
//            //createPacmanPathVector(mazeRow, mazeCol, 0, 1, pt);
//            mazeRow = pt->getY();
//            mazeCol = pt->getX() + 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
//
//            //check left
//            //createPacmanPathVector(mazeRow, mazeCol, 0, -1, pt);
//            mazeCol = pt->getX() - 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
//
//
//        }
//    }
//}


//drawing the maze 100x100px - MSIZExMSIZE, setting the color for each pixel in the maze, and displaying it
void drawMaze()
{
	int i, j;
	double m2scr = (2.0) / MSIZE;

	for (i = 0; i < MSIZE; i += CELL_SIZE)
	{
		for (j = 0; j < MSIZE; j += CELL_SIZE)
		{
			switch (maze[i][j])
			{
			case WALL:
				glColor3d(0.2, 0, 1);    //purple
				break;
			case SPACE:
				glColor3d(0, 0, 0);    //black
				break;
			case COIN:
				glColor3d(1, 1, 1);
				break;
			case UNREACHABLE:
				glColor3d(1, 0, 0);
				break;
			case CENTER:
				glColor3d(0.2, 0.6, 0.5);
				break;
			case TARGET_COIN:
				glColor3d(0, 1, 0);
				break;
			case PACMAN_VISITED:
				glColor3d(0, 0, 1);
				break;

			}


			glBegin(GL_POLYGON);
			glVertex2d(j, i);
			glVertex2d(j + CELL_SIZE, i);
			glVertex2d(j + CELL_SIZE, i + CELL_SIZE);
			glVertex2d(j, i + CELL_SIZE);
			glEnd();

		}
	}

}

//double calculatePacmanAngleToNextCoint(Point2D* coinLocation)
//{
//    int pac_x = pacman.getPacmanLocation()->getX();
//    int pac_y = pacman.getPacmanLocation()->getY();
//
//    int coin_x = coinLocation->getX();
//    int coin_y = coinLocation->getY();
//    int divisor = coin_x - pac_x;
//    int devident = coin_y - pac_y;
//    double angle =(atan2(devident, divisor) * 180 / M_PI);
//
//    return angle;
//
//}

void movePacmanToCoin()
{
	int pacman_x = pacman.getPacmanLocation()->getX();
	int pacman_y = pacman.getPacmanLocation()->getY();

	/*if (maze[pacman_y][pacman_x] == PACMAN_VISITED)
		maze[pacman_y][pacman_x] == SPACE;*/
	//eat the coin
	if (maze[pacman_y][pacman_x] == TARGET_COIN)
	{
		maze[pacman_y][pacman_x] = SPACE;
		if (!pacmanRoute.empty())
		{
			int size = pacmanRoute.size();
			for (int i = 0; i < size; i++)
				pacmanRoute.pop();
		}

		vector<Point2D*>::iterator itr = find(coins_locations.begin(), coins_locations.end(), nearestCoinToPacman);
		coins_locations.erase(itr);

		startPacmanCoinSearch = true;
		findNearestCoin();
		pacman.getPacmanLocation()->set_f(nearestCoinToPacman, 0);
		pacmanRoute.push(pacman.getPacmanLocation());
		
	}
	//multiple cells in pacmacn path vector
	else if (!pacmanPath.empty())
	{
		Point2D* nextMove = pacmanPath.top();
		int nextMove_x = nextMove->getX();
		int nextMove_y = nextMove->getY();
		double pacmanAngle;

		pacmanPath.pop();
		PacmanPawn::pacmanDirection pacDir = pacman.getDirection();

		if (pacman_x < nextMove_x)
			pacDir = PacmanPawn::pacmanDirection::Right;
		else if (pacman_x > nextMove_x)
			pacDir = PacmanPawn::pacmanDirection::Left;
		else if (pacman_y < nextMove_y)
			pacDir = PacmanPawn::pacmanDirection::Up;
		else if (pacman_y > nextMove_y)
			pacDir = PacmanPawn::pacmanDirection::Down;

		pacman.setTranslation(pacDir, new Point2D(nextMove_x, nextMove_y));
		maze[pacman_y][pacman_x] = SPACE;

	}
	else
	{
		startPacmanCoinSearch = true;
		findNearestCoin();
		pacman.getPacmanLocation()->set_f(nearestCoinToPacman, 0);
		pacmanRoute.push(pacman.getPacmanLocation());
	}


}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	drawMaze();
	glPopMatrix();

	pacman.drawPacman();
	monster1.drawMonster();

	glutSwapBuffers();// show what was drawn in "frame buffer"
}

void cleanUpMaze()
{
	for (int i = 0; i < MSIZE; i++)
		for (int j = 0; j < MSIZE; j++)
			if (maze[i][j] == PACMAN_VISITED)
				maze[i][j] = SPACE;
}

void idle()
{
	if (!coinsBfs)
	{
		cout << "in idle, startPacmanRun = " << startPacmanCoinSearch << endl;
		if (startPacmanCoinSearch)
			a_starIteration();
		else
		{
			cleanUpMaze();
			checkHorizontal = true;
			checkVertical = true;
			stepCounter++;
			//    if (stepCounter % 2 == 0)
			movePacmanToCoin();
			if (stepCounter % 50 == 0)
				pacman.changeIsOpen();
			/*if (stepCounter % 1000 == 0)
			{
			findNearestCoin();
			startPacmanCoinSearch = true;
			}*/
		}
	}

	glutPostRedisplay();// calls indirectly to display
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(200, 100);
	glutCreateWindow("Pacman");

	glutDisplayFunc(display); // refresh function
	glutIdleFunc(idle); // idle: when nothing happens
	init();

	glutMainLoop();
	return 0;

}
