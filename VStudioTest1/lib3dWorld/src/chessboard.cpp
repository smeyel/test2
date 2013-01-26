#include "chessboard.h"

using namespace cv;

Chessboard::Chessboard(Size iBoardSize, int iSquareSize)
{
	squareSize = iSquareSize;
	boardSize = iBoardSize;
	calcCorners(boardSize, squareSize);
}

Chessboard::Chessboard(Chessboard& iChessboard)
{
	squareSize = iChessboard.squareSize;
	boardSize = iChessboard.boardSize;
	calcCorners(boardSize, squareSize);
}

Chessboard::Chessboard()
{
}

void Chessboard::calcCorners(Size boardSize, int squareSize)
{
	for( int i = 0; i < boardSize.height; ++i )
		for( int j = 0; j < boardSize.width; ++j )
			corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
}
