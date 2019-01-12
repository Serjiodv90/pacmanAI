#include "GLUT.H"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>

#include "Point2D.h"
#include "pacmanPawn.h"
#include "Monster.h"

using namespace std;

const int MSIZE = 600;
const int W = MSIZE; // window width
const int H = MSIZE; // window height

const int SPACE = 1;
const int WALL = 2;
const int COIN = 3;
const int UNREACHABLE = 4;

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
vector<Point2D*> gray_coins;
bool coinsBfs = false;
Point2D* parent_forCoins[MSIZE][MSIZE];

vector<Point2D*> gray_target;
//Point2D* startPoint, *targetPoint;

int startPointForCoins_x = 3 * CELL_SIZE;
int startPointForCoins_y = 5 * CELL_SIZE;

Point2D* pacmanStartPoint = new Point2D(startPointForCoins_x, startPointForCoins_y);

PacmanPawn pacman(pacmanStartPoint, 2 * CELL_SIZE);
vector<double>tmpColor = { 0,1,1 };

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

void setUnreachableZone(int bottomRow, int topRow, int leftCol, int rightCol)
{
	for(int i = bottomRow; i < topRow; i++)
		for (int j = leftCol; j < rightCol; j++)
		{
			maze[i][j] = UNREACHABLE;
		}
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

				setUnreachableZone(i+1, i + middleDoubleSpaceCells, 0, MSIZE / 6);
				setUnreachableZone(i+1, i + middleDoubleSpaceCells, MSIZE - (MSIZE / 6), MSIZE);


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
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE);
		thickenWallInMaze(Right, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE - SPACE_SIZE);

		//right wall
		thickenWallInMaze(Right, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE + SPACE_SIZE);
	}

	// top section
	for (i = 2 * MSIZE / 3 - middleDoubleSpaceCells; i < 2 * MSIZE / 3 + CELL_SIZE; i++)
	{
		//left wall
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE);
		thickenWallInMaze(Right, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE - SPACE_SIZE);

		//right wall
		thickenWallInMaze(Right, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE);
		thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE + SPACE_SIZE);
	}

	// left vertical lines
	for (j = (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE; j > (MSIZE / 2) - (3 * SPACE_SIZE) - 3 * SPACE_SIZE - CELL_SIZE; j--)
	{
		// top
		thickenWallInMaze(Top, 2 * MSIZE / 3 - middleDoubleSpaceCells, j);
		thickenWallInMaze(Bottom, 2 * MSIZE / 3, j);

		// bottom
		thickenWallInMaze(Top, MSIZE / 3, j);
		thickenWallInMaze(Bottom, MSIZE / 3 + middleDoubleSpaceCells, j);
	}

	// right vertical lines
	for (j = (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE; j < (MSIZE / 2) + (3 * SPACE_SIZE) + 3 * SPACE_SIZE + CELL_SIZE; j++)
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
		{
			if (i == MSIZE - 7 * SPACE_SIZE + CELL_SIZE || i == MSIZE - 8 * SPACE_SIZE + CELL_SIZE)
				thickenWallInMaze(Top, i, j);
		}
	}

	// top rectangle
	for (i = MSIZE - 2 * SPACE_SIZE; i >= MSIZE - 3 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, (MSIZE / 2) - SPACE_SIZE);
		thickenWallInMaze(Left, i, (MSIZE / 2) + SPACE_SIZE);

		for (j = MSIZE / 2 - SPACE_SIZE; j < MSIZE / 2 + SPACE_SIZE; j++)
		{
			if (i == MSIZE - 2 * SPACE_SIZE || i == MSIZE - 3 * SPACE_SIZE)
				thickenWallInMaze(Bottom, i, j);
		}
	}

	// left +
	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE);

		if ((i <= MSIZE - 2 * SPACE_SIZE && i > MSIZE - 3 * SPACE_SIZE - CELL_SIZE) || (i < MSIZE - 4 * SPACE_SIZE && i > MSIZE - 6 * SPACE_SIZE - CELL_SIZE))
			thickenWallInMaze(Right, i, (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE - SPACE_SIZE);
	}

	for (j = (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE; j >= (MSIZE / 2) - (3 * SPACE_SIZE) - 2 * SPACE_SIZE - SPACE_SIZE; j--)
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

	// right +
	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 6 * SPACE_SIZE; i--)
	{
		thickenWallInMaze(Right, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE);

		if ((i <= MSIZE - 2 * SPACE_SIZE && i > MSIZE - 3 * SPACE_SIZE - CELL_SIZE) || (i < MSIZE - 4 * SPACE_SIZE && i > MSIZE - 6 * SPACE_SIZE - CELL_SIZE))
			thickenWallInMaze(Left, i, (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE + SPACE_SIZE);
	}

	for (j = (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE; j <= (MSIZE / 2) + (3 * SPACE_SIZE) + 2 * SPACE_SIZE + SPACE_SIZE; j++)
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
	/*setupCenterSquare();
	setupInitials();
	setupCenterWalls();
	setupBottomSection();
	setupTopSection();*/
}

bool checkPointCloseToWall(int row, int col)
{
	if (row -  CELL_SIZE < 0)
	{
		if (col -  CELL_SIZE < 0)
		{
			row -=  CELL_SIZE;
			col -=  CELL_SIZE;
		}
		else
		{
			row -= CELL_SIZE;
			col = col;
		}
	}
	else
	{
		col -= CELL_SIZE;
		row = row;
	}

	for (int i = row; i < 2 * CELL_SIZE && i < MSIZE; i++)
		for (int j = col; j < 2 * CELL_SIZE && j < MSIZE; j++)
		{
			if (maze[i][j] == WALL)
				return false;
		}


	return true;
}

void storeCurrentPointInParrentArray(int row, int col, Point2D* parentPoint, Point2D* parentArray[][MSIZE], vector<Point2D*> &grayArray)
{
	Point2D* ptAddToGray = new Point2D(col, row);
	parentArray[row][col] = parentPoint;
	grayArray.push_back(ptAddToGray);
}

bool setPointAsGray(int& mazeRow, int& mazeCol, Point2D*& parentPoint, Point2D* parentArray[][MSIZE], vector<Point2D*> &grayVector)
{
	if ((maze[mazeRow][mazeCol] == SPACE) && checkPointCloseToWall(mazeRow, mazeCol)
		/*((maze[mazeRow + 2 * CELL_SIZE][mazeCol] != WALL) && (maze[mazeRow][mazeCol + 2 * CELL_SIZE] != WALL)
		&& (maze[mazeRow - 2 * CELL_SIZE][mazeCol] != WALL) && (maze[mazeRow][mazeCol - 2 * CELL_SIZE] != WALL))*/
		)
	{
		maze[mazeRow][mazeCol] = COIN;	//add it to gray
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
	int step = 4 * CELL_SIZE;

	if (gray_coins.empty())	//grey is the edges that didn't visited yet
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
			if (maze[mazeRow][mazeCol] != COIN)
				maze[mazeRow][mazeCol] = COIN;	//y is i, x is j

			//check down
			mazeRow = pt->getY() + step;
			if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
			{
				//check up
				mazeRow = pt->getY() + step;
				if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
				{
					//check right
					mazeRow = pt->getY();
					mazeCol = pt->getX() + step;
					if (setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins))
					{
						//check left
						mazeCol = pt->getX() - step;
						setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

					}
				}
			}

			//mazeRow = pt->getY() + step;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			//	//check up
			//mazeRow = pt->getY() + step;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			//		//check right
			//mazeRow = pt->getY();
			//mazeCol = pt->getX() + step;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);
			//
			////check left
			//mazeCol = pt->getX() - step;
			//setPointAsGray(mazeRow, mazeCol, pt, parent_forCoins, gray_coins);

		}
	}
}

void setupCoins()
{
	coinsBfs = true;
	while (coinsBfs)
		bfsIteration();
}

void init()
{

	srand(time(0));
	setupMaze();

	gray_coins.push_back(pacmanStartPoint);

	setupCoins();

	glClearColor(0.7, 0.7, 0.7, 0);

	//glOrtho(-1, 1, -1, 1, -1, 1);
	glOrtho(0, MSIZE, 0, MSIZE, 0, MSIZE);    //regular origin!!! y from bottom to up, x from left to right

}

void moveFigureOnXAxis()
{
	int x = pacman.getPacmanLocation()->getX();
	int y = pacman.getPacmanLocation()->getY();

	PacmanPawn::pacmanDirection pacDir = pacman.getDirection();

	switch (pacDir)
	{
	case PacmanPawn::pacmanDirection::Right:
		break;
	default:
		break;
	}

	if ((maze[y - 2 * CELL_SIZE][(x + 1) + CELL_SIZE] != WALL) && (maze[y + 2 * CELL_SIZE][(x + 1) + CELL_SIZE] != WALL)
		&& (maze[y][(x + 1) + CELL_SIZE] != WALL))
	{
		x++;
		if (x >= MSIZE)
			x = 0;
		if (fmod(floor(x), 10) == 0)
			pacman.changeIsOpen();
		pacman.setTranslation(PacmanPawn::pacmanDirection::Right, new Point2D(x, y));
	}
	else if ((maze[(y + 1) + CELL_SIZE][x - 2 * CELL_SIZE] != WALL) && (maze[(y + 1) + CELL_SIZE][x + CELL_SIZE] != WALL)
		&& (maze[(y + 1) + CELL_SIZE][x] != WALL))
	{
		y++;
		if (fmod(floor(y), 10) == 0)
			pacman.changeIsOpen();

		pacman.setTranslation(PacmanPawn::pacmanDirection::Up, new Point2D(x, y));
	}



}

void drawCoin(int row, int col)
{
	glPushMatrix();
	glTranslated(row, col, 0);
	glScaled(CELL_SIZE, CELL_SIZE, 1);

	double PI = 3.14;
	double alpha, x, y, radius = 1, delta = PI / 20;

	//draw the outline of the wheel
	glBegin(GL_POLYGON);
	for (alpha = 0; alpha <= 2 * PI; alpha += delta)
	{
		x = radius * cos(alpha);
		y = radius * sin(alpha);
		glVertex2d(x, y);
	}
	glEnd();

	glPopMatrix();
}


//void setPointAsGrayForAStar(int& mazeRow, int& mazeCol, Point2D*& parentPoint)
//{
//    if (isBfsFoundPath(mazeRow, mazeCol, TARGET, VISITED_FROM_TARGET))    //found target
//        storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint);
//    else if (maze[mazeRow][mazeCol] == SPACE)
//    {
//        maze[mazeRow][mazeCol] = GRAY;
//        storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint);
//    }
//}
//
//void a_starIteration()
//{
//    Point2D* pt;
//    int mazeRow, mazeCol;
//
//    if (grayPQ.empty())    //grey is the edges that didn't visited yet
//        aStar_started = false;    //there is no path to the target
//    else
//    {
//        pt = grayPQ.top();    //this will be the parent
//        grayPQ.pop();
//
//        mazeRow = pt->getY();
//        mazeCol = pt->getX();
//
//        //paint pt VISITED
//        if (isBfsFoundPath(mazeRow, mazeCol, TARGET, VISITED_FROM_TARGET))    //found target
//            storeCurrentPointForAstar(mazeRow, mazeCol, pt);
//
//        else
//        {
//            if (maze[mazeRow][mazeCol] != START)
//                maze[mazeRow][mazeCol] = VISITED_FROM_START;    //y is i, x is j
//
//            //check down
//            mazeRow = pt->getY() + 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//            //check up
//            mazeRow = pt->getY() - 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//            //check right
//            mazeRow = pt->getY();
//            mazeCol = pt->getX() + 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//            //check left
//            mazeCol = pt->getX() - 1;
//            setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//            if (!aStar_started)    //target was found
//                showPath(pt, START, TARGET, parent_forStartPath);
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
				glColor3d(0.2, 0, 1);    //brown
				break;
			case SPACE:
				glColor3d(0, 0, 0);    //white
				break;
			case COIN:
				glColor3d(1, 1, 1);
				//drawCoin(i, j);
				break;
			case UNREACHABLE:
				glColor3d(0, 0, 0);
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
	stepCounter++;
	if (stepCounter % 10 == 0)
		moveFigureOnXAxis();


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
