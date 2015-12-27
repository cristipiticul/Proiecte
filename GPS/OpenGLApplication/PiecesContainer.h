#ifndef PIECES_CONTAINER_H
#define PIECES_CONTAINER_H

#include "stdafx.h"
#include "RectangularPiece.h"
#include <vector>

using namespace std;

class PiecesContainer
{
private:
	vector<RectangularPiece> pieces;

public:
	void addPiece(RectangularPiece& piece);
	float findMaxY(int x, int z, int sizeX, int sizeZ);
	void drawPieces();
};

#endif