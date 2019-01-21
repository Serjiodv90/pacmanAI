#include "GLUT.H"
#include <math.h>
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
vector<Point2D*> gray_coins;	//for bfs ditrebution of the coins
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
bool startPacmanCoinSearch = false;
//vector<Point2D*> pacmanRoute;
priority_queue <Point2D*, vector<Point2D*>, ComparePointsByDist> pacmanRoute;
vector<Point2D*> pacmanPath;

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
	for(int i = bottomRow; i < topRow; i++)
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

	setUnreachableZone((MSIZE / 2) - SPACE_SIZE + 1, (MSIZE / 2) + SPACE_SIZE ,
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

	// bottom rectangle
	for (i = MSIZE - 7 * SPACE_SIZE + CELL_SIZE; i >= MSIZE - 8 * SPACE_SIZE + CELL_SIZE; i--)
	{
		thickenWallInMaze(Right, i, (MSIZE / 2) - SPACE_SIZE);
		thickenWallInMaze(Left, i, (MSIZE / 2) + SPACE_SIZE);

		for (j = MSIZE / 2 - SPACE_SIZE; j < MSIZE / 2 + SPACE_SIZE; j++)
			if (i == MSIZE - 7 * SPACE_SIZE + CELL_SIZE || i == MSIZE - 8 * SPACE_SIZE + CELL_SIZE)
				thickenWallInMaze(Top, i, j);
		
	}

	setUnreachableZone(MSIZE - 8 * SPACE_SIZE + CELL_SIZE + 1, MSIZE - 7 * SPACE_SIZE + CELL_SIZE,
		MSIZE / 2 - SPACE_SIZE + 1, MSIZE / 2 + SPACE_SIZE);

	// top rectangle
	for (i = MSIZE - 2 * SPACE_SIZE; i >= MSIZE - 3 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, (MSIZE / 2) - SPACE_SIZE);
		thickenWallInMaze(Left, i, (MSIZE / 2) + SPACE_SIZE);

		for (j = MSIZE / 2 - SPACE_SIZE; j < MSIZE / 2 + SPACE_SIZE; j++)
			if (i == MSIZE - 2 * SPACE_SIZE || i == MSIZE - 3 * SPACE_SIZE)
				thickenWallInMaze(Bottom, i, j);
	}

	setUnreachableZone(MSIZE - 3 * SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE,
		MSIZE / 2 - SPACE_SIZE + 1, MSIZE / 2 + SPACE_SIZE);

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
	for (j = MSIZE - 2 * SPACE_SIZE; j > MSIZE / 2 - SPACE_SIZE; j--)
	{
		thickenWallInMaze(Bottom, 7 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, 6 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 7 * SPACE_SIZE - CELL_SIZE; i > 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, MSIZE - 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE / 2 - SPACE_SIZE);
	}

	setUnreachableZone(6 * SPACE_SIZE + CELL_SIZE + 1, 7 * SPACE_SIZE - CELL_SIZE, MSIZE / 2 - SPACE_SIZE + 1, MSIZE - 2 * SPACE_SIZE);


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
	for (j = 2 * SPACE_SIZE; j < MSIZE / 2 + SPACE_SIZE; j++)
	{
		thickenWallInMaze(Bottom, 4 * SPACE_SIZE - CELL_SIZE, j);
		thickenWallInMaze(Top, 3 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 4 * SPACE_SIZE - CELL_SIZE; i > 3 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, MSIZE / 2 + SPACE_SIZE);
	}

	setUnreachableZone(3 * SPACE_SIZE + CELL_SIZE + 1, 4 * SPACE_SIZE - CELL_SIZE, 2 * SPACE_SIZE + 1, MSIZE / 2 + SPACE_SIZE);


	// draw left +
	for (j = 2 * SPACE_SIZE; j < 7 * SPACE_SIZE; j++)
	{
		thickenWallInMaze(Bottom, 7 * SPACE_SIZE - CELL_SIZE, j);
		if ((j < 4 * SPACE_SIZE && j >= 2 * SPACE_SIZE) || (j > 5 * SPACE_SIZE && j < 7 * SPACE_SIZE))
			thickenWallInMaze(Top, 6 * SPACE_SIZE + CELL_SIZE, j);
	}

	for (i = 6 * SPACE_SIZE + CELL_SIZE; i > 5 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 4 * SPACE_SIZE);
		thickenWallInMaze(Left, i, 5 * SPACE_SIZE);
	}

	for (i = 7 * SPACE_SIZE - CELL_SIZE; i > 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, 7 * SPACE_SIZE);
	}

	for (j = 4 * SPACE_SIZE; j <= 5 * SPACE_SIZE; j++)
	{
		thickenWallInMaze(Top, 5 * SPACE_SIZE, j);
	}

	setUnreachableZone(6 * SPACE_SIZE + CELL_SIZE + 1, 7 * SPACE_SIZE - CELL_SIZE, 2 * SPACE_SIZE + 1, 7 * SPACE_SIZE);
	setUnreachableZone(5 * SPACE_SIZE + 1, 7 * SPACE_SIZE - CELL_SIZE, 4 * SPACE_SIZE + 1, 5 * SPACE_SIZE);

}

void setupInitials()
{
	int i, j;

	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 8 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE));
		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE));

		for (j = MSIZE / 2 - 3 * SPACE_SIZE; j < MSIZE / 2 + 3 * SPACE_SIZE; j++)
		{
			if (i == MSIZE - 5 * SPACE_SIZE)
				thickenWallInMaze(Bottom, i, j);
		}
	}

	for (i = 2 * SPACE_SIZE; i <= 8 * SPACE_SIZE; i++)
	{
		if (i < 5 * SPACE_SIZE)
			thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) - CELL_SIZE);
		else if (i < 8 * SPACE_SIZE)
			thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE));

		for (j = MSIZE / 2 - 3 * SPACE_SIZE; j < MSIZE / 2 + 3 * SPACE_SIZE; j++)
		{
			if (i == 2 * SPACE_SIZE || i == 5 * SPACE_SIZE || i == 8 * SPACE_SIZE)
			{
				thickenWallInMaze(Top, i, j);
				/*thickenWallInMaze(Top, i + CELL_SIZE, j);
				thickenWallInMaze(Top, i + 2*CELL_SIZE, j);*/
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
	for (int i = row; i < row + 2 * range && i < MSIZE; i++)
		if (maze[i][col] == WALL || maze[i][col] == UNREACHABLE || maze[i][col] == CENTER)
			return false;

	return true;
}

bool checkPointCloseToWallVertical(int row, int col, int range = 2 * CELL_SIZE)
{
	if (row - range <= 0)
		return false;

	row -= range;
	for(int i = row; i < row + 2*range && i < MSIZE; i++)
		if (maze[i][col] == WALL || maze[i][col] == UNREACHABLE || maze[i][col] == CENTER)
			return false;

	return true;
}

bool checkPointCloseToWallHorizontalRight(int row, int col, int range = 2 * CELL_SIZE)
{
	for (int j = col; j < col + 2 * range && j < MSIZE; j++)
		if (maze[row][j] == WALL || maze[row][j] == UNREACHABLE || maze[row][j] == CENTER)
			return false;

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
		maze[mazeRow][mazeCol] = COIN;	//add it to gray
		coinCounter++;
		storeCurrentPointInParrentArray(mazeRow, mazeCol, parentPoint, parentArray, grayVector);
		return true;
	}

	return false;
}

void bfsIteration(/*Point2D* parentArray[][MSIZE], vector<Point2D*> &gray_coins, int beginPoint,
	int beginPoint_visitedFrom, int goalPoint, int goalPoint_visitedFrom*/)
{
	Point2D* pt;
	int mazeRow, mazeCol;

	if (gray_coins.empty() || coinCounter >= MAX_COINS)	//grey is the edges that didn't visited yet
		coinsBfs = false;	//there is no path to the target

	else
	{
		pt = gray_coins[0];	//this will be the parent
		gray_coins.erase(gray_coins.begin());	//deque

		mazeRow = pt->getY();
		mazeCol = pt->getX();

		//paint pt VISITED 
		if ((mazeRow == MSIZE - (startPointForCoins_y)) && (mazeCol == MSIZE - (startPointForCoins_x)))	//found target	
		{
			storeCurrentPointInParrentArray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//parent_forCoins[mazeRow][mazeCol] = pt;
			return;
		}

		else
		{
			if (maze[mazeRow][mazeCol] == SPACE)
				maze[mazeRow][mazeCol] = COIN;	//y is i, x is j


			mazeRow = pt->getY() + COINS_STEP;
		//	if (checkPointCloseToWall(mazeRow, mazeCol))
				setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeRow = pt->getY() - COINS_STEP;
		//	if (checkPointCloseToWall(mazeRow, mazeCol))
				setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeRow = pt->getY();
			mazeCol = pt->getX() + COINS_STEP;
		//	if (checkPointCloseToWall(mazeRow, mazeCol))
				setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			mazeCol = pt->getX() - COINS_STEP;
		//	if (checkPointCloseToWall(mazeRow, mazeCol))
				setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			////check down
			//mazeRow = pt->getY() + COINS_STEP;
			//if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
			//{
			//	//check up
			//	mazeRow = pt->getY() - COINS_STEP;
			//	if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
			//	{
			//		//check right
			//		mazeRow = pt->getY();
			//		mazeCol = pt->getX() + COINS_STEP;
			//		if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
			//		{
			//			//check left
			//			mazeCol = pt->getX() - COINS_STEP;
			//			setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

			//		}
			//	}
			//}

			//mazeRow = pt->getY() - COINS_STEP;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			//	//check up
			//mazeRow = pt->getY() + COINS_STEP;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			//		//check right
			//mazeRow = pt->getY();
			//mazeCol = pt->getX() + COINS_STEP;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			////check left
			//mazeCol = pt->getX() - COINS_STEP;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

		}
	}
}

void setupCoins()
{
	while (coinsBfs)
		bfsIteration();
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

	Point2D* pacmanLocation = pacman.getPacmanLocation();
	nearestCoinToPacman = coins_locations[0];
	double minDistance = pacmanLocation->getDistanceFromTarget(nearestCoinToPacman);
	double tmpDist = 0.0;
	Point2D* tmpCoinLoc;
	int indexToDelete = 0;
	//startPacmanRun = true;

	for (int i = 1; i < coins_locations.size(); i++)
	{
		tmpCoinLoc = coins_locations[i];
		tmpDist = pacmanLocation->getDistanceFromTarget(tmpCoinLoc);
		if (tmpDist < minDistance)
		{

			minDistance = tmpDist;
			nearestCoinToPacman = tmpCoinLoc;
			indexToDelete = i;
		}
	}

	cout << "the first coin in: x = " << nearestCoinToPacman->getX() << ", y = " << nearestCoinToPacman->getY() << endl;
	//target coin is set, erase it from the coins vector so it won't be target again
	coins_locations.erase(coins_locations.begin() + indexToDelete);
	if(maze[nearestCoinToPacman->getY()][nearestCoinToPacman->getX()] == COIN)
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

//void moveFigureOnXAxis()
//{
//	int x = pacman.getPacmanLocation()->getX();
//	int y = pacman.getPacmanLocation()->getY();
//
//	PacmanPawn::pacmanDirection pacDir = pacman.getDirection();
//
//	switch (pacDir)
//	{
//	case PacmanPawn::pacmanDirection::Right:
//		break;
//	default:
//		break;
//	}
//
//	if (maze[y][x] == COIN || maze[y][x] == TARGET_COIN)
//		maze[y][x] = SPACE;
//
//	if ((maze[y - 2 * CELL_SIZE][(x + 1) + CELL_SIZE] != WALL) && (maze[y + 2 * CELL_SIZE][(x + 1) + CELL_SIZE] != WALL)
//		&& (maze[y][(x + 1) + CELL_SIZE] != WALL))
//	{
//		x++;
//		if (x >= MSIZE)
//			x = 0;
//		if (fmod(floor(x), 10) == 0)
//			pacman.changeIsOpen();
//
//		pacman.setTranslation(PacmanPawn::pacmanDirection::Right, new Point2D(x, y));
//	}
//	else if ((maze[(y + 1) + CELL_SIZE][x - 2 * CELL_SIZE] != WALL) && (maze[(y + 1) + CELL_SIZE][x + CELL_SIZE] != WALL)
//		&& (maze[(y + 1) + CELL_SIZE][x] != WALL))
//	{
//		y++;
//		if (fmod(floor(y), 10) == 0)
//			pacman.changeIsOpen();
//
//		pacman.setTranslation(PacmanPawn::pacmanDirection::Up, new Point2D(x, y));
//	}
//
//
//
//}

//void drawCoin(int row, int col)
//{
//	glPushMatrix();
//	glTranslated(col, row, 0);
//	glScaled(2, 2, 1);
//
//	double PI = 3.14;
//	double alpha, x, y, radius = 1, delta = PI / 20;
//
////	draw the outline of the wheel
//	glColor3d(1, 1, 1);
//	glBegin(GL_POLYGON);
//	for (alpha = 0; alpha <= 2 * PI; alpha += delta)
//	{
//		x = radius * cos(alpha);
//		y = radius * sin(alpha);
//		glVertex2d(x, y);
//	}
//	glEnd();
//
//	glPopMatrix();
//}



//bool isBfsFoundPath(int row, int col, int goalPoint)
//{
//	if (maze[row][col] == goalPoint)   //found target
//		return true;
//
//	return false;
//}
//
//void insertToQueueUniqe(Point2D* pt)
//{
//	set<Point2D*, ComparePointsByDist> tmpSet;
//	int size = pacmanRoute.size();
//	for (int i = 0; i < size; i++)
//	{
//		tmpSet.insert(pacmanRoute.top());
//		pacmanRoute.pop();
//	}
//
//	size = tmpSet.size();
//	for (set<Point2D*, ComparePointsByDist>::iterator itr = tmpSet.begin(); itr != tmpSet.end(); ++itr)
//	{
//		pacmanRoute.push(*itr);
//	}
//	tmpSet.erase(tmpSet.begin(), tmpSet.end());
//		
//}

void storeCurrentPointForAstar(int row, int col, Point2D*& parentPoint, Point2D*& targetPoint)
{
	Point2D* ptAddToGray = new Point2D(col, row);
	ptAddToGray->set_f(targetPoint, parentPoint->get_g() + 1);

	if(find(pacmanPath.begin(), pacmanPath.end(), parentPoint) == pacmanPath.end())
		pacmanPath.push_back(parentPoint);

	if(maze[row][col] != PACMAN_VISITED)
		pacmanRoute.push(ptAddToGray);
	//insertToQueueUniqe(ptAddToGray);
}

void setPointAsGrayForAStar(int mazeRow, int mazeCol, Point2D*& parentPoint, Point2D*& targetPoint)
{
	if(checkPointCloseToWall(mazeRow, mazeCol))
		if (maze[mazeRow][mazeCol] == TARGET_COIN || maze[mazeRow][mazeCol] == COIN || maze[mazeRow][mazeCol] == SPACE || maze[mazeRow][mazeCol] == PACMAN_VISITED)    //found target
			storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint, targetPoint);
}

bool checkRouteVertical(Point2D* parentPoint)
{
	int mazeRow = parentPoint->getY();
	int mazeCol = parentPoint->getX();

	if (checkPointCloseToWallVertical(mazeRow, mazeCol))
	{
		/*if (mazeRow < nearestCoinToPacman->getY())
		{*/
			//check up
			mazeRow = parentPoint->getY() + 1;
			setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
		/*}
		else if (mazeRow > nearestCoinToPacman->getY())
		{*/
			//check down
			mazeRow = parentPoint->getY() - 1;
			setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
		//}
		/*else
			return false;*/

		return true;
	}
	else
		return false;
}

bool checkRouteHorizontal(Point2D* parentPoint)
{
	int mazeRow = parentPoint->getY();
	int mazeCol = parentPoint->getX();

	if (checkPointCloseToWallHorizontal(mazeRow, mazeCol))
	{
		/*if (mazeCol < nearestCoinToPacman->getX())
		{*/
			//check right
			mazeRow = parentPoint->getY();
			mazeCol = parentPoint->getX() + 1;
			setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
		/*}
		else if (mazeCol > nearestCoinToPacman->getX())
		{*/
			//check left
			mazeCol = parentPoint->getX() - 1;
			setPointAsGrayForAStar(mazeRow, mazeCol, parentPoint, nearestCoinToPacman);
		//}
		/*else
			return false;*/
		

		return true;
	}
	else
		return false;
}

void a_starIteration()
{
    Point2D* pt;
    int mazeRow, mazeCol;
	cout << "in astar, is pacmanRoute empty: " << pacmanRoute.empty() << "size of vector: " <<pacmanRoute.size()<<", is not nearest coin: " << !nearestCoinToPacman << endl;

	if (pacmanRoute.empty() /*|| !nearestCoinToPacman*//*!findNearestCoin()*/)    //grey is the edges that didn't visited yet OR //couldn't find coin
	{
		startPacmanCoinSearch = false;    //there is no path to the target
		//findNearestCoin();
	}
    else
    {
		pt = pacmanRoute.top();    //this will be the parent
		pacmanRoute.pop();

		mazeRow = /*pacman.getPacmanLocation()->getY();*/pt->getY();
		mazeCol = /*pacman.getPacmanLocation()->getX();*/pt->getX();

        //paint pt VISITED
		if (maze[mazeRow][mazeCol] == TARGET_COIN)    //found target
		{
			storeCurrentPointForAstar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			startPacmanCoinSearch = false;
		}

        else
        {
            maze[mazeRow][mazeCol] = PACMAN_VISITED;    //y is i, x is j

			int pacman_x = pacman.getPacmanLocation()->getX();
			int pacman_y = pacman.getPacmanLocation()->getY();
			int nearestCoin_x = nearestCoinToPacman->getX();
			int nearestCoin_y = nearestCoinToPacman->getY();

			if (/*pacman_x*/mazeCol != nearestCoin_x && /*pacman_y */mazeRow != nearestCoin_y)
			{
				if (!checkRouteVertical(pt))
					checkRouteHorizontal(pt);
			}
			else if (/*pacman_x*/ mazeCol != nearestCoin_x)
			{
				if (!checkRouteHorizontal(pt))
					checkRouteVertical(pt);
			}
			else	// in case that pacman and the coin isn't on the same row (y value)
			{
				if (!checkRouteVertical(pt))
					checkRouteHorizontal(pt);
			}

   //         //check up
			//if (nearestCoinToPacman->getY() > mazeRow)
			//{
			//	mazeRow = pt->getY() + 1;
			//	setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			//}

   //         //check down
			//if (nearestCoinToPacman->getY() < mazeRow)
			//{
			//	mazeRow = pt->getY() - 1;
			//	setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			//}

   //         //check right
			//if (nearestCoinToPacman->getX() > mazeCol)
			//{
			//	mazeRow = pt->getY();
			//	mazeCol = pt->getX() + 1;
			//	setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			//}

   //         //check left
			//if (nearestCoinToPacman->getX() < mazeCol)
			//{
			//	mazeCol = pt->getX() - 1;
			//	setPointAsGrayForAStar(mazeRow, mazeCol, pt, nearestCoinToPacman);
			//}

    //        if (!startPacmanRun)    //target was found
				//findNearestCoin();
        }
    }
}


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
				/*drawCoin(i, j);
				glColor3d(0, 0, 0);*/
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

double calculatePacmanAngleToNextCoint(Point2D* coinLocation)
{
	int pac_x = pacman.getPacmanLocation()->getX();
	int pac_y = pacman.getPacmanLocation()->getY();

	int coin_x = coinLocation->getX();
	int coin_y = coinLocation->getY();
	int divisor = coin_x - pac_x;
	int devident = coin_y - pac_y;
	double angle =/* divisor == 0 ? 90 : (devident) == 0 ? 0 :*/ (atan2(devident, divisor) * 180 / M_PI);

	return angle;

}

void movePacmanToCoin()
{
	int pacman_x = pacman.getPacmanLocation()->getX();
	int pacman_y = pacman.getPacmanLocation()->getY();

	//eat the coin
	if (/*maze[y][x] == COIN || */maze[pacman_y][pacman_x] == TARGET_COIN)
	{
		maze[pacman_y][pacman_x] = SPACE;
		if (!pacmanRoute.empty())
		{
			int size = pacmanRoute.size();
			for (int i = 0; i < size; i++)
				pacmanRoute.pop();
		}

		
		
		startPacmanCoinSearch = true;
		findNearestCoin();
		pacman.getPacmanLocation()->set_f(nearestCoinToPacman, 0);
		pacmanRoute.push(pacman.getPacmanLocation());
		return;
	}
	//multiple cells in pacmacn path vector
	if (!pacmanPath.empty())
	{
		Point2D* nextMove = pacmanPath.front();
		int nextMove_x = nextMove->getX();
		int nextMove_y = nextMove->getY();
		double pacmanAngle;

		pacmanPath.erase(pacmanPath.begin());
		PacmanPawn::pacmanDirection pacDir = pacman.getDirection();


		/*if (pacman_x != nextMove_x && pacman_y != nextMove_y)
		{*/
		//pacmanAngle = calculatePacmanAngleToNextCoint(nextMove);
		//pacman.setTranslation(int(pacmanAngle), /*new Point2D(nextMove_x, nextMove_y)*/nearestCoinToPacman);

	//}

		if (pacman_x < nextMove_x)
			pacDir = PacmanPawn::pacmanDirection::Right;
		else if (pacman_x > nextMove_x)
			pacDir = PacmanPawn::pacmanDirection::Left;
		else if (pacman_y < nextMove_y)
			pacDir = PacmanPawn::pacmanDirection::Up;
		else if (pacman_y > nextMove_y)
			pacDir = PacmanPawn::pacmanDirection::Down;



		//pacmanAngle = calculatePacmanAngleToNextCoint(nextMove);
		//pacman.setTranslation(int(pacmanAngle), new Point2D(nextMove_x, nextMove_y)/*nearestCoinToPacman*/);
		pacman.setTranslation(pacDir, new Point2D(nextMove_x, nextMove_y));
		maze[pacman_y][pacman_x] = SPACE;

	}
	else
		startPacmanCoinSearch = true;


}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
	drawMaze();
	glPopMatrix();

	//pacman.drawPacmanClosed();
	pacman.drawPacman();
	monster1.drawMonster();

	glutSwapBuffers();// show what was drawn in "frame buffer"
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
			stepCounter++;
			if (stepCounter % 5 == 0)
				movePacmanToCoin();
			if (stepCounter % 50 == 0)
				pacman.changeIsOpen();
		}
	}

	/*if (coinsBfs)
		bfsIteration();*/


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
