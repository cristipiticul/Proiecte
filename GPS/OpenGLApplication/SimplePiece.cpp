#include "glut.h"
#include <gl/gl.h>
#include "SimplePiece.h"

SimplePiece::SimplePiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, const float color[]) :
	RectangularPiece(positionX, positionZ, sizeX, sizeZ, positionY, height, color)
{
}

void SimplePiece::rotate()
{
	int size_x = getSizeX();
	int size_z = getSizeZ();
	setSizeX(size_z);
	setSizeZ(size_x);
}

void SimplePiece::draw()
{
	setGLMaterial();

	int sizeX = this->getSizeX();
	int sizeZ = this->getSizeZ();
	float height = this->getHeight();

	float vertices[] = {
		0.0f,   0.0f,  0.0f,
		(float)sizeX,   0.0f,  0.0f,
		(float)sizeX,   0.0f, (float)sizeZ,
		0.0f,   0.0f, (float)sizeZ,

		0.0f,   0.0f,  0.0f,
		0.0f, height, 0.0f,
		(float)sizeX, height, 0.0f,
		(float)sizeX,   0.0f,  0.0f,

		(float)sizeX,   0.0f,  0.0f,
		(float)sizeX, height,  0.0f,
		(float)sizeX, height, (float)sizeZ,
		(float)sizeX,   0.0f, (float)sizeZ,

		0.0f,   0.0f, (float)sizeZ,
		(float)sizeX,   0.0f, (float)sizeZ,
		(float)sizeX, height, (float)sizeZ,
		0.0f, height, (float)sizeZ,

		0.0f, height,  0.0f,
		0.0f,   0.0f,  0.0f,
		0.0f,   0.0f, (float)sizeZ,
		0.0f, height, (float)sizeZ,

		0.0f, height, 0.0f,
		(float)sizeX, height, 0.0f,
		(float)sizeX, height, (float)sizeZ,
		0.0f, height, (float)sizeZ,
	};

	unsigned char indices[] = {
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
		12, 13, 14, 15,
		16, 17, 18, 19,
		20, 21, 22, 23
	};

	float normals[] = {
		0.0f,  -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,
		0.0f,  -1.0f, 0.0f,

		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,
		0.0f,  0.0f, -1.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		0.0f,  1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	glPushMatrix();
	glTranslatef(this->getX(), this->getY(), this->getZ());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indices);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

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