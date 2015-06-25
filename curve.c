/*
Name: Shuwen Ruan
Banner: B00510391
*/
#include "curve.h"

//=====helper functions=======
double* minPoint(double*, double*);
Point* getNextPointFromCurve(Curve*, Curve*, Point*);
Circle* getNextCircleFromCurve(Curve*, Curve*, Circle*, int);
Point* getNextPoint(Point*, Point*);
Curve* getNextCurve(Curve*, Curve*);

Point* dataPoint = NULL;
Curve* head = NULL;
Curve* currCameraCurve = NULL;
Point* currCameraPoint = NULL;
Point* currCameraUpPoint = NULL;
Point* temp = NULL;
int countAt = 0, countEye = 0, currUp = 1;
double eyeX, eyeY, eyeZ, atX, atY, atZ, upX, upY, upZ, speed = .05;
double step[3], stepTo[3];
double rotateMatrix[16];

//read points from file
void
readPoints(){
	double x, y, z;
	int count = 0;
	FILE* fp;
	Point* last;
	Point* curr;
	
	//open data.txt and read
	fp = fopen("data.txt", "r");
	
	//scan until end of file
	while(fscanf(fp, "%lf %lf %lf", &x, &y, &z) != EOF){
		//store data point to a linked list
		curr = malloc(sizeof(Point));
		curr->x = x;
		curr->y = y;
		curr->z = z;
		curr->next = NULL;
		if(dataPoint == NULL){
			dataPoint = curr;
			last = curr;
		}
		else{
			last->next = curr;
			last = curr;
		}
		count++;
	}
	fclose(fp);	
}

/*create the curve by read points
convert B-spline curve to Bezier curve
*/
void
createCurve(){
	Point* currPoint = dataPoint, * p0, * p1, * p2, * p3;
	Curve* currCurve, * lastCurve = NULL;
	Point* pArray;
	while(currPoint != NULL){
		//get 4 control points each time
		p0 = getNextPoint(dataPoint, currPoint);
		p1 = getNextPoint(dataPoint, p0);
		p2 = getNextPoint(dataPoint, p1);
		p3 = getNextPoint(dataPoint, p2);		
		
		//convert points to Bezier curve control points		
		currCurve = malloc(sizeof(Curve));
		pArray = malloc(sizeof(Point) * 4);		
		
		pArray[0].x = (p0->x / 6) + (p1->x * 2 / 3) + (p2->x / 6);
		pArray[0].y = (p0->y / 6) + (p1->y * 2 / 3) + (p2->y / 6);
		pArray[0].z = (p0->z / 6) + (p1->z * 2 / 3) + (p2->z / 6);
		
		pArray[1].x = (p1->x * 2 / 3) + (p2->x / 3);
		pArray[1].y = (p1->y * 2 / 3) + (p2->y / 3);
		pArray[1].z = (p1->z * 2 / 3) + (p2->z / 3);
		
		pArray[2].x = (p1->x / 3) + (p2->x * 2 / 3);
		pArray[2].y = (p1->y / 3) + (p2->y * 2 / 3);
		pArray[2].z = (p1->z / 3) + (p2->z * 2 / 3);
		
		pArray[3].x = (p1->x / 6) + (p2->x * 2 / 3) + (p3->x / 6);
		pArray[3].y = (p1->y / 6) + (p2->y * 2 / 3) + (p3->y / 6);
		pArray[3].z = (p1->z / 6) + (p2->z * 2 / 3) + (p3->z / 6);
		
		currCurve->pointsBezier = pArray;
		
		currCurve->subdividedPoint = NULL;
		currCurve->leftRail = NULL;
		currCurve->rightRail = NULL;
		currCurve->next = NULL;
		currCurve->up = NULL;
		
		if(lastCurve == NULL){
			lastCurve = currCurve;
			head = currCurve;
		}
		else{
			lastCurve->next = currCurve;
			lastCurve = currCurve;
		}
		
		currPoint = currPoint->next;
	}
}

/*
use de Casteljau’s algorithm to subdivide curve
*/
void
subdivideCurve(){
	Curve* curr = head; 
	double p0[3], p1[3], p2[3], p3[3];
	while(curr != NULL){
		//get 4 control point and convert to double array
		pointToArray(&curr->pointsBezier[0], p0);
		pointToArray(&curr->pointsBezier[1], p1);
		pointToArray(&curr->pointsBezier[2], p2);
		pointToArray(&curr->pointsBezier[3], p3);
		
		temp = NULL;
		
		//recursive subdivision
		subdivideBezierCurve(curr, 0, &p0[0], &p1[0], &p2[0], &p3[0]);
				
		curr = curr->next;
	}
}

/*
build the tracks
center rail, 2 side rails, pillars
and store every data points in world coordinate
*/
void
buildRail(){
	Curve* curr = head; 
	Point* currPoint, * next, * nextnext, * lastUpPoint, * upPoint;
	Circle* currCenterCircle, * lastCenterCircle = NULL;
	Circle* currLeftCircle, * lastLeftCircle = NULL;
	Circle* currRightCircle, * lastRightCircle = NULL;
	double p0[3], p1[3], p2[3], temp[3], up[3];
	double d0[3], d1[3], dd0[3];
	double* m, * rm, tempM[16];
	double r, k, trackTilt;
	
	while(curr != NULL){
		currPoint = curr->subdividedPoint;
		
		while(currPoint != NULL){
			//set up vertex
			up[0] = up[2] = 0;
			up[1] = 1;
			pointToArray(currPoint, p0);
			
			next = getNextPointFromCurve(head, curr, currPoint);
			pointToArray(next, p1);
			
			nextnext = getNextPointFromCurve(head, curr, next);
			pointToArray(nextnext, p2);
			
			//=====tilt the track======
			minus(p1, p0, d0);
			minus(p2, p1, d1);
			minus(d1, d0, dd0);
			
			r = sqrt(d0[0] * d0[0] + d0[1] * d0[1] + d0[2] * d0[2]);
			if(r < .001) k = 0;
			else k = (d0[2] * dd0[0] - d0[0] * dd0[2]) / (r * r * r);
			trackTilt += k * .3;
			trackTilt *= .9;
			
			//get the rotate matrix
			rm = malloc(sizeof(double) * 16);
			rm[2] = rm[3] = rm[6] = rm[7] = rm[8] = rm[9] =rm[11] = 0;
			rm[12] = rm[13] = rm[14] = 0;
			rm[10] = rm[15] = 1;
			rm[0] = rm[5] = cos(trackTilt);
			rm[1] = sin(trackTilt);
			rm[4] = -sin(trackTilt);
			
			//get the convert matrix
			m = myLookAt(&p0[0], &p1[0], &up[0]);
			
			//multiply 2 matrixes
			myMultMatrix(m, rm, &tempM[0]);
			
			//set the up vertex
			upPoint = malloc(sizeof(Point));			
			upPoint->x = tempM[4];
			upPoint->y = tempM[5];
			upPoint->z = tempM[6];
			upPoint->next = NULL;
			
			if(curr->up == NULL){
				curr->up = upPoint;
				lastUpPoint = upPoint;
			}
			else{
				lastUpPoint->next = upPoint;
				lastUpPoint = upPoint;
			}
			
			//=====center rail=======
			currCenterCircle = malloc(sizeof(Circle));
			arrayToPoint(p0, &currCenterCircle->center);
			
			//create a circle, r is .15
			//offset is 0 0 0 and convert object coordinate
			//to world coordinate
			createCircle(currCenterCircle, .15, 0, 0, 0, m);
			
			currCenterCircle->next = NULL;
			if(lastCenterCircle == NULL){
				curr->centerRail = currCenterCircle;
				lastCenterCircle = currCenterCircle;
			}
			else{
				lastCenterCircle->next = currCenterCircle;
				lastCenterCircle = currCenterCircle;
			}
			
			//=======left side rail========
			currLeftCircle = malloc(sizeof(Circle));
			//offset is -.5 .3 and 0
			temp[0] = -.5;
			temp[1] = .3;
			temp[2] = 0;
			//convert to world coordinate
			myMultVertex(&tempM[0], &temp[0]);
			//store the point
			arrayToPoint(temp, &currLeftCircle->center);
			
			//create a circle of left side rail
			createCircle(currLeftCircle, .15, -.5, .3, 0, &tempM[0]);
			
			currLeftCircle->next = NULL;
			if(lastLeftCircle == NULL){
				curr->leftRail = currLeftCircle;
				lastLeftCircle = currLeftCircle;
			}
			else{
				lastLeftCircle->next = currLeftCircle;
				lastLeftCircle = currLeftCircle;
			}
			
			//=======right side rail========
			currRightCircle = malloc(sizeof(Circle));
			//offset is .5 .3 and 0
			temp[0] = .5;
			temp[1] = .3;
			temp[2] = 0;
			//convert to world coordinate
			myMultVertex(&tempM[0], &temp[0]);
			//store the point
			arrayToPoint(temp, &currRightCircle->center);
			
			//create a circle of right side rail
			createCircle(currRightCircle, .15, .5, .3, 0, &tempM[0]);
			
			currRightCircle->next = NULL;
			if(lastRightCircle == NULL){
				curr->rightRail = currRightCircle;
				lastRightCircle = currRightCircle;
			}
			else{
				lastRightCircle->next = currRightCircle;
				lastRightCircle = currRightCircle;
			}
			
			free(m);
			free(rm);
			currPoint = currPoint->next;
		}
		lastCenterCircle = NULL;
		lastLeftCircle = NULL;
		lastRightCircle = NULL;
		curr = curr->next;
	}
}

/*recursive subdivision Bezier curve by 1/2
*/
void
subdivideBezierCurve(Curve* curve, int depth, double* p0,
			double* p1, double* p2, double* p3) {
	double* r0, * r1, * r2, * s0, * s1, * t0, tem[3];
	Point* pp0, * pp1, * pp2;
	double length;
	
	//get the length of point 1 to point 2
	minus(p1, p0, &tem[0]);
	length = normalize(&tem[0]);
	//if recursion to max depth or the length of 2 
	//points is less than 1, to exit the recursion
	if(depth >= MAX_CURVE_DEPTH || length < 1){
		//store the points to linked list
		pp0 = malloc(sizeof(Point));
		arrayToPoint(p0, pp0);
		
		pp1 = malloc(sizeof(Point));
		arrayToPoint(p1, pp1);
		
		pp2 = malloc(sizeof(Point));
		arrayToPoint(p2, pp2);
		
		pp0->next = pp1;
		pp1->next = pp2;
		pp2->next = NULL;
		
		if(temp == NULL){			
			curve->subdividedPoint = pp0;
			temp = pp2;
		}
		else{
			temp->next = pp0;
			temp = pp2;
		}
	}
	//or subdivided curve to 1/2 
	else{
		r0 = minPoint(p0, p1);
		r1 = minPoint(p1, p2);
		r2 = minPoint(p2, p3);
		
		s0 = minPoint(r0, r1);
		s1 = minPoint(r1, r2);
		
		t0 = minPoint(s0, s1);
		
		subdivideBezierCurve(curve, ++depth, p0, r0, s0, t0);
		subdivideBezierCurve(curve, ++depth, t0, s1, r2, p3);
		
		free(r0);
		free(r1);
		free(r2);
		free(s0);
		free(s1);
		free(t0);
	}
}

/* when riding roller coaster, update camera postion
based on the track
*/
void cameraOnTrack(){
	Point* next = currCameraPoint, * nextnext;
	double length = 0, speedMAX = .4, speedMIN = .1;
	double  p[3], at[3];
	int i;
	
	//dont update moving distance every frame
	//only when camera is get to the distance
	if(countEye == 0){		
		next = currCameraPoint;
		
		//set the speed of the roller coaster
		speed -= (step[1] * .01);
		if(speed > speedMAX) speed = speedMAX;
		if(speed < speedMIN) speed = speedMIN;
		at[0] = eyeX;
		at[1] = eyeY;
		at[2] = eyeZ;
		
		//get the next distance
		for(i = 0; i < 2; i++){
			if(next->next == NULL){			
				currCameraCurve = getNextCurve(head, currCameraCurve);
				next = currCameraCurve->subdividedPoint;
				currCameraUpPoint = currCameraCurve->up;
			}
			else{
				next = next->next;
				currCameraUpPoint = currCameraUpPoint->next;
			}
		}
		currCameraPoint = next;
		
		pointToArray(next, p);
		minus(p, at, step);
		length = normalize(step);
		countEye = length / speed;
			
	}
	
	//update the camera position
	eyeX += step[0] * speed;
	eyeY += step[1] * speed;
	eyeZ += step[2] * speed;
	
	//update the looking position
	nextnext = getNextPointFromCurve(head, currCameraCurve, 				currCameraPoint);
	atX += (nextnext->x - atX) / countEye;
	atY += (nextnext->y - atY) / countEye;
	atZ += (nextnext->z - atZ) / countEye;
	
	//update the up position
	upX += (currCameraUpPoint->x - upX) / countEye;
	upY += (currCameraUpPoint->y - upY) / countEye;
	upZ += (currCameraUpPoint->z - upZ) / countEye;
	
	countEye--;

}

//======functions that draw components======

//draw the cylinder based on 2 circles
void
drawCylinder(Circle* b, Circle* t, int type){
	double p0[3], p1[3], p2[3], p3[3], d0[3], d1[3], n[3];
	int i;
	
	//drawing by different display mode
	if(type == 2){
		glBegin(GL_LINE_STRIP);		
	}
	else{
		glBegin(GL_TRIANGLE_STRIP);	
	}
	if(type == 0){
		glColor3f(0, 0, 0);
	}
	//draw the strip by 2 circles
	for(i = 0; i <= MAX_CIRCLE_POINT; i++){
		pointToArray(&b->point[i], p0);
		pointToArray(&t->point[i], p1);
		if(i == 0){
			pointToArray(&b->point[MAX_CIRCLE_POINT], p2);
			pointToArray(&t->point[MAX_CIRCLE_POINT], p3);
		}
		else{
			pointToArray(&b->point[i - 1], p2);
			pointToArray(&t->point[i - 1], p3);
		}
		
		//compute normal up vertex
		minus(p2, p0, d0);
		minus(p3, p0, d1);
		cross(d0, d1, n);
		normalize(n);
		glNormal3dv(n);
		
		glVertex3dv(p0);
		
		//compute normal up vertex
		minus(p0, p1, d0);
		minus(p3, p1, d1);
		cross(d0, d1, n);
		normalize(n);
		glNormal3dv(n);
		
		glVertex3dv(p1);
	}
	glEnd();
}

//draw the track
void
drawTrack(int type){
	Curve* curr = head;
	Circle* currCenter, * currLeft, * currRight, * next;
	Circle tempCircle1, tempCircle2;
	double* m1, * m2, p1[3], p2[3], p3[3], p4[3];
	
	while(curr != NULL){
		currCenter = curr->centerRail;
		currLeft = curr->leftRail;
		currRight = curr->rightRail;
		
		while(currCenter != NULL){
		
			//=====draw center======
			trackCenterMaterial();
			
			next = getNextCircleFromCurve(head, curr, currCenter, 1);
			drawCylinder(currCenter, next, type);
			
			//======draw left side=====
			trackSideMaterial();
		
			next = getNextCircleFromCurve(head, curr, currLeft, 2);
			drawCylinder(currLeft, next, type);			
			
			//======draw right side======
			next = getNextCircleFromCurve(head, curr, currRight, 3);
			drawCylinder(currRight, next, type);
			
			//======draw support components======
			pointToArray(&currCenter->center, p1);
			pointToArray(&currLeft->center, p2);
			pointToArray(&currRight->center, p3);
			pointToArray(curr->up, p4);
			m1 = myLookAt(p1, p2, p4);
			m2 = myLookAt(p1, p3, p4);
			
			createCircle(&tempCircle1, .05, 0, 0, 0, m1);
			createCircle(&tempCircle2, .05, 0, 0, 1, m1);
			drawCylinder(&tempCircle1, &tempCircle2, type);
			
			createCircle(&tempCircle1, .05, 0, 0, 0, m2);
			createCircle(&tempCircle2, .05, 0, 0, 1, m2);			
			drawCylinder(&tempCircle1, &tempCircle2, type);
			
			free(m1);
			free(m2);
			
			currCenter = currCenter->next;
			currLeft = currLeft->next;
			currRight = currRight->next;
		}
		//======draw pillar========
		drawPillar(curr, type);
		
		curr = curr->next;
	}
}

//======draw the curve=======
void
drawCurve(){
	Curve* curr = head;
	Point* currPoint, * next;
	double p0[3], p1[3], v[3];
	
	//disable light for curve drawing
	glDisable(GL_LIGHTING);
	
	while(curr != NULL){
		currPoint = curr->subdividedPoint;
		while(currPoint != NULL){
			next = getNextPointFromCurve(head, curr, currPoint);
			
			//compute vector
			pointToArray(currPoint, p0);
			pointToArray(next, p1);
			minus(p1, p0, v);
			normalize(v);
			
			glPushMatrix();
			glTranslated(p0[0], p0[1], p0[2]);
			
			//draw the point
			glBegin(GL_POINTS);	
			glColor3f(.1, .1, .1);
			glVertex3d(0, 0, 0);
			glEnd();
			
			//draw the line vector
			glBegin(GL_LINES);	
			glColor3f(1, 1, 1);
			glVertex3d(0, 0, 0);
			glVertex3dv(v);
			glEnd();
			glPopMatrix();
			
			currPoint = currPoint->next;
		}		
		curr = curr->next;
	}
}

//draw the pillar
void
drawPillar(Curve* c, int type){
	Circle tempCircle1, tempCircle2;
	double p0[3], p1[3], p2[3], temp[3];
	double* m, * m2;
	int i;
	
	//======compute convert matrix=======
	pointToArray(c->subdividedPoint, &p0[0]);
	pointToArray(c->subdividedPoint->next, &p2[0]);
	
	p1[0] = p0[0];
	p1[1] = 0;
	p1[2] = p0[2];
	
	minus(&p0[0], &p2[0], &temp[0]);
	p2[0] = temp[0];
	p2[1] = temp[1];
	p2[2] = temp[2];
	
	m = myLookAt(&p0[0], &p1[0], &p2[0]);
	m[8] /= m[16];
	m[9] /= m[16];
	m[10] /= m[16];
	
	//======draw the left side pillar=========
	createCircle(&tempCircle1, .2, -1.5, 0, 0, m);
	createCircle(&tempCircle2, .2, -1.5, 0, p0[1], m);
	
	drawCylinder(&tempCircle1, &tempCircle2, type);
	
	if(type == 2) glBegin(GL_LINE_STRIP);		
	else glBegin(GL_POLYGON);
	glNormal3d(0, 1, 0);
	for(i = 0; i <= MAX_CIRCLE_POINT; i++){
		pointToArray(&tempCircle1.point[i], &temp[0]);
		glVertex3dv(&temp[0]);
	}
	glEnd();
	
	//======draw the right side pillar=========
	createCircle(&tempCircle1, .2, 1.5, 0, 0, m);
	createCircle(&tempCircle2, .2, 1.5, 0, p0[1], m);
	
	drawCylinder(&tempCircle1, &tempCircle2, type);
	
	if(type == 2) glBegin(GL_LINE_STRIP);		
	else glBegin(GL_POLYGON);
	glNormal3d(0, 1, 0);
	for(i = 0; i <= MAX_CIRCLE_POINT; i++){
		pointToArray(&tempCircle1.point[i], &temp[0]);
		glVertex3dv(&temp[0]);
	}
	glEnd();
	
	//=====draw horizontal pillar=======
	p0[0] = -1.5;
	p0[1] = 0;
	p0[2] = .2;
	
	myMultVertex(m, &p0[0]);
	
	p1[0] = 1.5;
	p1[1] = 0;
	p1[2] = .2;
	
	myMultVertex(m, &p1[0]);
	
	p2[0] = 0;
	p2[1] = 1;
	p2[2] = 0;
	
	m2 = myLookAt(&p0[0], &p1[0], &p2[0]);
	
	createCircle(&tempCircle1, .1, 0, 0, 0, m2);
	createCircle(&tempCircle2, .1, 0, 0, 1, m2);
	drawCylinder(&tempCircle1, &tempCircle2, type);

	free(m);
	free(m2);
}

//=======helper functions======

//get the mid point of the points
double*
minPoint(double* p0, double* p1){
	double* result = malloc(sizeof(double) * 3);
	result[0] = (p0[0] + p1[0]) * 0.5;
	result[1] = (p0[1] + p1[1]) * 0.5;
	result[2] = (p0[2] + p1[2]) * 0.5;
	
	return result;
}

//get the next point from a curve
//if end of curve get next curve's point
Point*
getNextPointFromCurve(Curve* h, Curve* c, Point* p){
	if(p->next == NULL){
		return getNextCurve(h, c)->subdividedPoint;
	}
	else{
		return p->next;
	}
}

//get the next circle from a curve
//if end of curve get next curve's point
Circle*
getNextCircleFromCurve(Curve* h, Curve* c, Circle* cc, int type){
	if(cc->next == NULL){
		switch(type){
		case 1:
			return getNextCurve(h, c)->centerRail;
		case 2:
			return getNextCurve(h, c)->leftRail;
		case 3:
			return getNextCurve(h, c)->rightRail;
		default:
			return NULL;
		}
	}
	else{
		return cc->next;
	}
}

//get the next point from a linked list
//if end of linked list get next from head
Point*
getNextPoint(Point* h, Point* p){
	if(p->next == NULL){
		return h;
	}
	else{
		return p->next;
	}
}

//get next curve from a linked list
//if end of linked list get next from head
Curve*
getNextCurve(Curve* h, Curve* c){
	if(c->next == NULL){
		return h;
	}
	else{
		return c->next;
	}
}

//convert a point to a double array
void
pointToArray(Point* p, double v[3]){
	v[0] = p->x;
	v[1] = p->y;
	v[2] = p->z;
}

//convert a double array to a point
void
arrayToPoint(double v[3], Point* p){
	p->x = v[0];
	p->y = v[1];
	p->z = v[2];
}

//create a circle by radius offset and converting matrix
void
createCircle(Circle* c, double r, double x, double y, double z,
			double* m){
	double theta = PI, temp[3];
	int i;
	for(i = 0; i <= MAX_CIRCLE_POINT; i++){
		temp[0] = r * sin(theta) + x;
		temp[1] = r * cos(theta) + y;
		temp[2] = z;
				
		myMultVertex(m, &temp[0]);
		arrayToPoint(&temp[0], &c->point[i]);
		
		theta += PI * 2 / MAX_CIRCLE_POINT;
	}
}
