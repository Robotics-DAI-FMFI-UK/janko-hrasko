#ifndef _PP_ENVIRONMENT_IMPL_H
#define _PP_ENVIRONMENT_IMPL_H

#ifndef _PP_ENVIRONMENT_H
#include "environment.h"
#endif

using namespace std;
using namespace cv;

// -------------------------------------------------------------------------------------------------------------------
/* constructor */

/* ----------------------------------------------------------------- < init > ----------------------------------------------------------------- */

bool Environment::init(int cam_ch, int cam_w, int cam_h, int board_w, int board_h, float board_sqs, Point2f b_tlP, Point2f b_trP, Point2f b_blP, Point2f b_brP, Point2f b_psmP, Point2f b_anoP, Point2f rmoP, string& statusMsg){
	// first thing
	namedWindow("environmentWin",1);
	cvSetMouseCallback("environmentWin",on_mouse_wrapper,this);
	//cvSetMouseCallback("environmentWin",Environment::my_mouse_callback2,0);

/* test xyz
	/* -----  init CAMERA  ----- */
/*	camera = Camera();
	bool cameraInit = camera.init(cam_ch, cam_w, cam_h, statusMsg);

	if(!cameraInit){
		statusMsg = "\nEnviroment init() failed. Reason:\n" + statusMsg;
		destroyWindow("environmentWin");
		return false;
	}else{
		statusMsg = "";
	}
test xyz */

	/*
	// zachytit prvy obrazok kvoli inicializacii prostredia
	Mat view_cap;
	if(!camera.grabCalibratedCaptureImage(view_cap, statusMsg)){
		statusMsg = "\nEnviroment init() failed. Reason:\n" + statusMsg;
		return false;
	}else{
		statusMsg = "";
	}
	*/
/* test xyz */ 
// debuggovacia vec - vymazat a odkomentovat to hore - dotestovat
	firstCalibratedView = imread("../images/image0.jpg");

// test	imwrite("../images/povodny.jpg",view);

	camera.calibrateImage(firstCalibratedView);

// test	imwrite("../images/kalibrovany.jpg",view);


	/* -----  init CHESSBOARD  ----- */
	chessboard = Chessboard();
	bool chessboardInit = chessboard.init(this, board_w, board_h, board_sqs, b_tlP, b_trP, b_blP, b_brP, b_psmP, b_anoP, rmoP, statusMsg);



	if(!chessboardInit){
		statusMsg = "\nEnviroment init() failed. Reason:\n" + statusMsg;
		destroyWindow("environmentWin");
		return false;
	}else{
		statusMsg = "";
	}


	/* -----  init CHESSBOARD  ----- */
	game = NULL;

	return true;
}

/* ------------------------------------------------------------ < get functions > ------------------------------------------------------------ */

Game* Environment::get_pointer_to_game(){
	return game;
}

Mat& Environment::get_firstCalibratedView(){
	return firstCalibratedView;
}

/* ------------------------------------------------------------ < camera section > ------------------------------------------------------------ */

bool Environment::camera_grab_image(Mat& view, bool calibrateCapturedImage){
	return camera.grabCaptureImage(view, calibrateCapturedImage);
}

/* ---------------------------------------------------------- < chessboard section > ---------------------------------------------------------- */

ChessboardSquare* Environment::chessboard_get_pointer_to_square(int index){
	return chessboard.get_pointer_to_square(index);
}

int Environment::chessboard_get_width(){
	return chessboard.getBoardWidth();
}

int Environment::chessboard_get_height(){
	return chessboard.getBoardHeight();
}

const ChessboardSquare* Environment::chessboard_findOutToWhichSquareBelongsThePoint(int x, int y){

	return chessboard.findToWhichSquareBelongsThePoint(Point2f(x,y));
}

Point2d Environment::chessboard_computeRWpositionOfThePoint(int x, int y){
	return chessboard.getRealWorldPositionFromImagePosition((float)x, (float)y);
}

/* ------------------------------------------------------------- < game section > ------------------------------------------------------------- */

bool Environment::start_new_game(string game_name){
	if(game != NULL) delete game;

	game = new Game();
	string message = "";

	bool gameInit = game->init(this,game_name,message);

	if(!gameInit){
		cout << endl << message << endl;
		
		delete game;
		game = NULL;

		return false;
	}

	return true;
}

bool Environment::delete_game(){
	if(game != NULL){
		delete game;
		game = NULL;
		return true;
	}
	return false;
}

bool Environment::restart_game(){
	if(game == NULL) return false;

	return game->restart();
}

// temporary solution for output, change back afterwars
// bool Environment::play_the_game(Player* p1, Player* p2, int& result, unsigned int max_iter, int console_output){
bool Environment::play_the_game(Player* p1, Player* p2, int& result, int& player1_mm_nodes, int& player2_mm_nodes, unsigned int max_iter, int console_output){
	// check pointers
	if((game == NULL)||(max_iter <= 0)) return false;
	if(p1 == NULL) return false;																				// at least 1st player must be present
	if((game->get_number_of_players())&&(p2 == NULL)) return false;												// if 2 player game, than also 2nd player must be present
	if((game->get_number_of_players())&&(p1->which_player_in_game == p2->which_player_in_game)) return false;	// if 2 player game, then they must be distinct

	// reset statistical data
	p1->minimax_tree_visited_nodes = 0;
	if(p2 != NULL) p2->minimax_tree_visited_nodes = 0;

	// text outup to console - about game start
	time_t game_start_time = time(0);
	create_readable_info_about_game_start_end(true,console_output,0,p1,p2,game_start_time);

	bool console_output_moves = (console_output > 1);
	cout << " ";																		// delete afterwards

	// play the game
	while(--max_iter > 0){																// max number of iterations
		if(!console_output_moves) cout << ".";											// just to let user know its not stuck, but computing
		/* actual game play */
		Player* p = (game->get_who_is_on_turn() == p1->which_player_in_game) ? p1 : p2;	// select which player will be maximizing

		if(!(p->human_or_AI)){															// if player is AI (computer)
			if(game->make_one_move_in_game_by_AI(p, console_output_moves)){				// try make one move by given strategy of players
				if(game->check_if_actual_node_is_set_as_final(result)){					// if move was successful, chceck if actual "actual_node" is not final
					break;	//return true;												// if yes, return true	
				}
				continue;																// else continue playing
			}
			result = 4; break;	//return false;											// if make 'one move' failed, return false
		}else{																			// if player is HUMAN player
			if(console_output < 2){ result = 4; break; }								// if game plays human player, there needs to be full text informations
			if(game->make_one_move_in_game_by_human(p)){								// try make one move
				if(game->check_if_actual_node_is_set_as_final(result)){					// if move was successful, chceck if actual "actual_node" is not final							
					break;	//return true;												// if yes, return true
				}
				continue;																// else continue playing
			}														
			result = 4; break;	//return false;											// if make 'one move' failed, return false
		}
	}
	
	// if game didn't end after "max_iter" moves, result will be DRAW (3), but distinct from normal DRAW (1)
	if(max_iter <= 0) result = 3;	//return true;										// 3 is information that maximum count of iterations is reached
	
	// text outup to console - about game end
	create_readable_info_about_game_start_end(false,console_output,result,p1,p2,game_start_time);

	// start temporary solution for output, delete afterwars
	player1_mm_nodes = p1->minimax_tree_visited_nodes;
	player2_mm_nodes = (p2 == NULL) ? 0 : p2->minimax_tree_visited_nodes;
	// end temporary solution for output, delete afterwars

	// return appropriate info
	switch(result){
		case 0 :
		case 1 :
		case 2 :
		case 3 :  return true;
		case 4 :
		default:  return false;
	};
}

void Environment::create_readable_info_about_game_start_end(bool start, int console_output, int result, Player* p1, Player* p2, time_t game_start_time){
		
	// console output
	if(start){							// create informations about start of game
		if(console_output > 1){
			cout << endl << "-----------------------------------------------------------------------";
			cout << endl << "Start the game \"" << game->get_game_name() << "\"  , at time: ";
			cout << asctime(localtime(&game_start_time));
		}
		if(console_output > 0){
			cout << endl << endl;
			cout << " Init game state:" << endl;
			cout << game->create_readable_info_about_actual_node_game_state();				// write initial state to output
			cout << endl;
		}
	}else{								// create informations about end of game
		if(console_output > 0){
			cout << endl << endl;
			cout << " Final game state:" << endl;
			cout << game->create_readable_info_about_actual_node_game_state();				// write final state to output
			cout << endl << endl;
			cout << " Game started at time: " << asctime(localtime(&game_start_time)) << endl;

			switch(result){
				case 0 :  cout << " Game ended as DRAW"; break;
				case 1 :  cout << ((game->get_number_of_players()) ? " Starting p" : " P") << "layer has WON"; break;
				case 2 :  cout << ((game->get_number_of_players()) ? " Starting p" : " P") << "layer has LOST"; break;
				case 3 :  cout << " Game was stoped, maximum count of iterations is reached"; break;
				case 4 :  cout << " ...something went wrong, need to restart game..."; break;
				default:  cout << " ...wrong answer, should not be even possible :) ..."; break;
			};
		
			time_t game_end_time = time(0);
			time_t game_duration = game_end_time - game_start_time;
			cout << endl << endl << " Game ended at time: " << asctime(localtime(&game_end_time));
			
			cout << endl <<	" Game duration:	";
			cout.width(2); cout.fill('0'); cout << game_duration/60;
			cout.width(); cout << ":"; 
			cout.width(2); cout.fill('0'); cout << game_duration%60; cout.width(); cout.fill(' '); cout << "  (mm:ss)";
			
			if((p1->ai_approach == 1)||(p1->ai_approach == 2)){
				cout << endl <<	endl << " Player_1 visited total of " << p1->minimax_tree_visited_nodes << " nodes in minimax";
				if(p1->ai_approach == 2){ cout << "_alpha_beta"; } cout << " game tree";
			}

			if(p2 != NULL){
				if((p2->ai_approach == 1)||(p2->ai_approach == 2)){
					cout << endl <<	endl << " Player_2 visited total of " << p2->minimax_tree_visited_nodes << " nodes in minimax";
					if(p2->ai_approach == 2){ cout << "_alpha_beta"; } cout << " game tree";
				}
			}	

			cout << endl << "-----------------------------------------------------------------------";		
		}
	}
}

/* --------------------------------------------------------- < image openCV section > --------------------------------------------------------- */

void Environment::on_mouse_wrapper(int event, int x, int y, int flags, void* param){
	// wrapper to get instance of class passed as parameter
	// because "on_mouse" is declared as static method in class, then there can't be used non-static method or attribute
	
	// http: //docs.opencv.org/modules/highgui/doc/user_interface.html#cvSetMouseCallback
	// http: //opencv-srf.blogspot.sk/2011/11/mouse-events.html

	Environment* env = (Environment*) param;

	if((event == CV_EVENT_LBUTTONDOWN)&&(event == CV_EVENT_LBUTTONDOWN)){
		cout << endl << endl << " ---- Mouse pressed  [  Left Button  ] ----------";
		cout << endl << " - Image coordinates:       [" << x << ", " << y << "]";

// * ak nebude treba dane funkcie kvoli niecomu inemu tak pouzijem toto a tie sa zmazu ako redundatne
		// * int id_square = env->chessboard.findToWhichSquareBelongsThePoint(Point2f((float)x,(float)y));
		// int id_square = env->chessboard_findOutToWhichSquareBelongsThePoint(x,y);
		// edit 2:
		const ChessboardSquare* chSq = env->chessboard_findOutToWhichSquareBelongsThePoint(x,y);

// ** ak nebude treba dane funkcie kvoli niecomu inemu tak pouzijem toto a tie sa zmazu ako redundatne
		// ** rw_pos = env->chessboard.getRealWorldPositionFromImagePosition((float)x, (float)y);
		Point2d rw_pos(-1,-1);
		rw_pos = env->chessboard_computeRWpositionOfThePoint(x,y);

		cout << endl << " - Real World coordinates:  " << rw_pos;

		if(chSq != NULL){
			cout << endl << " - belongs to chessboard_square: " << chSq->index;
			cout << endl << " - computed center of square (RW coordinates) :  " << chSq->center_RW_coords;
		}else{
			cout << endl << " - is out of chessboard area (doesn't belong to any chessboard square)";
		}
		
		cout << endl << " ( Real World coordinates are measured in milimeters )";
		
	}
}


#endif