#ifndef _PP_ENVIRONMENT_H
#define _PP_ENVIRONMENT_H

#ifndef _PP_CAMERA_H
#include "camera.h"
#endif

#ifndef _PP_CHESSBOARD_H
#include "chessboard.h"
#endif

#ifndef _PP_ROBOARM_H
#include "roboarm.h"
#endif

#ifndef _PP_GAME_H
#include "game.h"
#endif

using namespace std;
using namespace cv;

//--------------------------------------------------------------------------------------------------------------

// druha moznost: void on_mouse_wrapper(int event, int x, int y, int flags, void* param);

class Environment{

	Chessboard chessboard;
	Camera camera;
	MyRoboticArm roboArm;
    Game* game;

	Mat firstCalibratedView;		// first captured view (after calibration)

public:
	// Environment(){};
	//~Environment(){if(roboArm != NULL) delete roboArm;};

	/* init */
	bool init(int cam_ch, int cam_w, int cam_h, int board_w, int board_h, float board_sqs, Point2f b_tlP, Point2f b_trP, Point2f b_blP, Point2f b_brP, Point2f b_psmP, Point2f b_anoP, Point2f rmoP, string& statusMsg);

	/* get functions */
	Game* get_pointer_to_game();
	Mat& get_firstCalibratedView();

	/* camera section */
	bool camera_grab_image(Mat& view, bool calibrateCapturedImage);

	/* chessboard section */
	ChessboardSquare* chessboard_get_pointer_to_square(int index);
	int chessboard_get_width();		// needed in Game::init when checking playable area
	int chessboard_get_height();	// needed in Game::init when checking playable area

	// zatial su tieto funkcie pozostatkom, pouzivaju sa vo funkcii on_mouse_wrapper, mozno treba upravit a pomazat
	const ChessboardSquare* chessboard_findOutToWhichSquareBelongsThePoint(int x, int y);
	Point2d chessboard_computeRWpositionOfThePoint(int x, int y);

	/* game section */ 
	bool start_new_game(string game_name);
	bool delete_game();
	bool restart_game();
	// temporary solution for output, change back afterwars bool play_the_game(Player* p1, Player* p2, int& result, unsigned int max_iter, int console_output);		// parameters: "result" sets int value of result type, "max_iter" sets how many moves are allowed all together, "console_output" sets if there will be console output of moves and game_states
	bool play_the_game(Player* p1, Player* p2, int& result, int& player1_mm_nodes, int& player2_mm_nodes, unsigned int max_iter, int console_output);		// parameters: "result" sets int value of result type, "max_iter" sets how many moves are allowed all together, "console_output" sets if there will be console output of moves and game_states
	void create_readable_info_about_game_start_end(bool start, int console_output, int result, Player* p1, Player* p2, time_t game_start_time);

	/* image openCV section */
	// must be static in order to work :)
	// edit: namiesto tohto pouzijem on_mouse_wrapper ako globalnu funkciu
	// edit2: pouzijem on_mouse_wrapper ale ako staticku metodu, nie globalnu funkciu
	static void on_mouse_wrapper(int event, int x, int y, int flags, void* param);
};

#endif
