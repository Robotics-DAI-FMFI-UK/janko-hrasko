#include <iostream>
#include <sstream>		// std::stringstream
#include <fstream>		// pre pracu so subormi
#include <time.h>
#include <stdio.h>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <math.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "cameraCalibration.h";
#include "camera.h";				// camera.h			priamo potrebuje cameraCalibratio.h
#include "chessboard.h";
#include "roboarm_servo.h"
#include "roboarm.h"				// roboarm.h		priamo potrebuje roboarm_servo.h
#include "game.h"					// game.h			priamo potrebuje chessboard.h (konkretne ChessboardSquare class)
#include "environment.h";			// environment.h	priamo potrebuje {roboarm.h, board.h, camera.h, game.h}

#include "camera_impl.h"
#include "chessboard_impl.h"
#include "roboarm_servo_impl.h"
#include "roboarm_impl.h"
#include "game_impl.h"
#include "environment_impl.h"

using namespace std;
using namespace cv;
 

int main(int argc, char* argv[])
{
	// camera.calibrate bez parametru len vytiahne uz nakonfigurovane hodnoty z defaultneho suboru
	// camera.calibrate s argumentom kalibruje odznova celu kameru
	//string xml_CameraConfig_Settings = (argc > 1) ? argv[1] : "";
	//string cameraStatusMsg = "";
		
	
	// 2 je moja kamera, inde moze byt inak, -1 je hocijaka, inak zacina od 0
	// ../camera_calibration/camera_in_config.xml

	//xml_CameraConfig_Settings = "";

	//bool cameraInit = camera.init(2, 1280, 720, xml_CameraConfig_Settings, cameraStatusMsg);

	/***************************  settings for environment initialization  ***************************/
	/* settings for camera */
	int camera_channel = 2;
	int camera_width = 640;
	int camera_height = 480;

	/* settings for camera calibration */
	int board_innner_points_width = 7;
	int board_innner_points_height = 5;
	float board_square_size = (float)33.7;	// size of one square on board, in milimeters, real world property

	/* settings for transformation image coordinates -> RW coordinates */
	Point2f board_tlP (0.0,135.0);			// position in real world of Top	Left  inner board point
	Point2f board_trP (202.0,135.0);		// position in real world of Top	Right inner board point
	Point2f board_blP (0.0,0.0);			// position in real world of Bottom Left  inner board point
	Point2f board_brP (202.0,0.0);			// position in real world of Bottom Left  inner board point

	/* informations about coordinates to manipulate new objects and remove objects from game */
	//Point2f board_newObjP (210.0, 20.0);
	//Point2f board_rmvObjP (210.0, 50.0);
	Point2f board_pssMove (10.0, 10.0);
	Point2f board_newObjP (20.0, 20.0);
	Point2f board_rmvObjP (30.0, 30.0);
	

	/* - zatial takto(bez exceptions): return message */
	string statusMsg = "";

	// -----------------------------------------------------------------------------------------------------------

	/* environment initialization */
	Environment e = Environment();
	bool envInit = e.init(camera_channel, camera_width, camera_height,
						  board_innner_points_width, board_innner_points_height, board_square_size,
						  board_tlP, board_trP, board_blP, board_brP,
						  board_pssMove, board_newObjP, board_rmvObjP,
						  statusMsg);
		
	/* test if environment initialization was OK */
	if(!envInit){
		cout << "There seems to be a problem:" << endl << statusMsg;
		return 0;
	}

	/* 'play the game' section */
	vector<int> game_results = vector<int>(6,0);
	int text_output = 2;			// 0 = no text informations, 1 = only start-end, 2 = all informations about moves and states (needs to be this type in order to play with human player)

	/* start temporary solution for output, delete afterwards */
	int temp_player_1_total_minimax_nodes = 0, temp_player_2_total_minimax_nodes = 0;	// temporary solution for output, delete afterwards
	time_t game_cycle_start_time = time(0);												// temporary solution for output, delete afterwards
	/* end temporary solution for output, delete afterwards */

//	string game_name = "frogs";		unsigned int max_iterations = 25;	// maximum 25 iterations is enough for "frogs", but not really needed
	string game_name = "connect4";	unsigned int max_iterations = 55;	// maximum 55 iterations is enough for "connect4", but not really needed
//	string game_name = "alquerque";	unsigned int max_iterations = 85;	// maximum 85 iterations is good for "alquerque"

	/* start the game, initialize it */
	bool game_is_playable = e.start_new_game(game_name);

	unsigned int number_of_played_games = 0;

	/* play the game */
	while((number_of_played_games < 1) && game_is_playable){
		/* game need player(s) */
			/*	info for constructor of Player
				Player(game,_,_,_,_,_)					- Game*	- just pointer to parent Game
				Player(_,human_or_AI,_,_,_,_)			- bool	- false if AI, true if Human
				Player(_,_,which_player_in_game,_,_,_)	- bool	- false is 1st player, true is 2nd player
				Player(_,_,_,ai_approach,_,_,)			- int	- 0 = random player
																- 1 = minimax
																- 2 = minimax_alpha_beta
				Player(_,_,_,_,type_of_heuristic,_)		- int	- 0		= no heuristic, just returns 0
																- 1		= for game "frogs"
																- 2 & 3	= for game "connect4"	(different heuristics)
																- 4 & 5 = for game "alquerque"  (different heuristics)
				Player(_,_,_,_,_,minimax_depth)			- int	- depth prunning value for minimax			
		
				e.g.
				Player(game*, true, bool)			- is for human player
				Player(game*, false, bool, 0)		- is for AI random player
				Player(game*, false, false, 2,5,4)	- is for AI player, using minimax_AB, heuristic 5, depth prunning at level 4

			*/

//		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 0);
/*
		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 1, 2, 1);
		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 1, 3, 1);
*/
/*
		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 1, 2, 4);
		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 1, 3, 5);
*/
/*
		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 2, 5, 4);
		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 0);
*/
/*
		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 0);
		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 1, 3, 4);
*/

//		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 1, 5, 3);
//		Player* p1 = new Player(e.get_pointer_to_game(), true, false);
//		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 1, 4, 3);
//		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 0);

		Player* p1 = new Player(e.get_pointer_to_game(), true, false);
//		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 0);
//		Player* p1 = new Player(e.get_pointer_to_game(), false, false, 2, 1, 1);
//		Player* p2 = NULL;
		Player* p2 = new Player(e.get_pointer_to_game(), false, true, 2, 5, 4);

		int game_result_int = 0;
		
		/* start temporary solution for output, delete afterwards */
		int player1_mm_nodes = 0, player2_mm_nodes = 0;										// temporary solution for output, delete afterwards
		cout << endl << endl << " Play the game #" << (number_of_played_games + 1) << endl;	// temporary solution for output, delete afterwards
		/* end temporary solution for output, delete afterwards */

		/* temporary solution for output, changed function play_the_game to have parameters temp_player_1_total_minimax_nodes and temp_player_2_total_minimax_nodes, change afterwards */
		//bool play_game = e.play_the_game(p1,p2,game_result_int,max_iterations,2);				 // maximum 85 iterations of game moves, then DRAW (but type 3, not 1)
		bool play_game = e.play_the_game(p1,p2,game_result_int,player1_mm_nodes, player2_mm_nodes, max_iterations,2);	 // max_iterations is value of game moves, then DRAW (but type 3, not 1), console ouput is 0 - no output, 1 - only gameinfo, 2 - all info including possible moves (needs to be set 2 in order to play as human player)

		/* start temporary solution for output, delete afterwards */
		temp_player_1_total_minimax_nodes += player1_mm_nodes;
		temp_player_2_total_minimax_nodes += player2_mm_nodes;
		/* end temporary solution for output, delete afterwards */

		if(play_game){							
			switch(game_result_int){
				case 0 : { game_results.at(0)++; break;}
				case 1 : { game_results.at(1)++; break;}
				case 2 : { game_results.at(2)++; break;}
				case 3 : { game_results.at(3)++; break;}
				default: { game_results.at(4)++; break;}
			}
		}else{game_results.at(5)++;}

		/* game cycle, restart the game to initialized state */
		number_of_played_games++;
		game_is_playable = e.restart_game();
	}

	/* start temporary solution for output, delete afterwards */
	time_t game_cycle_end_time = time(0);																		// temporary solution for output, delete afterwards
	time_t game_cycle_duration = game_cycle_end_time - game_cycle_start_time;									// temporary solution for output, delete afterwards
	cout << endl << " Cycle of games started at " << asctime(localtime(&game_cycle_start_time));				// temporary solution for output, delete afterwards
	cout << endl << " Player_1 visited total of " << temp_player_1_total_minimax_nodes << " nodes in minimax";	// temporary solution for output, delete afterwards
	cout << endl << " Player_2 visited total of " << temp_player_2_total_minimax_nodes << " nodes in minimax";	// temporary solution for output, delete afterwards
	cout << endl << " Cycle of games ended at " << asctime(localtime(&game_cycle_end_time));					// temporary solution for output, delete afterwards
	cout << endl <<	" Cycle of games time duration:	";
	cout.width(2); cout.fill('0'); cout << game_cycle_duration/60; cout.width(); cout << ":"; 
	cout.width(2); cout.fill('0'); cout << game_cycle_duration%60; cout.width(); cout.fill(' '); cout << "  (mm:ss)";

	cout.width(); cout << endl << endl << " Results:" << endl;
	cout << "  Number of draws: " << game_results.at(0) << endl;
	cout << "  Player 1 wins " << game_results.at(1) << " times" << endl;
	cout << "  Player 2 wins " << game_results.at(2) << " times" << endl;
	cout << "  Maximum iterations of players move defined for game was reached: " << game_results.at(3) << " times" << endl;
	cout << "  Any game function in 'play_the_game' function returned False: " << game_results.at(4) << " times" << endl;
	cout << "  Function 'play_the_game' returned False: " << game_results.at(5) << " times" << endl;
	/* end temporary solution for output, delete afterwards */


	/* close everything */
	e.delete_game();

	cvWaitKey(0);
	destroyWindow("environmentWin");
	
	// -----------------------------------------------------------------------------------------------------------

	getchar();
	return 0;
}
