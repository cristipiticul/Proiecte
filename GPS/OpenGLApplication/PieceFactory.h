#pragma once

#include "RectangularPiece.h"

class PieceFactory {
public:
	PieceFactory();
	RectangularPiece* getNewPiece();
	void nextPieceType();
private:
	enum PieceType {
		SIMPLE, EDGE, CORNER
	};
	PieceType pieceType;
};