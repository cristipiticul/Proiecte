#pragma once

#include "RectangularPiece.h"

class EdgePiece : public RectangularPiece {
public:
	EdgePiece(int positionX, int positionZ, float positionY, float height, const float color[]);
	void rotate();
	void draw();
private:
	enum EdgeOrientation {
		LEFT, DOWN, RIGHT, UP
	};
	EdgeOrientation orientation;
};