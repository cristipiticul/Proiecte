#include "StdAfx.h"
#include "glut.h"
#include <gl/gl.h>
#include "RectangularPiece.h"

RectangularPiece::RectangularPiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, const float color[])
{
	this->positionX = positionX;
	this->positionY = positionY;
	this->positionZ = positionZ;
	this->sizeX = sizeX;
	this->sizeZ = sizeZ;
	this->height = height;
	setColor(color);
}

void RectangularPiece::setColor(const float color[]) {
	this->color[0] = color[0];
	this->color[1] = color[1];
	this->color[2] = color[2];

}

int RectangularPiece::getSizeX()
{
	return sizeX;
}

int RectangularPiece::getSizeZ()
{
	return sizeZ;
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

void RectangularPiece::setSizeX(int newSizeX)
{
	sizeX = newSizeX;
}

void RectangularPiece::setSizeZ(int newSizeZ)
{
	sizeZ = newSizeZ;
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

void RectangularPiece::setGLMaterial()
{
	GLfloat mat_ambient[] = { 0.2f, 0.2f, 0.2f, 0.2f };
	GLfloat mat_diffuse[] = { this->getRed(), this->getGreen(), this->getBlue(), 1.0f };
	GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_shininess[] = { 128.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}
