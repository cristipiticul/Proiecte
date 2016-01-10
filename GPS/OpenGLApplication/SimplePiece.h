#pragma once

#include "RectangularPiece.h"

class SimplePiece : public RectangularPiece {
public:
	SimplePiece(int positionX, int positionZ, int sizeX, int sizeZ, float positionY, float height, const float color[]);
	void rotate();
	void draw();
};