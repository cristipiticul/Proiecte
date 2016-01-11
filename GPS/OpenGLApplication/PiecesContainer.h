#pragma once

#include "RectangularPiece.h"
#include <vector>

using namespace std;

class PiecesContainer
{
private:
	vector<RectangularPiece*> pieces;

public:
	void addPiece(RectangularPiece* piece);
	RectangularPiece* removeLast();
	float findMaxY(int x, int z, int sizeX, int sizeZ);
	void drawPieces();
	size_t size();
};