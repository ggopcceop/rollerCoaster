/*
Name: Shuwen Ruan
Banner: B00510391
*/
#ifndef CURVE_H
#define CURVE_H

#include <GL/glut.h>
#include <string.h>
#include <stdio.h>

#include "demo.h"
#include "material.h"

//limits
#define MAX_CURVE_DEPTH 8
#define MAX_CIRCLE_POINT 6

//struct of a point
typedef struct Point{
	double x, y, z;
	struct Point* next;
} Point;

//struct of a circle
typedef struct Circle{
	struct Point point[MAX_CIRCLE_POINT + 1];
	struct Point center;
	struct Circle* next;
} Circle;

//struct of a curve
typedef struct Curve{
	struct Point* pointsBezier;
	struct Point* subdividedPoint;
	struct Circle* centerRail;
	struct Circle* leftRail;
	struct Circle* rightRail;
	struct Point* up;
	struct Curve* next;
} Curve;

//=====functions mamage curve======
void readPoints(void);
void createCurve(void);
void subdivideCurve(void);
void buildRail(void);
void subdivideBezierCurve(Curve*, int, double*, double*, double*, double*);

//update camera position
void cameraOnTrack(void);

//=====draw track======
void drawTrack(int);
void drawCurve(void);
void drawPillar(Curve*, int);

//=====helper functions=====
void pointToArray(Point*, double[3]);
void arrayToPoint(double[3], Point*);
void createCircle(Circle*, double, double, double, double, double*);

#endif
