#pragma once

class RectangularPiece
{
private:
	int positionX;
	int positionZ;
	float positionY;

	int sizeX;
	int sizeZ;
	float height;

	float color[3];
public:
	RectangularPiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, const float color[]);
	void setColor(const float color[]);
	int getSizeX();
	int getSizeZ();
	float getHeight();
	int getX();
	int getZ();
	float getY();
	void setX(int newPositionX);
	void setY(float newPositionY);
	void setZ(int newPositionZ);
	void setSizeX(int newSizeX);
	void setSizeZ(int newSizeZ);
	float getRed();
	float getGreen();
	float getBlue();
	void setGLMaterial();
	virtual void rotate() = 0;
	virtual void draw() = 0;
};
