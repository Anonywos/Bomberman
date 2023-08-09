#ifndef RENDERERS_H
#define RENDERERS_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <string>
#include <vector>

//#include "../game_classes/Level.h"

using namespace std;

void renderStrokeFontString(float x, float y, float z, void *font, string text) {
	glPushMatrix();
	glTranslatef(x, y,z);

	for (unsigned int i = 0; i < text.size(); i++) {
		glutStrokeCharacter(font, text.at(i));
	}

	glPopMatrix();
}

void renderBitMapCharacter(float x, float y, float z, void *font, string text, float r, float g, float b) {
	glPushMatrix();
	glBegin(GL_BITMAP);
	glColor3f(r, g, b);
	glRasterPos3f(x, y, z);
	for (unsigned int i = 0; i < text.size(); i++) {
		glutBitmapCharacter(font, text.at(i));
	}
	glEnd();
	glPopMatrix();
}

void renderMainMenu(int selectedMenuOption, float x, float y, float z, float optionsSpacing){
	glPushMatrix();
    
    glBegin(GL_BITMAP);
    renderBitMapCharacter(x, y, z, GLUT_BITMAP_TIMES_ROMAN_24, "Bomberman", 1.0, 0.0, 0.0);
	renderBitMapCharacter(x, y-5, z, GLUT_BITMAP_HELVETICA_18, "[ A ] - Start game", 1.0, 1.0, 1.0);
    renderBitMapCharacter(x, y-optionsSpacing, z, GLUT_BITMAP_HELVETICA_18, "[ S ] - Exit", 1.0, 1.0, 1.0);
    glEnd();
    
    glPopMatrix();
}

void renderGameOver(int selectedMenuOption, float x, float y, float z, float optionsSpacing){
	glPushMatrix();
    
    glBegin(GL_BITMAP);
    renderBitMapCharacter(x, y, z, GLUT_BITMAP_HELVETICA_18, " You Lost :( ", 1.0, 0.0, 0.0);
    renderBitMapCharacter(x, y-optionsSpacing, z, GLUT_BITMAP_HELVETICA_18, "[ A ] - Give Up", 1.0, 1.0, 1.0);
    glEnd();
    
    glPopMatrix();
}

void renderVictory(int selectedMenuOption, float x, float y, float z, float optionsSpacing){
	glPushMatrix();
    
    glBegin(GL_BITMAP);
    renderBitMapCharacter(x, y, z, GLUT_BITMAP_HELVETICA_18, " Congratulations!!! ", 1.0, 0.0, 0.0);
    renderBitMapCharacter(x, y-optionsSpacing, z, GLUT_BITMAP_HELVETICA_18, "[ A ] - Exit", 1.0, 1.0, 1.0);
    glEnd();
    
    glPopMatrix();
}
#endif
