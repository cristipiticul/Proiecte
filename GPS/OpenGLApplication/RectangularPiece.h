#ifndef RECTANGULAR_PIECE_H
#define RECTANGULAR_PIECE_H

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
	RectangularPiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, float color[]);
	int getSizeX();
	int getSizeZ();
	float getHeight();
	int getX();
	int getZ();
	float getY();
	void setX(int newPositionX);
	void setY(float newPositionY);
	void setZ(int newPositionZ);
	float getRed();
	float getGreen();
	float getBlue();
	void draw();
};

#endif