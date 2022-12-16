#ifndef _PP_CHESSBOARD_IMPl_H
#define _PP_CHESSBOARD_IMPl_H

#ifndef _PP_CHESSBOARD_H
#include "chessboard.h"
#endif

using namespace std;
using namespace cv;

// -------------------------------------------------------------------------------------------------------------------

float cosOfAngleBetween2VectorsWithSameStartingPoint(Point2f p0, Point2f p1, Point2f p2);
bool isPointInPolygonP0P1P2P3(Point2f bod, Point2f p0, Point2f p1, Point2f p2, Point2f p3);

//--------------------------------------------------------------------------------------------------------------

bool Chessboard::init(Environment* e, int innerPointsWidth, int innerPointsHeight, float square_size, Point2f tlP, Point2f trP, Point2f blP, Point2f brP, Point2f passMoveCoords, Point2f addNewObjCoords, Point2f removeObjCoords, string& msg){
	env = e;
	
	transfMatrix_initialized = false;
	
	innerPSize.width = innerPointsWidth;
	innerPSize.height = innerPointsHeight;

	squaresPSize.width = innerPSize.width + 2;
	squaresPSize.height = innerPSize.height + 2;

	this->square_size = square_size;

	tlIPRWPos = tlP;	trIPRWPos = trP;	blIPRWPos = blP;	brIPRWPos = brP;

	pass_move_position_IMG_coords = passMoveCoords;
	new_object_position_IMG_coords = addNewObjCoords;
	remove_object_position_IMG_coords = removeObjCoords;

	if(!initSquarePoints(env->get_firstCalibratedView(), msg)){
		return false;
	}

	initTransformMatrix();

	initSquares();

	return true;
	
}

const ChessboardSquare* Chessboard::findToWhichSquareBelongsThePoint(Point2f p){
	int bap_w = squaresPSize.width;
	int bap_h = squaresPSize.height;
	
	// first check if is point on board, if not, return -1
	// left, right, top, bottom je len vzhladom na umiestnenie v poli pointov, nie nutne aj pozicia v image
	Point2f ltCorner = boardSquaresPoints[0];
	Point2f rtCorner = boardSquaresPoints[bap_w - 1];
	Point2f lbCorner = boardSquaresPoints[bap_w*(bap_h - 1)];
	Point2f rbCorner = boardSquaresPoints[bap_w*(bap_h - 1) + bap_w - 1];

	if(!isPointInPolygonP0P1P2P3(p,ltCorner,rtCorner,lbCorner,rbCorner)){
		// out of chessboard, doesn't belong to any chessboard square
		return NULL;
	}

	// else check every square until found the one
	for(int j = 0; j < (bap_h - 1); j++){
		for(int i = 0; i < (bap_w - 1); i++){
			ltCorner = boardSquaresPoints[i + j*bap_w];
			rtCorner = boardSquaresPoints[i + j*bap_w + 1];
			lbCorner = boardSquaresPoints[i + j*bap_w + bap_w];
			rbCorner = boardSquaresPoints[i + j*bap_w + bap_w + 1];

			if(isPointInPolygonP0P1P2P3(p,ltCorner,rtCorner,lbCorner,rbCorner)){
				return &boardSquares.at(i + j*(bap_w - 1));
			}
		}
	}

	cout << "Something went wrong. :)" << endl;
	return NULL;
}

bool Chessboard::initSquarePoints(Mat& view, string& msg){
	// test
	if(view.empty()){
		msg = "initSquarePoints() failed. Reason:\n  Image Failed to Load.";  
		return false;
    }

	//---------------------------------------------------------------------------------------
	// find innner points of board on image parameter
	vector<Point2f> pointBuf;
	findCorners(pointBuf, &(env->get_firstCalibratedView()));

	//---------------------------------------------------------------------------------------
	// board inner points - dimensions
	const int b_w = innerPSize.width;
	const int b_h = innerPSize.height;

	// board all points - dimensions
	const int bap_w = squaresPSize.width;
	const int bap_h = squaresPSize.height;

	// board all points - positions
	boardSquaresPoints = vector<Point2f>(bap_w*bap_h, Point2f(-1,-1));
		
	// set inner points in board all points
	for(int j = 0; j < b_h; j++){
		for(int i = 0; i < b_w; i++){

			int idPoint = (i+1) + (j+1)*bap_w;
			int idInnerPoint  =  i    + j*b_w;

			boardSquaresPoints[idPoint] = pointBuf[idInnerPoint];
		}
	}

	// ------------------------------------------------------------------------------------

	// temporary dimension for sequention of inner points
	int delta_w = b_w - 1;

	// temporary sequention of changes (delta) of position of inner points
	vector<Point2f> deltaXY (delta_w, Point2f(0,0));

	// bilateral ratio of these changes
	vector<Point2f> podiel  (delta_w - 1, Point2f(0,0));
				
	// computation of left and right outer points (not all, only central) ------------------------------------
	for(int j = 0; j < b_h; j++){		

		int left_new_point  = (j+1)*bap_w;
		int right_new_point = (j+2)*bap_w - 1;

		// changes (delta) of position of inner points in one row
		for(int i = 0; i < delta_w; i++){
			deltaXY[i].x = pointBuf.at(i+1 + j*b_w).x - pointBuf.at(i + j*b_w).x;
			deltaXY[i].y = pointBuf.at(i+1 + j*b_w).y - pointBuf.at(i + j*b_w).y;
		}

		// approximation of left point from row of inner points ----------
		// bilateral ratio of changes of inner points in row

//test
		int num_x = 0, num_y = 0;
// potialto

		for(int i = 0; i < (delta_w - 1); i++){
			podiel[i].x = deltaXY[i].x / deltaXY[i+1].x;
// test
			num_x++;
			if((abs(deltaXY[i].x) < 0.01)||(abs(deltaXY[i+1].x) < 0.01)){
				num_x--;
				podiel[i].x = (float)0;
			}
// potialto
			podiel[i].y = deltaXY[i].y / deltaXY[i+1].y;

// test
			num_y++;
			if((abs(deltaXY[i].y) < 0.01)||(abs(deltaXY[i+1].y) < 0.01)){
				num_y--;
				podiel[i].y = (float)0;
// potialto
			}
		}

		Point2f koeficient (0,0);

		// set coefficient ---------------------- 
		for(int i = 0; i < (delta_w - 1); i++){
			koeficient += podiel[i];
		}

// test
//		koeficient.x /= (delta_w-1);
//		koeficient.y /= (delta_w-1);

		koeficient.x /= num_x;
		koeficient.y /= num_y;
// potialto

		// set new - left (not inner) - point
		// + (-koefifient)*...    =>   - koeficient*...
		boardSquaresPoints[left_new_point].x = boardSquaresPoints[left_new_point + 1].x - deltaXY[0].x*koeficient.x;
		boardSquaresPoints[left_new_point].y = boardSquaresPoints[left_new_point + 1].y - deltaXY[0].y*koeficient.y;

		// approximation of right point from row of inner points ----------
		// bilateral ratio of changes of inner points in row
			
		// podiel sa prepise, v opacnom poradi teraz

//test
		num_x = 0;
		num_y = 0;
// potialto

		for(int i = 0; i < (delta_w - 1); i++){
			podiel[i].x = deltaXY[i+1].x / deltaXY[i].x;
// test
			num_x++;
			if((abs(deltaXY[i+1].x) < 0.01)||(abs(deltaXY[i].x) < 0.01)){
				num_x--;
				podiel[i].x = (float)0;
			}
// potialto
			podiel[i].y = deltaXY[i+1].y / deltaXY[i].y;

// test
			num_y++;
			if((abs(deltaXY[i+1].y) < 0.01)||(abs(deltaXY[i].y) < 0.01)){
				num_y--;
				podiel[i].y = (float)0;
// potialto
			}
		}

		// set coefficient again ---------------------- 
		koeficient = Point2f(0,0);

		for(int i = 0; i < (delta_w - 1); i++){
			koeficient += podiel[i];
		}

// test

		koeficient.x /= num_x;
		koeficient.y /= num_y;
// potialto

		// set new - right (not inner) - point
		boardSquaresPoints[right_new_point].x = boardSquaresPoints[right_new_point - 1].x + deltaXY[delta_w-1].x*koeficient.x;
		boardSquaresPoints[right_new_point].y = boardSquaresPoints[right_new_point - 1].y + deltaXY[delta_w-1].y*koeficient.y;
	}

	// temporary dimension for sequention of inner points
	int delta_h = b_h - 1;

	// temporary sequention of changes (delta) of position of inner points
	deltaXY = vector<Point2f>(delta_h, Point2f(0,0));

	// bilateral ratio of these changes
	podiel = vector<Point2f> (delta_h - 1, Point2f(0,0));

	// computation of top and bottom outer points (all of them, even corner ones) --------------------------
	for(int i = 0; i < bap_w; i++){
		int top_new_point    = i;
		int bottom_new_point = i + bap_w*(bap_h - 1);

		// changes (delta) of position of inner points in one column
		for(int j = 0; j < delta_h; j++){
			deltaXY[j].x = boardSquaresPoints[ i + (j+2)*bap_w ].x - boardSquaresPoints[ i + (j+1)*bap_w ].x;
			deltaXY[j].y = boardSquaresPoints[ i + (j+2)*bap_w ].y - boardSquaresPoints[ i + (j+1)*bap_w ].y;
		}

		// approximation of top point from column of points ----------
		// bilateral ratio of changes of points in column
		
//test
		int num_x = 0, num_y = 0;
// potialto

		for(int j = 0; j < (delta_h - 1); j++){
			podiel[j].x = deltaXY[j].x / deltaXY[j+1].x;
// test
			num_x++;
			// zacal som tu na urovni 0.001, postupne na 0.01 ale aj pri tomto sa objavili problemove body, teraz pokus s tymto
			if((abs(deltaXY[j].x) < 0.1)||(abs(deltaXY[j+1].x) < 0.1)){
				num_x--;
				podiel[j].x = (float)0;
			}
// potialto
			podiel[j].y = deltaXY[j].y / deltaXY[j+1].y;

// test
			num_y++;
			// zacal som tu na urovni 0.001, postupne na 0.01 ale aj pri tomto sa objavili problemove body, teraz pokus s tymto
			if((abs(deltaXY[j].y) < 0.1)||(abs(deltaXY[j+1].y) < 0.1)){
				num_y--;
				podiel[j].y = (float)0;
// potialto
			}		
		}

		Point2f koeficient (0,0);

		// set coefficient ---------------------- 
		for(int j = 0; j < (delta_h - 1); j++){
			koeficient += podiel[j];
		}

// test
//		koeficient.x /= (delta_w-1);
//		koeficient.y /= (delta_w-1);

		koeficient.x /= num_x;
		koeficient.y /= num_y;
// potialto

		// set new - top (not inner) - point
		boardSquaresPoints[top_new_point].x = boardSquaresPoints[top_new_point + bap_w].x - deltaXY[0].x*koeficient.x;
		boardSquaresPoints[top_new_point].y = boardSquaresPoints[top_new_point + bap_w].y - deltaXY[0].y*koeficient.y;

		// approximation of bottom point from column of points ----------
		// bilateral ratio of changes of points in column
		
//test
		num_x = 0;
		num_y = 0;
// potialto

		for(int j = 0; j < (delta_h - 1); j++){
			podiel[j].x = deltaXY[j+1].x / deltaXY[j].x;
// test
			num_x++;
			// zacal som tu na urovni 0.001, postupne na 0.01 ale aj pri tomto sa objavili problemove body, teraz pokus s tymto
			if((abs(deltaXY[j+1].x) < 0.1)||(abs(deltaXY[j].x) < 0.1)){
				num_x--;
				podiel[j].x = (float)0;
			}
// potialto
			podiel[j].y = deltaXY[j+1].y / deltaXY[j].y;

// test
			num_y++;
			// zacal som tu na urovni 0.001, postupne na 0.01 ale aj pri tomto sa objavili problemove body, teraz pokus s tymto
			if((abs(deltaXY[j+1].y) < 0.1)||(abs(deltaXY[j].y) < 0.1)){
				num_y--;
				podiel[j].y = (float)0;
// potialto
			}		
		}

		koeficient = Point2f(0,0);

		// set coefficient ---------------------- 
		for(int j = 0; j < (delta_h - 1); j++){
			koeficient += podiel[j];
		}

// test
//		koeficient.x /= (delta_w-1);
//		koeficient.y /= (delta_w-1);

		koeficient.x /= num_x;
		koeficient.y /= num_y;
// potialto

		// set new - bottom (not inner) - point
		boardSquaresPoints[bottom_new_point].x = boardSquaresPoints[bottom_new_point - bap_w].x + deltaXY[delta_h - 1].x*koeficient.x;
		boardSquaresPoints[bottom_new_point].y = boardSquaresPoints[bottom_new_point - bap_w].y + deltaXY[delta_h - 1].y*koeficient.y;
	}
	
	// test if points are mapped accurate to board square points (corners)
	showPoints(view);

	return true;
}

void Chessboard::initSquares(){
	
	int bap_w = squaresPSize.width;
	int bap_h = squaresPSize.height;
	
	Point2f stredTlBr, stredTrBl; 

	// board squares
	boardSquares = vector<ChessboardSquare>((bap_w - 1)*(bap_h - 1), ChessboardSquare());
	
	// compute real world position of center of each square
	for(int j = 0; j < (bap_h - 1); j++){
		for(int i = 0; i < (bap_w - 1); i++){
			// new square
			ChessboardSquare chbSq;
			
			// set properties
			chbSq.index = i + j*(bap_w - 1);

			chbSq.tlCorner = &boardSquaresPoints[i + j*bap_w];
			chbSq.trCorner = &boardSquaresPoints[i + j*bap_w + 1];
			chbSq.blCorner = &boardSquaresPoints[i + j*bap_w + bap_w];
			chbSq.brCorner = &boardSquaresPoints[i + j*bap_w + bap_w + 1];

			// compute center of square
			stredTlBr.x = (float)0.5*(chbSq.brCorner->x - chbSq.tlCorner->x) + chbSq.tlCorner->x;
			stredTlBr.y = (float)0.5*(chbSq.brCorner->y - chbSq.tlCorner->y) + chbSq.tlCorner->y;

			stredTrBl.x = (float)0.5*(chbSq.blCorner->x - chbSq.trCorner->x) + chbSq.trCorner->x;
			stredTrBl.y = (float)0.5*(chbSq.blCorner->y - chbSq.trCorner->y) + chbSq.trCorner->y;

			chbSq.center_Img_coords = Point2f((stredTlBr.x + stredTrBl.x)/(float)2 , (stredTlBr.y + stredTrBl.y)/(float)2);
			chbSq.center_RW_coords = (Point2f)getRealWorldPositionFromImagePosition(chbSq.center_Img_coords);

			// insert to position in collection
			boardSquares.at(i + j*(bap_w - 1)) = chbSq;
		}
	}
}


void Chessboard::initTransformMatrix(){
	// init
	transformMatrix = Mat(3,3,CV_64FC1);				// result

	Mat transformMatrix1 (3,3,CV_64FC1);				// points tl, tr, bl
	Mat transformMatrix2 (3,3,CV_64FC1);				// points tl, tr, br
	Mat transformMatrix3 (3,3,CV_64FC1);				// points tl, bl, br
	Mat transformMatrix4 (3,3,CV_64FC1);				// points tr, bl, br

	// temporary matrixes
	Mat ImgPoints(3,3,CV_64FC1);
	Mat RWPoints(3,3,CV_64FC1);
	
	int tlInnerPoint = squaresPSize.width + 1;
	int trInnerPoint = 2*squaresPSize.width -2;
	int blInnerPoint = squaresPSize.width*(squaresPSize.height - 2) + 1;
	int brInnerPoint = squaresPSize.width*(squaresPSize.height - 1) - 2;

	// matrix 1 - points tl, tr, bl
	ImgPoints.at<double>(0,0) = (double)boardSquaresPoints[tlInnerPoint].x;
	ImgPoints.at<double>(0,1) = (double)boardSquaresPoints[trInnerPoint].x;
	ImgPoints.at<double>(0,2) = (double)boardSquaresPoints[blInnerPoint].x;
	ImgPoints.at<double>(1,0) = (double)boardSquaresPoints[tlInnerPoint].y;
	ImgPoints.at<double>(1,1) = (double)boardSquaresPoints[trInnerPoint].y;
	ImgPoints.at<double>(1,2) = (double)boardSquaresPoints[blInnerPoint].y;
	ImgPoints.at<double>(2,0) = 1.0;
	ImgPoints.at<double>(2,1) = 1.0;
	ImgPoints.at<double>(2,2) = 1.0;
	
	RWPoints.at<double>(0,0) = (double)tlIPRWPos.x;
	RWPoints.at<double>(0,1) = (double)trIPRWPos.x;
	RWPoints.at<double>(0,2) = (double)blIPRWPos.x;
	RWPoints.at<double>(1,0) = (double)tlIPRWPos.y;
	RWPoints.at<double>(1,1) = (double)trIPRWPos.y;
	RWPoints.at<double>(1,2) = (double)blIPRWPos.y;
	RWPoints.at<double>(2,0) = 1.0;
	RWPoints.at<double>(2,1) = 1.0;
	RWPoints.at<double>(2,2) = 1.0;

	// computation
	transformMatrix1 = RWPoints * ImgPoints.inv();
	
	// matrix 2 - points tl, tr, br
	ImgPoints.at<double>(0,0) = (double)boardSquaresPoints[tlInnerPoint].x;
	ImgPoints.at<double>(0,1) = (double)boardSquaresPoints[trInnerPoint].x;
	ImgPoints.at<double>(0,2) = (double)boardSquaresPoints[brInnerPoint].x;
	ImgPoints.at<double>(1,0) = (double)boardSquaresPoints[tlInnerPoint].y;
	ImgPoints.at<double>(1,1) = (double)boardSquaresPoints[trInnerPoint].y;
	ImgPoints.at<double>(1,2) = (double)boardSquaresPoints[brInnerPoint].y;
		
	RWPoints.at<double>(0,0) = (double)tlIPRWPos.x;
	RWPoints.at<double>(0,1) = (double)trIPRWPos.x;
	RWPoints.at<double>(0,2) = (double)brIPRWPos.x;
	RWPoints.at<double>(1,0) = (double)tlIPRWPos.y;
	RWPoints.at<double>(1,1) = (double)trIPRWPos.y;
	RWPoints.at<double>(1,2) = (double)brIPRWPos.y;

	// computation
	transformMatrix2 = RWPoints * ImgPoints.inv();

	// matrix 3 - points tl, bl, br
	ImgPoints.at<double>(0,0) = (double)boardSquaresPoints[tlInnerPoint].x;
	ImgPoints.at<double>(0,1) = (double)boardSquaresPoints[blInnerPoint].x;
	ImgPoints.at<double>(0,2) = (double)boardSquaresPoints[brInnerPoint].x;
	ImgPoints.at<double>(1,0) = (double)boardSquaresPoints[tlInnerPoint].y;
	ImgPoints.at<double>(1,1) = (double)boardSquaresPoints[blInnerPoint].y;
	ImgPoints.at<double>(1,2) = (double)boardSquaresPoints[brInnerPoint].y;
		
	RWPoints.at<double>(0,0) = (double)tlIPRWPos.x;
	RWPoints.at<double>(0,1) = (double)blIPRWPos.x;
	RWPoints.at<double>(0,2) = (double)brIPRWPos.x;
	RWPoints.at<double>(1,0) = (double)tlIPRWPos.y;
	RWPoints.at<double>(1,1) = (double)blIPRWPos.y;
	RWPoints.at<double>(1,2) = (double)brIPRWPos.y;

	// computation
	transformMatrix3 = RWPoints * ImgPoints.inv();

	// matrix 4 - points tr, bl, br
	ImgPoints.at<double>(0,0) = (double)boardSquaresPoints[trInnerPoint].x;
	ImgPoints.at<double>(0,1) = (double)boardSquaresPoints[blInnerPoint].x;
	ImgPoints.at<double>(0,2) = (double)boardSquaresPoints[brInnerPoint].x;
	ImgPoints.at<double>(1,0) = (double)boardSquaresPoints[trInnerPoint].y;
	ImgPoints.at<double>(1,1) = (double)boardSquaresPoints[blInnerPoint].y;
	ImgPoints.at<double>(1,2) = (double)boardSquaresPoints[brInnerPoint].y;
		
	RWPoints.at<double>(0,0) = (double)trIPRWPos.x;
	RWPoints.at<double>(0,1) = (double)blIPRWPos.x;
	RWPoints.at<double>(0,2) = (double)brIPRWPos.x;
	RWPoints.at<double>(1,0) = (double)trIPRWPos.y;
	RWPoints.at<double>(1,1) = (double)blIPRWPos.y;
	RWPoints.at<double>(1,2) = (double)brIPRWPos.y;

	// computation
	transformMatrix4 = RWPoints * ImgPoints.inv();


	// compute resulting transform matrix - each element of resulting transform matrix is a mean of given 4 matrices - for each element
	for(int i = 0; i < transformMatrix.rows; i++){
		for(int j = 0; j < transformMatrix.cols; j++){			
			transformMatrix.at<double>(i,j) = transformMatrix1.at<double>(i,j);
			transformMatrix.at<double>(i,j) += transformMatrix2.at<double>(i,j);
			transformMatrix.at<double>(i,j) += transformMatrix3.at<double>(i,j);
			transformMatrix.at<double>(i,j) += transformMatrix4.at<double>(i,j);

			transformMatrix.at<double>(i,j) /= 4.0;

			if(abs(transformMatrix.at<double>(i,j)) < 0.00001){
				transformMatrix.at<double>(i,j) = 0.0;
			}
		}
	}


	transfMatrix_initialized = true;
}

int Chessboard::getBoardWidth(){
	return squaresPSize.width - 1;
}
int Chessboard::getBoardHeight(){
	return squaresPSize.height - 1;
}

Point2d Chessboard::getRealWorldPositionFromImagePosition(Point2f point){
	return getRealWorldPositionFromImagePosition(point.x, point.y);
}

Point2d Chessboard::getRealWorldPositionFromImagePosition(float img_x, float img_y){
	// check if transform matrix was inicialized
	if(!transfMatrix_initialized){
		return Point2f(0,0);
	}	
	
	// temp
	Mat point_img = Mat(3,1,CV_64FC1);
	Mat point_rw  = Mat(3,1,CV_64FC1);

	point_img.at<double>(0,0) = (double)img_x;
	point_img.at<double>(1,0) = (double)img_y;
	point_img.at<double>(2,0) = 1.0;

	point_rw.at<double>(0,0) = 0.0;
	point_rw.at<double>(1,0) = 0.0;
	point_rw.at<double>(2,0) = 0.0;

	point_rw = transformMatrix * point_img;

	return Point2d(point_rw.at<double>(0,0), point_rw.at<double>(1,0));
}

Point2f Chessboard::getCentreOfSquare(int id){
	// test - out of array, then return [-1,-1]
	if((id < 0) || (id >= (int)boardSquares.size())){
		return Point2f(-1, -1);
	}
	
	return boardSquares.at(id).center_RW_coords;
}

ChessboardSquare* Chessboard::get_pointer_to_square(int index){
	if((index < 0)||(index >= (int)boardSquares.size())){
		return NULL;
	}
	return &boardSquares.at(index);
}

bool Chessboard::findCorners(vector<Point2f> &corners, Mat* view = NULL){
	Mat tempView;

	if(view != NULL){
		tempView = Mat(*view);
	}else{
		env->camera_grab_image(tempView, true);
	}

	bool found = findChessboardCorners(tempView, innerPSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

	// recalculate with higher accuracy
	if(found){
		Mat viewGray;
		cvtColor(tempView, viewGray, CV_BGR2GRAY);
		cornerSubPix(viewGray, corners, Size(11,11), Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
		return true;
	}

	return false;
}

const Point2f Chessboard::get_new_object_position(){
	return new_object_position_RW_coords;
}

const Point2f Chessboard::remove_object_position(){
	return remove_object_position_RW_coords;
}

void Chessboard::showPoints(Mat& view){
	
	for(int j = 0; j < squaresPSize.height; j++){
		for(int i = 0; i < squaresPSize.width; i++){
			
			int idSquarePoint = i + j*squaresPSize.width;

			if(boardSquaresPoints[idSquarePoint].x != -1){
				circle(view, boardSquaresPoints[idSquarePoint], 3, CV_RGB(255-4*idSquarePoint,0,4*idSquarePoint), -1);	
			}

			imshow("environmentWin",view);

		}
	}		

}

// ----------------------------------------------------------------------------------------------------------------------------------

// returns cosinus of angle (in radians) of 2 vectors, p0p1 and p0p2
// both vectors have mutual starting point p0
// cos(angle) = (v1.v2) / (|v1|*|v2|)
float cosOfAngleBetween2VectorsWithSameStartingPoint(Point2f p0, Point2f p1, Point2f p2){
	// vector1 a vector2
	Point2f v1 (p1.x - p0.x, p1.y - p0.y);
	Point2f v2 (p2.x - p0.x, p2.y - p0.y);

	float a = (v1.x*v2.x + v1.y*v2.y);
	float b = sqrt((v1.x*v1.x + v1.y*v1.y))*(sqrt(v2.x*v2.x + v2.y*v2.y));

	if(abs(b) < 0.000001){
		return 0.0;			// no division by zero
	}

	float c = a/b;
	return c;
};

bool isPointInPolygonP0P1P2P3(Point2f bod, Point2f p0, Point2f p1, Point2f p2, Point2f p3){
	// podmienka p0 je lavy horny bod, p1 je pravy horny bod, p2 je lavy dolny bod, p3 je pravy dolny bod polygonu (obdlzniku)
	// vzhladom na suradnicovu sustavu monitoru, kde [0,0] je lavy horny roh, x sa zvacsuje smerom doprava, y sa zvacsuje smerom dolu

	// p0 ----- p1
	//    |   |
	// p2 ----- p3

	if(((bod.x < p0.x)&&(bod.x < p1.x)&&(bod.x < p2.x)&&(bod.x < p3.x))||
	   ((bod.x > p0.x)&&(bod.x > p1.x)&&(bod.x > p2.x)&&(bod.x > p3.x))||
	   ((bod.y < p0.y)&&(bod.y < p1.y)&&(bod.y < p2.y)&&(bod.y < p3.y))||
	   ((bod.y > p0.y)&&(bod.y > p1.y)&&(bod.y > p2.y)&&(bod.y > p3.y))){
		return false;
	}

	// este sa moze nachadzat v obale obdlznika, ale nie priamo v nom
	// vypoctova kontrola, vyrata sa uhol medzi 2 vektormi a pripadne este raz uhol medzi 2 vektormi
	// musi byt, ze uhol medzi vektorom p0p1 a p0point je do velkosti uhla medzi p0p1 a p0p2 a tiez medzi p0p2 a p0point
	// ak je, tak druhy krok kontroly je medzi p3p1 a p3point a tiez p3p2 a p3point

	// cosUhla2VektorovSrovnakymZaciatkomVp1 ak je kladny tak je medzi nimi uhol do 90 stupnov	- pri obdlznikoch
	// posudzuje sa podla uhla medzi p0p1 a p0p2 ( pripadne p3p1 a p3p2 ) a nie 90 stupnov - vseobecne pri polygonoch
	
	float cosUhlaP0P1aP0P2 = cosOfAngleBetween2VectorsWithSameStartingPoint(p0,p1,p2);
	if(cosOfAngleBetween2VectorsWithSameStartingPoint(p0,p1,bod) < cosUhlaP0P1aP0P2){
		return false;
	}
	if(cosOfAngleBetween2VectorsWithSameStartingPoint(p0,p2,bod) < cosUhlaP0P1aP0P2){
		return false;
	}

	float cosUhlaP3P1aP3P2 = cosOfAngleBetween2VectorsWithSameStartingPoint(p3,p1,p2);
	if(cosOfAngleBetween2VectorsWithSameStartingPoint(p3,p1,bod) < cosUhlaP3P1aP3P2){
		return false;
	}
	if(cosOfAngleBetween2VectorsWithSameStartingPoint(p3,p2,bod) < cosUhlaP3P1aP3P2){
		return false;
	}

	return true;
};

#endif