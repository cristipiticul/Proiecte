#include "StdAfx.h"

#include "PiecesContainer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

using namespace std;

inline bool inside(int regionX, int regionZ, int sizeX, int sizeZ, int pointX, int pointZ)
{
	if (pointX > regionX && pointX < regionX + sizeX && pointZ > regionZ && pointZ < regionZ + sizeZ) {
		return true;
	}
	return false;
}

inline bool intervalIntersects(int interval1X1, int interval1X2, int interval2X1, int interval2X2)
{
	if (interval1X1 >= interval1X2) {
		swap(interval1X1, interval1X2);
	}
	if (interval2X1 >= interval2X2) {
		swap(interval2X1, interval2X2);
	}
	// Interval 1 to the left of interval 2
	if (interval1X2 <= interval2X1) {
		return false;
	}
	// Interval 1 to the right
	if (interval1X1 >= interval2X2) {
		return false;
	}
	return true;
}

inline bool verticalSegmentsIntersect(int line1X1, int line1Z1, int line1X2, int line1Z2, int line2X1, int line2Z1, int line2X2, int line2Z2)
{
	if (line1X1 == line1X2 && line2X1 == line2X2 && line1X1 == line2X1
		&& intervalIntersects(line1Z1, line1Z2, line2Z1, line2Z2)) {
			return true;
	}
	return false;
}

inline bool horizontalSegmentsIntersect(int line1X1, int line1Z1, int line1X2, int line1Z2, int line2X1, int line2Z1, int line2X2, int line2Z2)
{
	if (line1Z1 == line1Z2 && line2Z1 == line2Z2 && line1Z1 == line2Z1
		&& intervalIntersects(line1X1, line1X2, line2X1, line2X2)) {
			return true;
	}
	return false;
}

bool overlaps(int x, int z, int sizeX, int sizeZ, RectangularPiece& piece)
{
	if (inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX(), piece.getZ() + piece.getSizeZ()) ||
		inside(x, z, sizeX, sizeZ, piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ()) ||
		inside(piece.getX(), piece.getZ(), piece.getSizeX(), piece.getSizeZ(), x, z) ||
		horizontalSegmentsIntersect(x, z, x + sizeX, z, piece.getX(), piece.getZ(), piece.getX() + piece.getSizeX(), piece.getZ()) ||
		horizontalSegmentsIntersect(x, z + sizeZ, x + sizeX, z + sizeZ, piece.getX(), piece.getZ() + piece.getSizeZ(), piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ()) ||
		verticalSegmentsIntersect(x, z, x, z + sizeZ, piece.getX(), piece.getZ(), piece.getX(), piece.getZ() + piece.getSizeZ()) ||
		verticalSegmentsIntersect(x + sizeX, z, x + sizeX, z + sizeZ, piece.getX() + piece.getSizeX(), piece.getZ(), piece.getX() + piece.getSizeX(), piece.getZ() + piece.getSizeZ())) {
			return true;
	}
	return false;
}

void PiecesContainer::addPiece(RectangularPiece& piece)
{
	pieces.push_back(piece);
}
float PiecesContainer::findMaxY(int x, int z, int sizeX, int sizeZ)
{
	vector<RectangularPiece>::iterator it;
	float maxY = 0.0f;
	for (it = pieces.begin(); it != pieces.end(); it++) {
		if (overlaps(x, z, sizeX, sizeZ, *it)) {
			float y = it->getY() + it->getHeight();
			if (y > maxY) {
				maxY = y;
			}
		}
	}
	return maxY;
}

void PiecesContainer::drawPieces()
{
	vector<RectangularPiece>::iterator it;
	for (it = pieces.begin(); it != pieces.end(); it++) {
		it->draw();
	}
}