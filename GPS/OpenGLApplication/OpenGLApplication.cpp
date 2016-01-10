// OpenGLApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include "glut.h"
#include <gl/gl.h>
#include "SimplePiece.h"
#include "CornerPiece.h"
#include "PiecesContainer.h"
#include "ColorGenerator.h"
#include "PieceFactory.h"

using namespace std;

int screen_width = 640;
int screen_height = 480;
bool left_button_down = false;
float camera_rotation_y = M_PI, camera_rotation_xz = -M_PI / 4;
const int ZOOM_MIN = 0;
const int ZOOM_MAX = 10;
/// The higher the zoom, the closer the camera will be to the center of the scene.
int zoom = (ZOOM_MIN + ZOOM_MAX) / 2;
const float DIST_MIN = 5.0f;
const float DIST_MAX = 20.0f;
const int GROUND_SIZE_X = 20;
const int GROUND_SIZE_Z = 20;

PieceFactory pieceFactory;
ColorGenerator colorGenerator;

bool wireframe = false;


void zoomIn() {
	if (zoom + 1 <= ZOOM_MAX) {
		zoom++;
	}
	else {
		zoom = ZOOM_MAX;
	}
}

void zoomOut() {
	if (zoom - 1 >= ZOOM_MIN) {
		zoom--;
	}
	else {
		zoom = ZOOM_MIN;
	}
}

void initOpenGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, screen_width, screen_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHT0);
}


float GROUND_COLOR[3] = { 1.0f, 0.0f, 0.0f };
SimplePiece ground(-GROUND_SIZE_X / 2, -GROUND_SIZE_Z / 2, GROUND_SIZE_X, GROUND_SIZE_Z, -0.125, 0.125, GROUND_COLOR);

float PIECE_COLOR[3] = { 0.0f, 0.7f, 0.0f };
SimplePiece piece(0, 0, 2, 4, 1, 0.5, PIECE_COLOR);

RectangularPiece *currentPiece;

PiecesContainer piecesContainer;

void initPieces()
{
	currentPiece = pieceFactory.getNewPiece();
	//currentPiece = new CornerPiece(-3, -3, 1, 0.5, colorGenerator.getCurrentColor());

	piecesContainer.addPiece(&ground);
	piecesContainer.addPiece(&piece);
}

static const GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
static const GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 0.1f };
static const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat material_zero[] = { 0.0f, 0.0f, 0.0f, 1.0f };

GLfloat y;
void renderScene(void)
{
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	float distance = DIST_MIN + (ZOOM_MAX - zoom + ZOOM_MIN) * (DIST_MAX - DIST_MIN) / (ZOOM_MAX - ZOOM_MIN);
	float eyeX = sin(camera_rotation_y) * distance * sin(camera_rotation_xz);
	float eyeY = cos(camera_rotation_xz) * distance;
	float eyeZ = cos(camera_rotation_y) * distance * sin(camera_rotation_xz);
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.5, 0.0, 0.0, 1.0, 0.0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, light_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, light_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, light_diffuse);
	glTranslatef(light_position[0], light_position[1], light_position[2]);
	glutSolidSphere(0.1, 10, 10);
	//disable emmision
	glMaterialfv(GL_FRONT, GL_EMISSION, material_zero);
	glPopMatrix();

	piecesContainer.drawPieces();
	currentPiece->draw();

	glColor3f(0.0f, 1.0f, 0.0f);

	glutSwapBuffers();
}

void changeSize(int w, int h)
{
	screen_width = w;
	screen_height = h;

	if (h == 0)
		h = 1;

	float ratio = 1.0*w / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, -50.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
}

void refreshCurrentPieceY() {
	currentPiece->setY(piecesContainer.findMaxY(currentPiece->getX(), currentPiece->getZ(), currentPiece->getSizeX(), currentPiece->getSizeZ()) + 1.0f);
}

void moveCurrentPieceAboveGround() {
	if (ground.getZ() < currentPiece->getZ()) {
		currentPiece->setZ(currentPiece->getZ() - 1);
	}
	else {
		currentPiece->setZ(ground.getZ());
	}

	if (ground.getZ() + ground.getSizeZ() > currentPiece->getZ() + currentPiece->getSizeZ()) {
		currentPiece->setZ(currentPiece->getZ() + 1);
	}
	else {
		currentPiece->setZ(ground.getZ() + ground.getSizeZ() - currentPiece->getSizeZ());
	}

	if (ground.getX() + ground.getSizeX() > currentPiece->getX() + currentPiece->getSizeX()) {
		currentPiece->setX(currentPiece->getX() + 1);
	}
	else {
		currentPiece->setX(ground.getX() + ground.getSizeX() - currentPiece->getSizeX());
	}

	if (ground.getX() < currentPiece->getX()) {
		currentPiece->setX(currentPiece->getX() - 1);
	}
	else {
		currentPiece->setX(ground.getX());
	}
}

void changeCurrentPieceColor() {
	currentPiece->setColor(colorGenerator.getNextColor());
}

void processNormalKeys(unsigned char key, int __x, int __y)
{
	switch (key)
	{
	case 'r':
	{
		currentPiece->rotate();
		moveCurrentPieceAboveGround();
		refreshCurrentPieceY();
		break;
	}
	case 'w':
		wireframe = !wireframe;
		break;
	case 't':
		pieceFactory.nextPieceType();
		{
			RectangularPiece *oldPiece = currentPiece;
			currentPiece = pieceFactory.getNewPiece();
			delete oldPiece;
		}
		refreshCurrentPieceY();
		break;
	case ' ':
		currentPiece->setY(piecesContainer.findMaxY(currentPiece->getX(), currentPiece->getZ(), currentPiece->getSizeX(), currentPiece->getSizeZ()));
		piecesContainer.addPiece(currentPiece);
		{
			RectangularPiece *oldPiece = currentPiece;
			currentPiece = pieceFactory.getNewPiece();
		}
		refreshCurrentPieceY();
		glutPostRedisplay();
		break;
	case 'c':
		changeCurrentPieceColor();
		break;
	case '+':
		zoomIn();
		break;
	case '-':
		zoomOut();
		break;
	}
}

void processSpecialKeys(int key, int x, int y)
{
	bool changed = false;
	switch (key) {
	case GLUT_KEY_UP:
		changed = true;
		currentPiece->setZ(currentPiece->getZ() - 1);
		break;
	case GLUT_KEY_DOWN:
		changed = true;
		currentPiece->setZ(currentPiece->getZ() + 1);
		break;
	case GLUT_KEY_RIGHT:
		changed = true;
		currentPiece->setX(currentPiece->getX() + 1);
		break;
	case GLUT_KEY_LEFT:
		changed = true;
		currentPiece->setX(currentPiece->getX() - 1);
		break;
	}
	if (changed) {
		refreshCurrentPieceY();
		moveCurrentPieceAboveGround();
		glutPostRedisplay();
	}
}

int prev_x;
int prev_y;
const int center_x = screen_width / 2;
const int center_y = screen_height / 2;
void mouseFunc(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			glutSetCursor(GLUT_CURSOR_NONE);
			left_button_down = true;
			x = center_x;
			y = center_y;
			prev_x = x;
			prev_y = y;
			glutWarpPointer(x, y);
		}
		else {
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			left_button_down = false;
		}
	}
}

void mouseMotionFunc(int x, int y)
{
	if (left_button_down) {
		camera_rotation_y += (x - prev_x) * 0.001953125f;
		camera_rotation_xz += (y - prev_y) * 0.001953125f;

		while (camera_rotation_y > 2 * M_PI) {
			camera_rotation_y -= 2 * M_PI;
		}
		while (camera_rotation_y < 0) {
			camera_rotation_y += 2 * M_PI;
		}
		if (camera_rotation_xz > -0.001953125f) {
			camera_rotation_xz = -0.001953125f;
		}
		if (camera_rotation_xz < -M_PI / 2) {
			camera_rotation_xz = -M_PI / 2;
		}
		if (abs(x - center_x) + abs(y - center_y) >= 20) {
			prev_x = center_x;
			prev_y = center_y;
			glutWarpPointer(center_x, center_y);
		}
		else {
			prev_x = x;
			prev_y = y;
		}
	}
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
	glutSpecialFunc(processSpecialKeys);

	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);

	//Initialize some OpenGL parameters
	initPieces();
	initOpenGL();

	//Starts the GLUT infinite loop
	glutMainLoop();
	return 0;
}

