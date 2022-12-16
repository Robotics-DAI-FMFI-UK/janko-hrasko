#ifndef _PP_CHESSBOARD_H
#define _PP_CHESSBOARD_H

#ifndef _PP_ENVIRONMENT_H
class Environment;							// forward declaration	
#endif

using namespace std;
using namespace cv;

// predpokladam, ze plati:
// boardSquaresPoints[0] je lavy horny roh sachovnice, 
// [squaresPSize.width - 1] je pravy horny
// [squaresPSize.width*(squaresPSize.height-1)] je lavy spodny
// [squaresPSize.width*(squaresPSize.height-1) + squaresPSize.width] je pravy spodny

class ChessboardSquare{
  public:
	int index;						// index of chessboard square - its identifier in chessboard - starts 0 as top left square on chessboard

	/* coordinates of chessboard square - pointers to coordinates in collection "boardSquaresPoints" in class Chessboard */
	Point2f* tlCorner;				// Top Left corner of chessboard square
	Point2f* trCorner;				// Top Right corner of chessboard square
	Point2f* blCorner;				// Bottom Left corner of chessboard square
	Point2f* brCorner;				// Bottom Right corner of chessboard square

	/* coordinates of center of chessboard square */
	Point2f center_Img_coords;		// coordinates in image
	Point2f center_RW_coords;		// coordinates in Real World
	
	// constructor
	ChessboardSquare() : index(-1), tlCorner(NULL), trCorner(NULL), blCorner(NULL), brCorner(NULL), center_Img_coords(Point2f(-1.0,-1.0)), center_RW_coords(Point2f(-1.0,-1.0)) {}
};

//--------------------------------------------------------------------------------------------------------------

class Chessboard{
	/* pointer to containing class */
	Environment* env;
		
	// size of the whole playable space - chessboard - provided by number of corners points for chessboard squares
	Size innerPSize;								// size of board inner corners points (corners of inner squares)
	Size squaresPSize;								// size of board all corners points - including outer corners points

	// square size in real world, in milimeters
	float square_size;						

	// image coordinates for chessboard squares corners points
	vector<Point2f> boardSquaresPoints;				// position of all points on board, relative to image (not real world position)
	
	// collection of chessboard squares + "pass move" place + "new object" place + "remove object" place
	vector<ChessboardSquare> boardSquares;			// vector with all chessboard squares
	Point2f pass_move_position_IMG_coords;			// coordinates (on captured image) of position, where human should click, in order to play "pass move" when in GUI mode
	Point2f new_object_position_RW_coords;			// coordinates (in Real World) of position, where new object will be placed, in order to put it in game with robotic arm as new object
	Point2f new_object_position_IMG_coords;			// coordinates (on captured image) of position, where new object will be placed, in order to put it in game with robotic arm as new object
	Point2f remove_object_position_RW_coords;		// coordinates (in Real World) of position, where object will be placed, in order to remove it with robotic arm from game
	Point2f remove_object_position_IMG_coords;		// coordinates (on captured image) of position, where object will be placed, in order to remove it with robotic arm from game
	
	// transformation matricies, transformation from image coordinates to Real World coordinates
	Mat transformMatrix;							// transform matrix, multiply it from right with column vector (Image position: point.x; point.y; 1.0) and get RealWorld position of Point in image
	bool transfMatrix_initialized;					// check this before multiplication in getRealWorldPositionFromImagePosition()

	// inner points will be more accurate then corners of board, bcs their positions on captured image are not approximate (as corners are) but precisely computed
	Point2f tlIPRWPos;								// position in real world of Top	Left  inner board point
	Point2f trIPRWPos;								// position in real world of Top	Right inner board point
	Point2f blIPRWPos;								// position in real world of Bottom Left  inner board point
	Point2f brIPRWPos;								// not really needed, position in real world of Bottom Right inner board point
	
  public:
	//Board();

	bool init(Environment* e, int innerPointsWidth, int innerPointsHeight, float square_size, Point2f tlP, Point2f trP, Point2f blP, Point2f brP, Point2f passMoveCoords, Point2f addNewObjCoords, Point2f removeObjCoords, string& msg);
	bool initSquarePoints(Mat& view, string& msg);
	void initTransformMatrix();
	void initSquares();

	int getBoardWidth();
	int getBoardHeight();

	Point2d getRealWorldPositionFromImagePosition(Point2f point);
	Point2d getRealWorldPositionFromImagePosition(float img_x, float img_y);

	Point2f getCentreOfSquare(int id);

	const ChessboardSquare* findToWhichSquareBelongsThePoint(Point2f p);

	ChessboardSquare* get_pointer_to_square(int index);

	bool findCorners(vector<Point2f> &corners, Mat* view);

	const Point2f get_new_object_position();
	const Point2f remove_object_position();

	void showPoints(Mat& view);

};

#endif