// OpenGLApplication.cpp : Defines the entry point for the console application.
//

#include "GL/glut.h"
#include <gl/gl.h>
#include <stdio.h>
#include <vector>

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
}

class RectangularPiece
{
private:
    float positionX;
    float positionZ;
    float positionY;

    float initialSizeX;
    float initialSizeZ;
    float height;

    float color[3];
public:
    RectangularPiece(float positionX, float positionZ, float sizeX, float sizeZ, float positionY, float height, float color[])
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

    float getSizeX()
    {
        return initialSizeX;
    }

    float getSizeZ()
    {
        return initialSizeZ;
    }

    float getHeight()
    {
        return height;
    }

    float getX()
    {
        return positionX;
    }

    float getZ()
    {
        return positionZ;
    }

    float getY()
    {
        return positionY;
    }

    void setX(float newPositionX)
    {
        positionX = newPositionX;
    }

    void setY(float newPositionY)
    {
        positionY = newPositionY;
    }

    void setZ(float newPositionZ)
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
    float sizeX = piece.getSizeX();
    float sizeZ = piece.getSizeZ();
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
    glColor3f(piece.getRed(), piece.getGreen(), piece.getBlue());
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);
    glDisableClientState(GL_VERTEX_ARRAY);

    for (float i = 0; i < sizeX; i++) {
        float x = i + 0.5f;
        for (float j = 0; j < sizeZ; j++) {
            float z = j + 0.5f;

            glPushMatrix();
            {
                glTranslatef(x, piece.getHeight() + 0.125f, z);
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

                {
                    GLUquadric* quad = gluNewQuadric();
                    gluDisk(quad, 0.0, 0.25, 30, 1);
                    gluDeleteQuadric(quad);
                }

                {
                    GLUquadric* quad = gluNewQuadric();
                    gluCylinder(quad, 0.25, 0.25, 0.125, 30, 1);
                    gluDeleteQuadric(quad);
                }
            }
            glPopMatrix();
        }
    }
    glPopMatrix();
}

class PiecesContainer
{
private:
    vector<RectangularPiece> pieces;

    bool inside(float regionX, float regionZ, float sizeX, float sizeZ, float pointX, float pointZ)
    {
        if(pointX > regionX && pointX < regionX + sizeX && pointZ >= regionZ && pointZ <= regionZ + sizeZ) {
            return true;
        }
        return false;
    }

    bool overlaps(float x, float z, float sizeX, float sizeZ, RectangularPiece& piece)
    {
        if(inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ()) ||
           inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ()) ||
           inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ() + piece.getSizeZ()) ||
           inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ()) ||
           inside(piece.getX(), piece.getZ(), piece.getSizeX(), piece.getSizeZ(), x, z)) {
            return true;
       }
       return false;
    }
public:
    void addPiece(RectangularPiece& piece)
    {
        pieces.push_back(piece);
    }
    float findMaxY(float x, float z, float sizeX, float sizeZ)
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

