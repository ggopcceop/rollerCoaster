/*
Name: Shuwen Ruan
Banner: B00510391

This is a roller coaster demo program that draw a roller coaster
based on a b-spline curve
*/
#ifndef DEMO_H
#define DEMO_H

#include <GL/glut.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

//curve related functions
#include "curve.h"
//stored material data
#include "material.h"

#define PI 3.14159265358979323846

//struct to stored points and curves
typedef struct Curve Curve;
typedef struct Point Point;

//======callback functions======
void myDisplay(void);
void myTimer(int);
void myKeyboard(unsigned char, int, int);

//======helper functions======
double normalize(double[3]);
void minus(double[3], double[3], double[3]);
void cross(double[3], double[3], double[3]);
double dot(double[3], double[3]);
double* myLookAt(double[3], double[3], double[3]);
void myMultVertex(double*, double*);
void myMultMatrix(double*, double*, double*);

#endif
