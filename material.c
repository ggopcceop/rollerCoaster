/*
Name: Shuwen Ruan
Banner: B00510391
*/
#include "material.h"

//set material of center rail
void
trackCenterMaterial(){
	GLfloat ambient[] = {.2125, .1275, .054, 1.0 };
	GLfloat diffuse[] = {.714, .4284, .18144, 1.0 };
	GLfloat specular[] = { .393548, .271906, .166721, 1.0 };
	GLfloat shine = 25.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}

//set material of side rails
void
trackSideMaterial(){
	GLfloat ambient[] = {.25, .25, .25, 1.0 };
	GLfloat diffuse[] = {.4, .4, .4, 1.0 };
	GLfloat specular[] = { .774597, .774597, .774597, 1.0 };
	GLfloat shine = 70.0;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shine);
}
