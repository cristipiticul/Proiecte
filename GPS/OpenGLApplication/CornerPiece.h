#pragma once

#include "RectangularPiece.h"

class CornerPiece : public RectangularPiece {
public:
	CornerPiece(int positionX, int positionZ, float positionY, float height, const float color[]);
	void rotate();
	void draw();
private:
	enum CornerOrientation {
		LEFT_UP, LEFT_DOWN, RIGHT_DOWN, RIGHT_UP
	};
	CornerOrientation orientation;
};