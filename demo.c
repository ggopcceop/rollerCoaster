/*
Name: Shuwen Ruan
Banner: B00510391

This is a roller coaster demo program that draw a roller coaster
based on a b-spline curve
*/
#include "demo.h"

void init(void);

Point* dataPoint;
Curve* head;
Curve* currCameraCurve;
Point* currCameraPoint;
Point* currCameraUpPoint;

float df = 0;
int displayMode = 0;
int cameraMode = 0;
int countEye;

//for gluLookAt
double eyeX, eyeY, eyeZ, atX, atY, atZ, upX, upY, upZ;

//display list to draw the track
static GLuint displayTrack, displayCurve, displayLine, displayShadow;

int
main(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutInitWindowSize(1000, 500);
	glutCreateWindow("Roller Coaster");
	
	glutDisplayFunc(myDisplay);
	glutTimerFunc(33, myTimer, 0);
	glutKeyboardFunc(myKeyboard);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	
	//initialize data
	init();
	
	glutMainLoop();
	return 0;
}

/*
initialize data. creating display list
*/
void
init(){	
	//======create curve========
	
	//read points from file data.txt
	readPoints();	
	//create a curve based on control points
	createCurve();
	//subdivided points to Bezier Curves
	subdivideCurve();
	//use curve to build track
	buildRail();
	
	//=====build display list======
	
	//display list to diplay track as normal
	displayTrack = glGenLists(1);
    glNewList(displayTrack, GL_COMPILE);	
	drawTrack(1);	
    glEndList();
    
    //display list to display track as lines
    displayLine = glGenLists(1);
    glNewList(displayLine, GL_COMPILE);	
	drawTrack(2);	
    glEndList();
    
    //display list to display the curve
    displayCurve = glGenLists(1);
    glNewList(displayCurve, GL_COMPILE);	
	drawCurve();	
    glEndList();
    
    displayShadow = glGenLists(1);
    glNewList(displayShadow, GL_COMPILE);	
	drawTrack(0);
    glEndList();
    
    //=====light======
    
    //enable lighting
    glEnable(GL_LIGHT0);	
	GLfloat ambient[] = { 1, 1, 1, 1.0 };
	GLfloat diffuse[] = {.5, .5, .5, 1.0 };
	GLfloat specular[] = { .3, .3, .3, 1.0 };
	GLfloat position[] = { 1.0, 2.0, 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	
	//set default display mode to 0 (display the normal track)
	displayMode = 0;
	
	//set default look at data
	atX = atZ = upX = upY = upZ = 0;
	eyeY = 60;
	atY = 30;
	upY = 1;
	
	//set default line and point
	glLineWidth(1);
	glPointSize(6);
	glEnable(GL_POINT_SMOOTH);
}

//======callback functions======

void
myDisplay(){
	GLfloat m[16]= { 0.0 };
	int i;
	double step;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//======viewing======
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 2, 0.2, 300);
	
	//look up 1 above (better look when riding the roller)
	glTranslated(0, -1, 0);
	gluLookAt(eyeX, eyeY, eyeZ, atX, atY, atZ, upX, upY, upZ);
	
	
	//=====World Crood=====
	glMatrixMode(GL_MODELVIEW);
   	glLoadIdentity();
   	
   	//=====draw ground=====	
	
	glBegin(GL_POLYGON);
	glColor3f(0.4, .5, 0);
	glVertex3d(-200, 0, -200);
	glVertex3d(200, 0, -200);
	glVertex3d(200, 0, 200);
	glVertex3d(-200, 0, 200);    
	glEnd();
	
	//=====draw shadows====	
	if(displayMode == 0){
		glDisable(GL_DEPTH_TEST);
		m[0] = m[10] = m[15] = 200;
		m[4] = -100;
		m[6] = 100;
		m[7] = -1.0;
		glPushMatrix();
		glMultMatrixf(m);
		glCallList(displayShadow);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
	}
	
    //======draw sky======
	step = PI / 15;
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(.2313, .3568, .9608);
	glVertex3d(0, 200, 0);
	glColor3f(.7313, .8568, 1);
	for(i = 0; i <= 30; i++){		
		glVertex3d(200 * cos(i * step), 0, 200 * sin(i * step));
	}
	glEnd();
	
	
	//enable lighting for track
	glEnable(GL_LIGHTING);	
    
    //=====draw track=====
    switch(displayMode){
    case 0:
    	glCallList(displayTrack);
    	break;
    case 1:
    	glCallList(displayCurve);
    	break;
    case 2:
    	glCallList(displayLine);
    	break;
    default:
    	break;
    }
    
    glDisable(GL_LIGHTING);
    
	glutSwapBuffers();
}

void
myTimer(int value){	
	//update camera data
	if(cameraMode == 0){
		df += PI / 360;
		eyeX = 80 * sin(df);
		eyeZ = 80 * cos(df);
	}
	else{
		cameraOnTrack();
   	}
   	
	glutPostRedisplay();
	glutTimerFunc(33, myTimer, value);
}

void
myKeyboard(unsigned char key, int x, int y){
	switch(key){
	//key s for display surround looking to the track
	case 's':
		cameraMode = 0;
		eyeX = 30 * sin(df);
		eyeY = 80;
		eyeZ = 30 * cos(df);
		atX = atZ = upX = upZ = 0;
		atY = 40;
		upY = 1;		
		break;
	//key r to ride the roller
	case 'r':
		//set default data when start riding
		cameraMode = 1;
		currCameraCurve = head;
		currCameraPoint = currCameraCurve->subdividedPoint;
		currCameraUpPoint = currCameraCurve->up;
		eyeX = currCameraPoint->x;
		eyeY = currCameraPoint->y;
		eyeZ = currCameraPoint->z;
		atX = currCameraPoint->next->x;
		atY = currCameraPoint->next->y;
		atZ = currCameraPoint->next->z;
		upY = 1;
		countEye = 0;
		break;
	//key p to change display mode of the track
	case 'p':
		displayMode = (displayMode + 1) % 3;
		break;
	default:
		break;
	}
	
	glutPostRedisplay();
}

//========helper functions=========
//normalize a vertex
double
normalize(double v[3]) {
	double d;
	d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (d == 0.0) {
		return 0.0;
	}
	
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
	
	return d;
}

//minus vertex1 by vertex2
void
minus(double v1[3], double v2[3], double out[3]){
	out[0] = v1[0] - v2[0];
	out[1] = v1[1] - v2[1];
	out[2] = v1[2] - v2[2];
}

//get the cross product
void
cross(double v1[3], double v2[3], double out[3])
{
	out[0] = v1[1]*v2[2] - v1[2]*v2[1];
	out[1] = v1[2]*v2[0] - v1[0]*v2[2];
	out[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

//get the dot product
double
dot(double v1[3], double v2[3]){
	return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

/*look at method
get current point, pointing point and up vertex
to create a matrix for transform vertex from object
coordinate to world coordinate
*/
double*
myLookAt(double p0[3], double p1[3], double up[3]){
	double n[3], u[3], v[3], length;
	double* m = malloc(sizeof(double) * 17);
	
	minus(p1, p0, n);
	
	length = normalize(n);	
	
	cross(n, up, u);
	
	normalize(u);

	cross(u, n, v);
	
	m[0] = u[0];
	m[1] = u[1];
	m[2] = u[2];

	m[4] = v[0];
	m[5] = v[1];
	m[6] = v[2];

	m[8] = n[0] * length;
	m[9] = n[1] * length;
	m[10] = n[2] * length;

	m[12] = p0[0];
	m[13] = p0[1];
	m[14] = p0[2];

	m[15] = 1;
	m[3] = m[7] = m[11] = 0;
	m[16] = length;
	
	return m;
}

//get the vertex multiplied by a matrix
void
myMultVertex(double* m, double* v){
	double result[3];
	result[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + m[12];
	result[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + m[13];
	result[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + m[14];
	
	v[0] = result[0];
	v[1] = result[1];
	v[2] = result[2];
}

//get multiplication of two matrixes
void
myMultMatrix(double* m1, double* m2, double* out){
	int i, j, k;
	for(i = 0; i < 16; i++) out[i] = 0;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			for(k = 0; k < 4; k++){
				out[j*4 + i] += m1[k*4 + i] *  m2[j*4 + k];
			}
		}
	}
}
