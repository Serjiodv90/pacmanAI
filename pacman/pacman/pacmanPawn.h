#pragma once
#ifndef _PACMANPAW_H
#define _PACMANPAW_H

#include "GLUT.h"
#include <vector>
#include <math.h>
#include "Point2D.h"

using namespace std;



class PacmanPawn
{

private:
	bool isOpen = false;
	Point2D* translationPoint;
	double scale;
	vector<double> rotationVector;
	void drawPacmanClosed();
	void drawPacmanOpen();

public:
	PacmanPawn(Point2D* startPoint, double scale);
	~PacmanPawn();

	void setIsOpen(bool isOpen);
	void changeIsOpen();
	void drawPacman();
	Point2D* getPacmanLocation();
	void setTranslation(Point2D* toPoint);
};


#endif // !_PACMANPAW_H
