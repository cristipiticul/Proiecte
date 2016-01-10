#include "PieceFactory.h"
#include "SimplePiece.h"
#include "CornerPiece.h"
#include "ColorGenerator.h"

extern ColorGenerator colorGenerator;

PieceFactory::PieceFactory()
{
	pieceType = SIMPLE;
}

RectangularPiece* PieceFactory::getNewPiece()
{
	switch (pieceType) {
	case SIMPLE:
		return new SimplePiece(0, 0, 2, 4, 0.0f, 0.5f, colorGenerator.getCurrentColor());
	case CORNER:
		return new CornerPiece(0, 0, 0.0f, 0.5f, colorGenerator.getCurrentColor());
	}
}

void PieceFactory::nextPieceType()
{
	switch (pieceType) {
	case SIMPLE:
		pieceType = CORNER;
		break;
	case CORNER:
		pieceType = SIMPLE;
		break;
	}
}
