#include "StdAfx.h"
#include "glut.h"
#include <gl/gl.h>
#include "RectangularPiece.h"

RectangularPiece::RectangularPiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, float color[])
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

void RectangularPiece::draw()
{
	GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 0.2f };
	GLfloat mat_diffuse[] = { this->getRed(), this->getGreen(), this->getBlue(), 1.0f };
	GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess[] = { 128.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

	int sizeX = this->getSizeX();
	int sizeZ = this->getSizeZ();
	float height = this->getHeight();

	float vertices[] = {
		0.0f,   0.0f,  0.0f,
		(float)sizeX,   0.0f,  0.0f,
		(float)sizeX,   0.0f, (float)sizeZ,
		0.0f,   0.0f, (float)sizeZ,
		0.0f, height,  0.0f,
		(float)sizeX, height,  0.0f,
		(float)sizeX, height, (float)sizeZ,
		0.0f, height,(float)sizeZ
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
	glTranslatef(this->getX(), this->getY(), this->getZ());

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
				glTranslatef(x, this->getHeight() + 0.125f, z);
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

				glPushMatrix();
				{
				    /// Rotate to have the good face up (for lighting)
					glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
					GLUquadric* quad = gluNewQuadric();
					gluDisk(quad, 0.0, 0.25, 30, 1);
					gluDeleteQuadric(quad);
				}
				glPopMatrix();

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

int RectangularPiece::getSizeX()
{
	return initialSizeX;
}

int RectangularPiece::getSizeZ()
{
	return initialSizeZ;
}

float RectangularPiece::getHeight()
{
	return height;
}

int RectangularPiece::getX()
{
	return positionX;
}

int RectangularPiece::getZ()
{
	return positionZ;
}

float RectangularPiece::getY()
{
	return positionY;
}

void RectangularPiece::setX(int newPositionX)
{
	positionX = newPositionX;
}

void RectangularPiece::setY(float newPositionY)
{
	positionY = newPositionY;
}

void RectangularPiece::setZ(int newPositionZ)
{
	positionZ = newPositionZ;
}

float RectangularPiece::getRed()
{
	return color[0];
}

float RectangularPiece::getGreen()
{
	return color[1];
}

float RectangularPiece::getBlue()
{
	return color[2];
}