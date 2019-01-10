#include "GLUT.H"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <queue>
#include <Windows.h>
#include "Point2D.h"
#include "pacmanPawn.h"
#include "Monster.h"

using namespace std;

const int MSIZE = 600;
const int W = MSIZE; // window width
const int H = MSIZE; // window height

const int SPACE = 1;
const int WALL = 2;
insert error!!!
/*******************************************************************/
/*		ALWAYS WORK WITH MULTIPLICATION OF CELL_SIZE IN THE MAZE
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

enum wallSide {Top, Bottom, Left, Right};	//the position of the wall

//gray queue for BFS algorithm
vector<Point2D*> gray_start;
vector<Point2D*> gray_target;
//Point2D* startPoint, *targetPoint;

Point2D* pacmanStartPoint = new Point2D(300, 250);

PacmanPawn pacman(pacmanStartPoint, 2*CELL_SIZE);
vector<double>tmpColor = { 0,1,1 };

Monster monster1(vector<double>({ 0, 1, 1 }) , new Point2D(300, 300), 2*CELL_SIZE);


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

void setupPerimeter()
{
	int i, j, k;

	//calclate the second y coordinate in maze of the horizontal line from bottom up
	int middleDoubleSpaceCells = (((MSIZE / 3) - SPACE_SIZE) / 2);
	middleDoubleSpaceCells -= middleDoubleSpaceCells % CELL_SIZE;	//need to be multiplication of CEEL_SIZE

	for (i = 0; i < MSIZE; i++)
	{
		if (i < MSIZE / 3 || i > (MSIZE / 3) * 2)	//only first third and last third are walls
		{
			thickenWallInMaze(Left, i);
			thickenWallInMaze(Right, i, MSIZE - CELL_SIZE);
			//for (j = 0; j < CELL_SIZE; j++)
			//{
			//	
			//	maze[i][j] = WALL;	// left walls
			//	maze[i][MSIZE - CELL_SIZE - j] = WALL;	// right bound walls of the maze
			//}
		}
		else
		{	//draw the left and right middle sections first lines
			if ((i == MSIZE / 3) || (i == 2 * MSIZE / 3))
			{
				
				//for the lower edge , it's positive, for the top edge is negative
				middleDoubleSpaceCells = (i == MSIZE / 3) ? middleDoubleSpaceCells : (middleDoubleSpaceCells * (-1));

				for (k = 0; k <= MSIZE / 6; k++)
				{
					
					//top and bottom bounds of the middle section ,left and right sides
					thickenWallInMaze(Bottom, i, k);
					thickenWallInMaze(Bottom, i, MSIZE - k - 1);
					thickenWallInMaze(Bottom, i + middleDoubleSpaceCells, k);
					thickenWallInMaze(Bottom, i + middleDoubleSpaceCells, MSIZE - k - 1);
					//for (j = 0; j < CELL_SIZE; j++)
					//{

					//	maze[i + j][k ] = WALL;	//bottom and top left
					//	maze[i + j][MSIZE - k - 1] = WALL;	//bottom and top right 

					//	maze[i + middleDoubleSpaceCells + j][k] = WALL;
					//	maze[i + middleDoubleSpaceCells + j][MSIZE - k - 1 - 1] = WALL;
					//}

				}
			}//set the bottom and top vertical walls in the middle section for left and right
			else if(((i >  MSIZE / 3) && (i < MSIZE / 3 + middleDoubleSpaceCells)) || 
				(i < 2 * MSIZE / 3) && (i > 2 * MSIZE / 3 - middleDoubleSpaceCells))//set the top vertical walls in the middle section for left and right
			{				
				thickenWallInMaze(Left, i, MSIZE / 6);
				thickenWallInMaze(Right, i, MSIZE - MSIZE / 6);
				//for (j = 0; j < CELL_SIZE; j++)
				//{
				//	maze[i][MSIZE / 6 + j] = WALL;	//left 
				//	maze[i][MSIZE - MSIZE / 6 - j] = WALL;	//right
				//}
			}
		}
		thickenWallInMaze(Bottom, -1, i);
		thickenWallInMaze(Top, MSIZE - CELL_SIZE, i);
		//for (j = 0; j < CELL_SIZE; j++)
		//{
		//	maze[j][i] = WALL;	//bottom wall
		//	maze[MSIZE - CELL_SIZE - j][i] = WALL;	// top bound walls of the maze
		//}
	}
}

void setupCenterSquare()
{
	int i, j;

	for (i = (MSIZE / 2) - SPACE_SIZE; i <= (MSIZE / 2) + SPACE_SIZE; i++)
	{
		//left wall
		thickenWallInMaze(Left, i, (MSIZE / 2) - (3 * SPACE_SIZE));
		//maze[i][(MSIZE / 2) - (3 * SPACE_SIZE)] = WALL;
		//right wall
		thickenWallInMaze(Right, i, (MSIZE / 2) + (3 * SPACE_SIZE));
		//maze[i][(MSIZE / 2) + (3 * SPACE_SIZE)] = WALL;

		/*if ((i == (MSIZE / 2) - SPACE_SIZE) || (i == (MSIZE / 2) + SPACE_SIZE))
		{
			for (j = (MSIZE / 2) - (3 * SPACE_SIZE); j < (MSIZE / 2) + (3 * SPACE_SIZE); j++)
			{
				if (i == (MSIZE / 2) + SPACE_SIZE)
				{
					if (j < (MSIZE / 2) - SPACE_SIZE || j >(MSIZE / 2) + SPACE_SIZE)
						maze[i][j] = WALL;
				}
				else maze[i][j] = WALL;
			}
		}*/
	}
}

void setupInitials()
{
	int i, j;

	for (i = MSIZE - 2 * SPACE_SIZE; i > MSIZE - 8 * SPACE_SIZE ;i--)
	{
		maze[i][(MSIZE / 2) - (3 * SPACE_SIZE)] = WALL;
		maze[i][(MSIZE / 2) + (3 * SPACE_SIZE)] = WALL;

		for (j = MSIZE/2 - 3 * SPACE_SIZE; j < MSIZE/2 + 3 * SPACE_SIZE; j++)
		{
			if (i == MSIZE - 5 * SPACE_SIZE)
					maze[i][j] = WALL;
		}
	}

	for (i = 2 * SPACE_SIZE; i <= 8 * SPACE_SIZE; i++)
	{
		if(i < 5 * SPACE_SIZE)
			maze[i][(MSIZE / 2) + (3 * SPACE_SIZE) - CELL_SIZE] = WALL;
		else if (i < 8 * SPACE_SIZE)
			maze[i][(MSIZE / 2) - (3 * SPACE_SIZE)] = WALL;

		for (j = MSIZE / 2 - 3 * SPACE_SIZE; j < MSIZE / 2 + 3 * SPACE_SIZE; j++)
		{
			if (i == 2 * SPACE_SIZE || i == 5 * SPACE_SIZE || i == 8 * SPACE_SIZE)
				maze[i][j] = WALL;
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



}





void init()
{

	srand(time(0));
	setupMaze();
	
	glClearColor(0.7, 0.7, 0.7, 0);

	//glOrtho(-1, 1, -1, 1, -1, 1);
	glOrtho(0, MSIZE, 0, MSIZE, 0, MSIZE);	//regular origin!!! y from bottom to up, x from left to right
	
}

void moveFigureOnXAxis()
{
	int x = pacman.getPacmanLocation()->getX();
	int y = pacman.getPacmanLocation()->getY();
	x++;
	if (maze[y][x] != WALL)
	{
		if (x >= MSIZE)
			x = 0;
		if (fmod(floor(x), 10) == 0)
			pacman.changeIsOpen();

		pacman.setTranslation(new Point2D(x, y));		
	}


	

	
}


//void setPointAsGrayForAStar(int& mazeRow, int& mazeCol, Point2D*& parentPoint)
//{
//	if (isBfsFoundPath(mazeRow, mazeCol, TARGET, VISITED_FROM_TARGET))	//found target			
//		storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint);
//	else if (maze[mazeRow][mazeCol] == SPACE)
//	{
//		maze[mazeRow][mazeCol] = GRAY;
//		storeCurrentPointForAstar(mazeRow, mazeCol, parentPoint);
//	}
//}

//void a_starIteration()
//{
//	Point2D* pt;
//	int mazeRow, mazeCol;
//
//	if (grayPQ.empty())	//grey is the edges that didn't visited yet
//		aStar_started = false;	//there is no path to the target
//	else
//	{
//		pt = grayPQ.top();	//this will be the parent
//		grayPQ.pop();
//		
//		mazeRow = pt->getY();
//		mazeCol = pt->getX();
//
//		//paint pt VISITED
//		if (isBfsFoundPath(mazeRow, mazeCol, TARGET, VISITED_FROM_TARGET))	//found target	
//			storeCurrentPointForAstar(mazeRow, mazeCol, pt);
//
//		else
//		{
//			if (maze[mazeRow][mazeCol] != START)
//				maze[mazeRow][mazeCol] = VISITED_FROM_START;	//y is i, x is j
//
//			//check down
//			mazeRow = pt->getY() + 1;
//			setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//			//check up
//			mazeRow = pt->getY() - 1;
//			setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//			//check right
//			mazeRow = pt->getY();
//			mazeCol = pt->getX() + 1;
//			setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//			//check left
//			mazeCol = pt->getX() - 1;
//			setPointAsGrayForAStar(mazeRow, mazeCol, pt);
//
//			if (!aStar_started)	//target was found
//				showPath(pt, START, TARGET, parent_forStartPath);
//		}
//	}
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
				glColor3d(0.2, 0, 1);	//brown
				break;
			case SPACE:
				glColor3d(0, 0, 0);	//white
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



void main(int argc, char* argv[])
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
}