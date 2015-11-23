// OpenGLApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include "glut.h"
#include <gl/gl.h>

using namespace std;

int screen_width=640;
int screen_height=480;

void initOpenGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, screen_width, screen_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 1.0f, 1000.0f);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHT0);
}

class RectangularPiece
{
private:
	int positionX;
	int positionZ;
	float positionY;

	int initialSizeX;
	int initialSizeZ;
	float height;

	float color[3];
public:
	RectangularPiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, float color[])
	{
		this->positionX = positionX;
		this->positionY = positionY;
		this->positionZ = positionZ;
		initialSizeX = sizeX;
		initialSizeZ = sizeZ;
		this->height = height;
		this->color[0] = color[0];
		this->color[1] = color[1];
		this->color[2] = color[2];
	}

	int getSizeX()
	{
		return initialSizeX;
	}

	int getSizeZ()
	{
		return initialSizeZ;
	}

	float getHeight()
	{
		return height;
	}

	int getX()
	{
		return positionX;
	}

	int getZ()
	{
		return positionZ;
	}

	float getY()
	{
		return positionY;
	}

	void setX(int newPositionX)
	{
		positionX = newPositionX;
	}

	void setY(float newPositionY)
	{
		positionY = newPositionY;
	}

	void setZ(int newPositionZ)
	{
		positionZ = newPositionZ;
	}

	float getRed()
	{
		return color[0];
	}

	float getGreen()
	{
		return color[1];
	}

	float getBlue()
	{
		return color[2];
	}
};

void draw(RectangularPiece piece)
{
	GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 0.2 };
	GLfloat mat_diffuse[] = { piece.getRed(), piece.getGreen(), piece.getBlue(), 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 100.0 };
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	int sizeX = piece.getSizeX();
	int sizeZ = piece.getSizeZ();
	float height = piece.getHeight();

	float vertices[] = {
		0.0f,   0.0f,  0.0f,
		sizeX,   0.0f,  0.0f,
		sizeX,   0.0f, sizeZ,
		0.0f,   0.0f, sizeZ,
		0.0f, height,  0.0f,
		sizeX, height,  0.0f,
		sizeX, height, sizeZ,
		0.0f, height, sizeZ
	};

	unsigned char indices[] = {
		0, 1, 2, 3,
		0, 4, 5, 1,
		1, 5, 6, 2,
		3, 2, 6, 7,
		4, 0, 3, 7,
		4, 5, 6, 7
	};

	glPushMatrix();
	glTranslatef(piece.getX(), piece.getY(), piece.getZ());

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);
	glDisableClientState(GL_VERTEX_ARRAY);

	for (int i = 0; i < sizeX; i++) {
		float x = i + 0.5f;
		for (int j = 0; j < sizeZ; j++) {
			float z = j + 0.5f;

			glPushMatrix();
			{
				glTranslatef(x, piece.getHeight() + 0.125f, z);
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

				/*{
					GLUquadric* quad = gluNewQuadric();
					gluDisk(quad, 0.0, 0.25, 30, 1);
					gluDeleteQuadric(quad);
				}*/

				/*{
					GLUquadric* quad = gluNewQuadric();
					gluCylinder(quad, 0.25, 0.25, 0.125, 30, 1);
					gluDeleteQuadric(quad);
				}*/
			}
			glPopMatrix();
		}
	}
	glPopMatrix();
}

inline bool inside(int regionX, int regionZ, int sizeX, int sizeZ, int pointX, int pointZ)
{
	if(pointX > regionX && pointX < regionX + sizeX && pointZ >= regionZ && pointZ <= regionZ + sizeZ) {
		return true;
	}
	return false;
}

inline bool intervalIntersects(int interval1X1, int interval1X2, int interval2X1, int interval2X2)
{
	if (interval1X1 >= interval1X2) {
		swap(interval1X1, interval1X2);
	}
	if (interval2X1 >= interval2X2) {
		swap(interval2X1, interval2X2);
	}
	// Interval 1 to the left of interval 2
	if (interval1X2 <= interval2X1) {
		return false;
	}
	// Interval 1 to the right
	if (interval1X1 >= interval2X2) {
		return false;
	}
	return true;
}

inline bool verticalSegmentsIntersect(int line1X1, int line1Z1, int line1X2, int line1Z2, int line2X1, int line2Z1, int line2X2, int line2Z2)
{
	if (line1X1 == line1X2 && line2X1 == line2X2 && line1X1 == line2X1
		&& intervalIntersects(line1Z1, line1Z2, line2Z1, line2Z2)) {
			return true;
	}
	return false;
}

inline bool horizontalSegmentsIntersect(int line1X1, int line1Z1, int line1X2, int line1Z2, int line2X1, int line2Z1, int line2X2, int line2Z2)
{
	if (line1Z1 == line1Z2 && line2Z1 == line2Z2 && line1Z1 == line2Z1
		&& intervalIntersects(line1X1, line1X2, line2X1, line2X2)) {
			return true;
	}
	return false;
}

bool overlaps(int x, int z, int sizeX, int sizeZ, RectangularPiece& piece)
{
	if(inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ() + piece.getSizeZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ()) ||
		inside(piece.getX(), piece.getZ(), piece.getSizeX(), piece.getSizeZ(), x, z) ||
		horizontalSegmentsIntersect(x, z, x + sizeX, z, piece.getX(), piece.getZ(), piece.getX() + piece.getSizeX(), piece.getZ()) ||
		horizontalSegmentsIntersect(x, z + sizeZ, x + sizeX, z + sizeZ, piece.getX(), piece.getZ() + piece.getSizeZ(), piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ()) ||
		verticalSegmentsIntersect(x, z, x, z + sizeZ, piece.getX(), piece.getZ(), piece.getX(), piece.getZ() + piece.getSizeZ()) ||
		verticalSegmentsIntersect(x + sizeX, z, x + sizeX, z + sizeZ, piece.getX() + piece.getSizeX(), piece.getZ(), piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ())) {
			return true;
	}
	return false;
}

class PiecesContainer
{
private:
	vector<RectangularPiece> pieces;

public:
	void addPiece(RectangularPiece& piece)
	{
		pieces.push_back(piece);
	}
	float findMaxY(int x, int z, int sizeX, int sizeZ)
	{
		vector<RectangularPiece>::iterator it;
		float maxY = 0.0f;
		for(it = pieces.begin(); it != pieces.end(); it++) {
			if(overlaps(x, z, sizeX, sizeZ, *it)) {
				float y = it->getY() + it->getHeight();
				if(y > maxY) {
					maxY = y;
				}
			}
		}
		return maxY;
	}

	void drawPieces()
	{
		vector<RectangularPiece>::iterator it;
		for(it = pieces.begin(); it != pieces.end(); it++) {
			draw(*it);
		}
	}
};

float GROUND_COLOR[3] = {1.0f, 0.0f, 0.0f};
RectangularPiece ground(-5, -5, 10, 10, -0.125, 0.125, GROUND_COLOR);

float PIECE_COLOR[3] = {0.0f, 0.7f, 0.0f};
RectangularPiece piece(0, 0, 2, 4, 1, 0.5, PIECE_COLOR);


float PIECE2_COLOR[3] = {0.0f, 0.0f, 0.7f};
RectangularPiece piece2(-3, -3, 2, 4, 1, 0.5, PIECE2_COLOR);

PiecesContainer piecesContainer;

GLfloat y;
void renderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(0.0, 5.0, -9.0, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0);
	
	GLfloat light_position[] = { 0.0, 10.0, 0.0, 1.0 };
	GLfloat light_ambient[] = { 0.1, 0.1, 0.1, 0.1 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_spot_direction[] = { 0.0, -1.0, 0.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	piecesContainer.drawPieces();
	draw(piece2);

	glColor3f(0.0f, 1.0f, 0.0f);

	glutSwapBuffers();
}

void changeSize(int w, int h)
{
	screen_width=w;
	screen_height=h;

	if(h == 0)
		h = 1;

	float ratio = 1.0*w/h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -50.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
}

void processNormalKeys(unsigned char key, int __x, int __y)
{
	switch(key)
	{
	case 'w':
		//process
		piece2.setZ(piece2.getZ() + 1.0f);
		glutPostRedisplay();
		break;
	case 's':
		//process
		piece2.setZ(piece2.getZ() - 1.0f);
		glutPostRedisplay();
		break;
	case 'd':
		//process
		piece2.setX(piece2.getX() + 1.0f);
		glutPostRedisplay();
		break;
	case 'a':
		//process
		piece2.setX(piece2.getX() - 1.0f);
		glutPostRedisplay();
		break;
	case ' ':
		piece2.setY(piecesContainer.findMaxY(piece2.getX(), piece2.getZ(), piece2.getSizeX(), piece2.getSizeZ()));
		piecesContainer.addPiece(piece2);
		glutPostRedisplay();
		break;
	}
	piece2.setY(piecesContainer.findMaxY(piece2.getX(), piece2.getZ(), piece2.getSizeX(), piece2.getSizeZ()) + 1.0f);
}

int main(int argc, char* argv[])
{
	//Initialize the GLUT library
	glutInit(&argc, argv);
	//Set the display mode
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	//Set the initial position and dimensions of the window
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(screen_width, screen_height);
	//creates the window
	glutCreateWindow("OGEL");
	//Specifies the function to call when the window needs to be redisplayed
	glutDisplayFunc(renderScene);
	//Sets the idle callback function
	glutIdleFunc(renderScene);
	//Sets the reshape callback function
	glutReshapeFunc(changeSize);
	//Keyboard callback function
	glutKeyboardFunc(processNormalKeys);
	//Initialize some OpenGL parameters
	initOpenGL();

	piecesContainer.addPiece(ground);
	piecesContainer.addPiece(piece);
	//Starts the GLUT infinite loop
	glutMainLoop();
	return 0;
}

