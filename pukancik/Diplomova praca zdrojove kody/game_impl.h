#ifndef _PP_GAME_IMPL_H
#define _PP_GAME_IMPL_H

#ifndef _PP_GAME_H
#include "game.h"
#endif

using namespace std;
using namespace cv;

// -------------------------------------------------------------------------------------------------------------------

/* constructor GameSquare */
GameObject::GameObject() : gameSq(NULL), objInfo(NULL) {};
GameObject::GameObject(GameSquare* gs, ObjectInfo* oi) : gameSq(gs), objInfo(oi) {};

// -------------------------------------------------------------------------------------------------------------------

/* constructor GameSquare */
GameSquare::GameSquare() : chessboardSq(NULL), gameObj(NULL) {};
GameSquare::GameSquare(ChessboardSquare* chSq, GameObject* gO) : chessboardSq(chSq), gameObj(gO) {}

// -------------------------------------------------------------------------------------------------------------------

/* constructor GameMove */
GameMove::GameMove() : type_of_move(-1), gs_from(NULL), gs_to(NULL), obj_before_type(NULL), obj_after_type(NULL), parent_game(NULL), reward_player_1(0), reward_player_2(0), add_to_new_objects_after_remove(false) {};
GameMove::GameMove(Game* game) : type_of_move(-1), gs_from(NULL), gs_to(NULL), obj_before_type(NULL), obj_after_type(NULL), parent_game(game), reward_player_1(0), reward_player_2(0), add_to_new_objects_after_remove(false) {};

/* creates 'string hash' of given 'game_move' */
string GameMove::string_hash(){
	stringstream ss_output;

	switch(type_of_move){
		/* 'pass move' */
		case 0  : ss_output << type_of_move; break;
		/* add new object */
		case 1  :
			{
				if((gs_to == NULL)||(obj_after_type == NULL)) return string("");
				ss_output << type_of_move << "-" << gs_to->chessboardSq->index << "-" << obj_after_type->type;
				break;
			}
		/* move object */
		case 2  :
			{
				if((gs_from == NULL)||(gs_to == NULL)||(obj_before_type == NULL)||(obj_after_type == NULL)) return string("");
				ss_output << type_of_move << "-" << gs_from->chessboardSq->index << "-" << obj_before_type->type << "-" << gs_to->chessboardSq->index << "-" << obj_after_type->type;
				break;
			}
		/* remove object */
		case 3  :
			{
				if((gs_from == NULL)||(obj_before_type == NULL)) return string("");
				ss_output << type_of_move << "-" << gs_from->chessboardSq->index << "-" << obj_before_type->type;
				break;
			}
		default : return string("");
	}
	
	return ss_output.str();
}

string GameMove::human_readable_info(){
	stringstream ss_output;

	switch(type_of_move){
		/* 'pass move' */
		case 0  : ss_output << "[" << string_hash() << "] 'Pass move', change no object, next player turn."; break;
		/* add new object */
		case 1  :
			{
				if((gs_to == NULL)||(obj_after_type == NULL)) return string("");
				ss_output << "[" << string_hash() << "] Add new object of type " << obj_after_type->type << " to index " << gs_to->chessboardSq->index;
				break;
			}
		/* move object */
		case 2  :
			{
				if((gs_from == NULL)||(gs_to == NULL)||(obj_before_type == NULL)||(obj_after_type == NULL)) return string("");
				ss_output << "[" << string_hash() << "] Move object of type " << obj_before_type->type << " from index " << gs_from->chessboardSq->index;
				ss_output << " to index " << gs_to->chessboardSq->index << " as type " << obj_after_type->type;
				break;
			}
		/* remove object */
		case 3  :
			{
				if((gs_from == NULL)||(obj_before_type == NULL)) return string("");
				ss_output << "[" << string_hash() << "] Remove object of type " << obj_before_type->type << " from index " << gs_from->chessboardSq->index;
				break;
			}
		default : return string("");
	}

	return ss_output.str();
}

/* constructor GameMove_SideEffect */
GameMove_SideEffect::GameMove_SideEffect() : GameMove() {};
GameMove_SideEffect::GameMove_SideEffect(Game* game) : GameMove(game) {};
GameMove_SideEffect::GameMove_SideEffect(GamePossibleMove* gpm){
	type_of_move = gpm->type_of_move; gs_from = gpm->gs_from; gs_to = gpm->gs_to;
	obj_before_type = gpm->obj_before_type; obj_after_type = gpm->obj_after_type;
	parent_game = gpm->parent_game; reward_player_1 = gpm->reward_player_1; reward_player_2 = gpm->reward_player_2;
	add_to_new_objects_after_remove = gpm->add_to_new_objects_after_remove;
}

/* constructor GamePossibleMove */
GamePossibleMove::GamePossibleMove() : GameMove(), 
									   side_effects(list<GameMove_SideEffect>()), pm_move_for_roboArm(list<GameSimpleFinalMove>()),
									   player_continues_at_next_turn(false), player_must_move_at_next_turn(false),
									   restrict_move_only_to_this_object(false), restricted_object_can_make_pass_move(false) {};

GamePossibleMove::GamePossibleMove(Game* game)
{
	*this =	GamePossibleMove();
	parent_game = game;
};

/* constructor GameSimpleFinalMove */
GameSimpleFinalMove::GameSimpleFinalMove() : parent_game(NULL),	gs_from(NULL), gs_to(NULL), obj_after_type(NULL) {};
GameSimpleFinalMove::GameSimpleFinalMove(Game* game) : parent_game(game), gs_from(NULL), gs_to(NULL), obj_after_type(NULL) {};	
GameSimpleFinalMove::GameSimpleFinalMove(Game* game, GameSquare* gs_f, GameSquare* gs_t, ObjectInfo* obj_t) : parent_game(game), gs_from(gs_f), gs_to(gs_t), obj_after_type(obj_t) {};

// -------------------------------------------------------------------------------------------------------------------

/* constructor GameState */
GameState::GameState() : parent_game(NULL),
						 gameObjects(list<GameObject>()),
						 gameSquares(vector<GameSquare>()),
						 possible_new_objects(vector<int>()), 
						 player_turn(false),
						 //only_add_new_objects_as_possible_move(false),
						 player_must_move(false), restricted_object_move_at_square(-1),
						 restricted_object_can_create_pass_move(false),
						 isFinalState(false), game_result(-1), player1_score(0), player2_score(0)
{
/* 
	VERY IMPORTANT , "if a reallocation happens, all iterators, POINTERS and references related to the container are invalidated."
*/
	gameSquares.reserve(64);	// 8x8 chessboard
};

GameState::GameState(Game* game){
	*this =	GameState();
	parent_game = game;
};


// copy collections "gameObjects", "gameSquares" and "possible_new_objects" as new vectors, for child 'game_square', so it needs to change pointers also
void GameState::copy_collections_of_gs_for_child_gs(GameState& gs){
	// init gameObjects
	this->gameObjects.clear();
	
	// init gameSquares with apropriate size and default GameSquare() -> GameSquare(NULL,NULL)
	this->gameSquares = vector<GameSquare>(gs.gameSquares.size(),GameSquare());
	// loop and set
	for(unsigned int i = 0; i < gs.gameSquares.size(); i++){
		this->gameSquares[i].chessboardSq = gs.gameSquares[i].chessboardSq;

		// if there is 'object' on this 'game_square'
		if(gs.gameSquares[i].gameObj != NULL){
			this->gameObjects.push_back(GameObject(&(this->gameSquares[i]), gs.gameSquares[i].gameObj->objInfo));

			this->gameSquares[i].gameObj = &(this->gameObjects.back());
		}
	}

	// collection "possible_new_objects" contains primitive type - int
	possible_new_objects = vector<int>(gs.possible_new_objects);
}

void GameState::change_possible_new_objects(unsigned int index, int change){
	// check if index value in range
	if((index < 0)||(index >= possible_new_objects.size())) return;
	
	// only add +1 or remove -1
	int value = possible_new_objects.at(index);
	if(value == 777) return;	// 777 means infinity => no change of value
	if(change == 1) possible_new_objects.at(index) += 1;						
	if((change == -1)&&(value > 0)) possible_new_objects.at(index) -= 1;		
	
	return;
}

// remove "GameObject" from collection "gameObjects" which is on given 'game_square'
bool GameState::remove_object_at_game_square(GameSquare* gs){
	for(list<GameObject>::iterator it = gameObjects.begin(); it != gameObjects.end(); it++){
		if(it->gameSq == gs){
			it = gameObjects.erase(it);
			return true;
		}
	}
	return false;	// if object at given 'game_state' was not found
}

/* get function */
GameSquare* GameState::get_game_square_at_index(unsigned int index){
	return ((index >= 0)&&(index < gameSquares.size())) ? &gameSquares.at(index) : NULL;
}

// return pointer to GameSquare in local collection "gameSquare", which have 'chessboard_square' index given as parameter, otherwise NULL
GameSquare* GameState::get_game_squares_with_chsq_index(unsigned int index){
	// try direct approach first
	if(index < gameSquares.size())
		if(gameSquares.at(index).chessboardSq->index == index) return &gameSquares.at(index);

	// otherwise try find it in vector
	for(unsigned int i = 0; i < gameSquares.size(); i++){
		if(gameSquares[i].chessboardSq->index == index){
			return &gameSquares[i];
		} 
	}
	return NULL;
}

bool GameState::get_possible_new_objects_value_at_index(unsigned int index, int& value){
	// if index out of range
	if((index < 0)||(index >= possible_new_objects.size())) return false;
	
	// otherwise set parameter value
	value = possible_new_objects.at(index);
	return true;
}

// -------------------------------------------------------------------------------------------------------------------

/* constructors GameTreeNode */
GameTreeNode::GameTreeNode() : parent_game(NULL), parent_node(NULL), game_state(GameState()),
							   possible_moves(unordered_map<string,GamePossibleMove>()), child_nodes(list<pair<string,GameTreeNode>>()),
							   depth(-1), result_of_move(""), isNodeDeveloped(false), isMoveChosen(false), previous_move_was_pass_move(false), reward(-3)
{};

GameTreeNode::GameTreeNode(Game* game, GameTreeNode* parent, int node_depth, string move_from_parent){	
	*this =	GameTreeNode();
	this->parent_game = game;
	this->parent_node = parent;
	this->depth = node_depth;
	this->result_of_move = move_from_parent;
};

/* init root game_tree_node */
bool GameTreeNode::init_tree_root_node(){
	// init only root GameTreeNode, depth == 0, parent_game pointer is set, parent_node is NULL
	if((depth != 0)||(parent_game == NULL)||(parent_node != NULL)){
		return false;
	}
	
	// test if collection "possible_moves" is empty, otherwise clear it
	if(!possible_moves.empty()){
		possible_moves.clear();
	}

	// test if collection "child_nodes" is empty, otherwise clear it
	if(!child_nodes.empty()){
		child_nodes.clear();
	}
	
	// reset some more properties
	previous_move_was_pass_move = false;
	result_of_move = "";
	isNodeDeveloped = false;
	isMoveChosen = false;
	
	reward = -3;

	game_state = GameState(parent_game);

	/* call function of Game class that creates initial state from its attributes */
	parent_game->create_initial_state(&game_state);

	return true;	
};

/* get function */
GameState* GameTreeNode::get_game_state(){
	return &game_state;
}

GameSquare* GameTreeNode::get_game_square_at_index(int index){
	//return ((index >= 0)&&((unsigned int)index < game_state.gameSquares.size())) ? &game_state.gameSquares.at(index) : NULL ;
	return game_state.get_game_square_at_index((unsigned int)index);
}

GameSquare* GameTreeNode::get_game_square_with_chsq_index(int index){
	return game_state.get_game_squares_with_chsq_index((unsigned int)index);
}

GamePossibleMove* GameTreeNode::get_possible_move_by_key_value(string& key_value){
	// try find 'possible_move' by its string key_value 
	try{
		// insert side effect to list of side_effects to appropriate possible_move
		GamePossibleMove* pm = &possible_moves.at(key_value);
		return pm;
	}catch(out_of_range& oor){
		return NULL;				
	}
}

bool GameTreeNode::isGameStateFinal(){
	return game_state.isFinalState;
}

// -------------------------------------------------------------------------------------------------------------------

/* constructor GameTree */
GameTree::GameTree() : parent_game(NULL), root(GameTreeNode()) {};
GameTree::GameTree(Game* game) : parent_game(game), root(GameTreeNode(game, NULL, 0, "")) {};

/* init root game_tree_node */
bool GameTree::init(){
	return root.init_tree_root_node();
};

/* get functions */
GameTreeNode* GameTree::get_root(){ return &root;};
const Game* GameTree::get_game(){ return parent_game; };

// -------------------------------------------------------------------------------------------------------------------

/* constructor Player */
Player::Player() : game(NULL), human_or_AI(false), which_player_in_game(false), ai_approach(0),  type_of_heuristic(0), minimax_depth(1), minimax_tree_visited_nodes(0) {};
Player::Player(Game* g, bool hORai, bool wpin, int ai_approach = 0, int heu = 0, int md = 0)
	: game(g), human_or_AI(hORai), which_player_in_game(wpin), ai_approach(ai_approach), type_of_heuristic(heu), minimax_depth(md), minimax_tree_visited_nodes(0) {};

// -------------------------------------------------------------------------------------------------------------------

/* constructor Playable_Area */
Playable_Area::Playable_Area() : width(-1), height(-1), left_top_corner(0) {};
Playable_Area::Playable_Area(int w, int h, unsigned int ltc) : width(w), height(h), left_top_corner(ltc) {};

/* -------------------------------------------------------------------- < GAME CLASS > -------------------------------------------------------------------- */

/* constructor Game */
Game::Game() : /* pointer to containing class */
			   env(NULL),
			   /* info about game */
			   game_name(""), number_of_players(false),
			   player_must_move_each_turn(false), what_no_possible_moves_means(0), how_to_check_final_state(0),
			   /* game space and objects */
			   playable_area(Playable_Area()), objectsTypes(vector<ObjectInfo>()),
			   /* game states structure */
			   game_tree(NULL), actual_node(NULL),
			   /* data defining inital state of game, or after restart -> new game */
			   init_game_squares(list<pair<ChessboardSquare*, ObjectInfo*>>()),
			   init_player_turn(false), init_player_must_move(false), 
//			   init_only_add_new_objects_as_possible_move(false),
			   init_restricted_object_move_at_square(-1), init_possible_new_objects(vector<int>()),
			   /* bool constraints */
			   game_initialized(false),
			   /* another informations about game */
			   obj_colours(vector<Scalar>()), obj_shapes(vector<int>())
{};

/* destructor*/
Game::~Game(){
	if(game_tree != NULL){
		delete game_tree;
// toto dalej este neotestovane
		game_tree = NULL;
	}
};

/* ------------------------------------------------------------------ < initialization > ------------------------------------------------------------------ */

/* init */
bool Game::init(Environment* e, string gameName, string& statusMsg){
	// pointer to containing class Environment
	env = e;

	// string value identifier for files and folders of game
	game_name = gameName;

	// reserve some default initial sizes of collections 
	objectsTypes.reserve(10);			// typically, simple board game will consist no more than 2 different types of object for each player + 1 neutral 0 type , so 5 is good value for start to reserve

	// set true before read_gameinfo_file(), game_initialized is checked inside and set false if something is not right
	game_initialized = true;

	// gameinfo.lp initialization, must proceed init.lp initialization 
	if(!read_gameinfo_file(statusMsg)){
		game_initialized = false; 
		return false;
	}

	// some default setting
	init_player_must_move = false;										// default is - nope
	init_restricted_object_move_at_square = -1;							// default is - there is no restriction 
	init_possible_new_objects = vector<int>(objectsTypes.size(),0);		// default is - collection of zeroes, same size as collection "objectsTypes"

	// init.lp initialization, must be after gameinfo.lp initialization
	if(!read_init_file(statusMsg)){
		game_initialized = false; 
		return false;
	}

	// everything is OK --> init successfull
	game_initialized = true;

	// just to be sure, set this, even if it is already set in constructor Game()
	if(game_tree != NULL){
		delete game_tree;
		game_tree = NULL;
	}
		
	// reSET attributes (player_turn, gameObjects, gameSquares) to initial state of game
	if(!(restart())) return false;
	
	// check playable_area
	if(!((playable_area.width == -1)||(playable_area.height == -1)||(playable_area.left_top_corner == -1))){
		// check if 'game_state' contains 'game_square' with 'chessboard_square' with index "left top corner" value
		if(game_tree->get_root()->get_game_state()->get_game_squares_with_chsq_index(playable_area.left_top_corner) == NULL){
			statusMsg.append(" Game init failed. Reason:\n  Wrong attributes for \"playable_area\" (left top corner) in file \""); statusMsg.append(game_name); statusMsg.append("_gameinfo.lp\".\n");
			return false;	 
		}
		// check number of 'game_squares'
		if(game_tree->get_root()->get_game_state()->gameSquares.size() != (playable_area.width*playable_area.height)){
			statusMsg.append(" Game init failed. Reason:\n  Wrong attributes for \"playable_area\" (number of game_squares against proportions of playable_area) in file \""); statusMsg.append(game_name); statusMsg.append("_gameinfo.lp\".\n");
			return false;
		}
		// check if playable_area fits into chessboard
		int pa_tlc_current_row	  = playable_area.left_top_corner / env->chessboard_get_width();	// position of object accordingly to playable_area
		int pa_tlc_current_column = playable_area.left_top_corner % env->chessboard_get_width();	// position of object accordingly to playable_area

		if(((pa_tlc_current_column + playable_area.width) > env->chessboard_get_width())||((pa_tlc_current_row + playable_area.height) > env->chessboard_get_height())){
			statusMsg.append(" Game init failed. Reason:\n  Wrong attributes for \"playable_area\" (width or height) in file \""); statusMsg.append(game_name); statusMsg.append("_gameinfo.lp\".\n");
			return false;
		}
	}

	return true;
}

/* restarts game, empty game_sequence, set first game state according to initial values and remove 'old' files 'game'_stateofgame.lp and 'game'_results.lp */
bool Game::restart(){
	// test, better be sure
	if(!game_initialized){
		return false;
	}

	// delete old game_tree, but test so it will delete only not NULL game_tree
	if(game_tree != NULL){
		delete game_tree;
	}

	// create new GameTree structure
	game_tree = new GameTree(this);

	// init game_tree by creating initial state in root_game_tree_node
	game_tree->init();

	// set actual game_tree_node to root of game_tree
	actual_node = game_tree->get_root();

	// remove-delete files with last stateofgame.lp and moves.lp
	remove_state_files();

	return true;
}


bool Game::read_gameinfo_file(string &statusMsg){
	// predpokladam ze vstup je korektny, cize
	// - jeden riadok = jeden predikat
	// - predikaty su zapisane v spravnej syntaxi
	// - indexy v predikatoch idu po sebe od 0, bez medzier medzi indexami, cize 0-1-2-3-4-...-n
	
	fstream file_gameinfo;
	
	string file_gameinfo_source = "..\\games\\" + game_name + "\\" + game_name + "_gameinfo.lp";
	
	file_gameinfo.open(file_gameinfo_source.c_str(), ios::in);
	
	// if open method failed
	if(!file_gameinfo){
		statusMsg.append("Game init failed. Reason:\nFile \""); statusMsg.append(game_name); statusMsg.append("_gameinfo.lp\" not found or didn't open...\n");
		return false;
	}

	// file open OK
	string line;
	stringstream ss;

	while(getline(file_gameinfo,line)){
		// ignore empty line
		if(line.empty()){
			continue;
		}

		// find first not empty char at line
		int predicate_name_start = line.find_first_not_of(' ');

		// ignore comment
		// find first char '%', if it is first non empty char than it means that whole line is commentar
		if(line.find_first_of('%') == predicate_name_start){
			continue;
		}

		// find some vital parts of predicate
		int predicate_opening_parenthese = line.find_first_of('(', predicate_name_start + 1);
		int predicate_ending_dot = line.find_first_of('.', predicate_name_start + 1);

		// test if there is problem with predicate syntax    -    if any of these is equal to string::npos means it's not in "line" string
		// every predicate must have name (string, at least 1 char) and ending dot ('.')
		if((predicate_name_start == string::npos)||(predicate_ending_dot == string::npos)){
			game_initialized = false;
			break;
		}

		// predicate without parameters don't have parentheses, or at least not before dot '.'
		bool no_parameter_predicate = ((predicate_opening_parenthese == string::npos)||(predicate_ending_dot < predicate_opening_parenthese));

		// find another vital parts of predicate
		int predicate_closing_parenthese = (no_parameter_predicate) ? string::npos : line.find_last_of(')'); 

		// test if there is problem with predicate syntax    -    if any of these is equal to string::npos means it's not in "line" string while there should be ( bcs -> !no_parameter_predicate)
		if(!no_parameter_predicate&&((predicate_opening_parenthese == string::npos)||(predicate_closing_parenthese == string::npos))){
			game_initialized = false;
			break;
		}
		
		// depending on "bool no_parameter_predicate" set predicate name
		string predicate_name = (no_parameter_predicate) ? line.substr(predicate_name_start, predicate_ending_dot - predicate_name_start) : line.substr(predicate_name_start, predicate_opening_parenthese - predicate_name_start);
		
		// matching predicate names:
		// 1) predicate defining number of players	-  syntax: predicate_name(integer)
		if (!predicate_name.compare("number_of_players")){		// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false;
				break;
			}

			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			int temp_number_of_players = -1;
			if(!(ss >> temp_number_of_players)){				
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}		

			// in gameinfo syntax, number_of_players(1) means 1 player, number_of_players(2) means 2 players, which is represented in Game class as:
			// bool number_of_players = false = 1 player    OR    bool number_of_players = true = 2 players
			switch (temp_number_of_players){
				case 1 : number_of_players = false; continue;
				case 2 : number_of_players = true; continue;
				default: game_initialized = false; break;
			}
			if(!game_initialized) break;						// if otherwise - value != 1 or 2

			continue;
		}

		// 2) predicate defining what to do if there are no possible moves  - syntax: predicate_name(integer)
		if (!predicate_name.compare("what_no_possible_moves_means")){		// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false;
				break;
			}

			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			if(!(ss >> what_no_possible_moves_means)){				
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}		

			continue;
		}

		// 3) predicate defining if player on turn must move with some object every turn    -    syntax: predicate_name.
		if (!predicate_name.compare("player_must_move_every_turn")){	// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// if this predicate is present, set "Game::player_must_move_each_turn" to TRUE
			player_must_move_each_turn = true;
			continue;
		}

		// 4) predicate defining how to check if given game_state is final    -    syntax: predicate_name(integer).
		if (!predicate_name.compare("how_to_check_final_state")){		// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false;
				break;
			}

			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			if(!(ss >> how_to_check_final_state)){				
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}		

			continue;
		}

		// 5) predicate defining playable_area  - syntax: predicate_name(integer)
		if (!predicate_name.compare("playable_area")){			// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false;
				break;
			}

			// find another vital parts of predicate
			int first_comma = line.find_first_of(',',predicate_opening_parenthese + 1);
			int second_comma = line.find_first_of(',',first_comma + 1);

			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if((first_comma == string::npos)||(second_comma == string::npos)){
				game_initialized = false;			
				break;
			}

			// 1st parameter - playable_area width - type (integer)
			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, first_comma - predicate_opening_parenthese - 1));
			if(!(ss >> playable_area.width)){			// set playable_area attribute - width
				game_initialized = false;				// if there is problem with conversion, end while cycle
				break;
			}	

			// 2nd parameter - playable_area height - type (integer)
			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(first_comma + 1, second_comma - first_comma - 1));
			if(!(ss >> playable_area.height)){			// set playable_area attribute - height
				game_initialized = false;				// if there is problem with conversion, end while cycle
				break;
			}

			// 3rd parameter - playable_area left top corner - type (integer)
			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(second_comma + 1, predicate_closing_parenthese - second_comma - 1));
			if(!(ss >> playable_area.left_top_corner)){	// set playable_area attribute - left corner
				game_initialized = false;				// if there is problem with conversion, end while cycle
				break;
			}
			// ok, playable_area is set
			continue;
		}
		
		// 6) predicate defining types of objects	 -    syntax: predicate_name(integer, integer, string, integer, integer) 
		if (!predicate_name.compare("type_of_objects")){		// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false;
				break;
			}

			// find another vital parts of predicate
			int first_comma = line.find_first_of(',',predicate_opening_parenthese + 1);
			int second_comma = line.find_first_of(',',first_comma + 1);
			int opening_quotes = line.find_first_of('\"',second_comma + 1);
			int closing_quotes = line.find_first_of('\"',opening_quotes + 1);
			int third_comma = line.find_first_of(',',closing_quotes + 1);
			int fourth_comma = line.find_first_of(',',third_comma + 1);
			
			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if((first_comma == string::npos)||(second_comma == string::npos)||(opening_quotes == string::npos)||
			   (closing_quotes == string::npos)||(third_comma == string::npos)||(fourth_comma == string::npos)){
					game_initialized = false;			
					break;
			}
			
			// this should be result of predicate at particular line
			ObjectInfo objInfo;

			// 1st parameter - type of object - type (integer)
			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, first_comma - predicate_opening_parenthese - 1));

			if(!(ss >> objInfo.type)){							// set objInfo attribute - typeIndex
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}
			
			// 2nd parameter - type of object - player (integer)
			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(first_comma + 1, second_comma - first_comma - 1));
			int temp_objInfo_player = -1;
			if(!(ss >> temp_objInfo_player)){					
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}

			// in gameinfo syntax, player(1) means 1st player, player(2) means 2nd player, which is represented in ObjectInfo class as:
			// bool player = false = 1st player    OR    bool player = true = 2nd player
			switch (temp_objInfo_player){
				case 0 : objInfo.player = false; break;			// no player, this is for 'empty object' 
				case 1 : objInfo.player = false; break;			// 1st player
				case 2 : objInfo.player = true; break;			// 2nd player
				default: game_initialized = false; break;
			}
			if(!game_initialized) break;						// if otherwise - value != 0 or 1 or 2

			// 3rd parameter - type of object - name (string)
			objInfo.name = line.substr(opening_quotes + 1, closing_quotes - opening_quotes - 1);
			
			// 4th parameter - type of object - colour (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(third_comma + 1, fourth_comma - third_comma - 1));
			if(!(ss >> objInfo.colour)){						// set objInfo attribute - colour
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}

			// 5th parameter - type of object - shape (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(fourth_comma + 1, predicate_closing_parenthese - fourth_comma - 1));
			if(!(ss >> objInfo.shape)){							// set objInfo attribute - shape
				game_initialized = false;						// if there is problem with conversion, end while cycle
				break;
			}

			// test to achieve ordered collection, where elements are listed in order by their type value
			if(objectsTypes.size() != objInfo.type){
				game_initialized = false;
				break;
			}
			
			// predicate syntax and predicate order are OK => add new type of object
			objectsTypes.push_back(objInfo);
			
			continue;
		}
	}

	// close file
	file_gameinfo.close();
	
	// check if while cycle end abruptly
	if(!game_initialized){
		statusMsg.append("Game init failed. Reason:\n Game don't have proper game settings e.g. predicate syntax or predicate order (specific - in file - \"");
		statusMsg.append(game_name); statusMsg.append("_gameinfo.lp\").\n");
		return false;
	}

	// everything went OK
	return true;
}

/* read predicates from file 'game'_init.lp */
/* sets these attributes :
	- init_player_turn				( predicate  - "player_turn" )
	- init_player_must_move			( predicate  - "player_must_move_this_turn" )   
	- init_object_restricted_move	( predicate  - "object_restricted_move" )
	- init_possible_new_objects		( predicates - "init_possible_new_objects"
												 - "possible_infinite_new_objects")
	- init_game_squares				( predicate  - "game_square" )				*/
bool Game::read_init_file(string &statusMsg){
	// predpokladam ze vstup je korektny, cize
	// - jeden riadok = jeden predikat
	// - predikaty su zapisane v spravnej syntaxi
	// - indexy v predikatoch idu po sebe od 0, bez medzier medzi indexami, cize 0-1-2-3-4-...-n
	
	// init.lp  file	-	open file with init informations
	fstream file_init;
	
	string file_init_source = "..\\games\\" + game_name + "\\" + game_name + "_init.lp";
	
	file_init.open(file_init_source.c_str(), ios::in);
	
	// if open method failed
	if(!file_init){
		statusMsg.append("Game init failed. Reason:\nFile \""); statusMsg.append(game_name); statusMsg.append("_init.lp\" not found or didn't open...\n");
		return false;
	}

	// file open OK
	string line;
	stringstream ss;

	while(getline(file_init,line)){
		// ignore empty line
		if(line.empty()) continue;

		// find first not empty char at line
		int predicate_name_start = line.find_first_not_of(' ');

		// ignore comment
		// find first char '%', if it is first non empty char than it means that whole line is commentar
		if(line.find_first_of('%') == predicate_name_start) continue;

		// find some vital parts of predicate
		int predicate_opening_parenthese = line.find_first_of('(', predicate_name_start + 1);
		int predicate_ending_dot = line.find_first_of('.', predicate_name_start + 1);

		// test if there is problem with predicate syntax    -    if any of these is equal to string::npos means it's not in "line" string
		// every predicate must have name (string, at least 1 char) and ending dot ('.')
		if((predicate_name_start == string::npos)||(predicate_ending_dot == string::npos)){
			game_initialized = false; break;
		}

		// predicate without parameters don't have parentheses, or at least not before dot '.'
		bool no_parameter_predicate = ((predicate_opening_parenthese == string::npos)||(predicate_ending_dot < predicate_opening_parenthese));

		// find another vital parts of predicate
		int predicate_closing_parenthese = (no_parameter_predicate) ? string::npos : line.find_last_of(')'); 

		// test if there is problem with predicate syntax    -    if any of these is equal to string::npos means it's not in "line" string while there should be ( bcs -> !no_parameter_predicate)
		if(!no_parameter_predicate&&((predicate_opening_parenthese == string::npos)||(predicate_closing_parenthese == string::npos))){
			game_initialized = false; break;
		}
		
		// depending on "bool no_parameter_predicate" set predicate name
		string predicate_name = (no_parameter_predicate) ? line.substr(predicate_name_start, predicate_ending_dot - predicate_name_start) : line.substr(predicate_name_start, predicate_opening_parenthese - predicate_name_start);
		
		// matching predicate names:
		// 1) predicate defining starting player	-  syntax: predicate_name(integer)
		if (!predicate_name.compare("player_turn")){			// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false; break;
			}

			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			int temp_starting_player;
			if(!(ss >> temp_starting_player)){		
				game_initialized = false; break;				// if there is problem with conversion, end while cycle
			}		

			// in init syntax, player_turn(1) means 1st player, player_turn(2) means 2nd player, which is represented in Game class as:
			// bool init_player_turn = false = 1st player    OR    bool init_player_turn = true = 1st player
			switch (temp_starting_player){
				case 1 : init_player_turn = false; continue;
				case 2 : init_player_turn = true; continue;
				default: game_initialized = false; break;
			}
			if(!game_initialized) break;						// if otherwise - value != 1 or 2

			continue;
		}

		// 2) predicate defining if player on turn must move with some object    -    syntax: predicate_name.
		if (!predicate_name.compare("player_must_move_this_turn")){		// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// if this predicate is present, set "init_player_must_move" to TRUE
			init_player_must_move = true;
			continue;
		}

		// 3) predicate defining if there is restriction which particular object must move at actual game state    -    syntax: predicate_name(integer)
		if (!predicate_name.compare("restricted_object_move_at_square")){	// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false; break;
			}

			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			if(!(ss >> init_restricted_object_move_at_square)){	// set number_of_players attribute
				game_initialized = false; break;				// if there is problem with conversion, end while cycle
			}		

			continue;
		}

		// 4) predicate defining if there are possible new objects, which can be added as new objects into game, and their count    -    syntax: predicate_name(integer,integer)
		if (!predicate_name.compare("possible_new_objects")){	// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false; break;
			}

			// find some vital parts of predicate
			int comma = line.find_first_of(',',predicate_opening_parenthese + 1);

			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if(comma == string::npos){
				game_initialized = false; break;
			}
			
			// 1st parameter - possible_new_object - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, comma - predicate_opening_parenthese - 1));

			int pno_type;
			if(!(ss >> pno_type)){
				game_initialized = false; break;
			}
			
			// 2nd parameter - possible_new_object - count (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(comma + 1, predicate_closing_parenthese - comma - 1));
			
			int pno_count;
			if(!(ss >> pno_count)){
				game_initialized = false; break;
			}

			// test if particular type of object exists in relevant collection - objectsTypes and also init_possible_new_objects
			if((pno_type < 0)||(pno_type >= (int)init_possible_new_objects.size())){
				game_initialized = false; break;
			}

			// if there is no predicate defining that particular type of object, it is 0 by default ( done in Game::init(), before Game::read_init() )
			if(init_possible_new_objects.at(pno_type) != 0){
				game_initialized = false; break;									// 0 is set by default, if another value, then it was already set, bad input
			}
			init_possible_new_objects.at(pno_type) = pno_count;
		
			continue;
		}

		// 5) predicate defining if there are possible new objects, which can be added as new objects into game, and their count is infinity    -    syntax: predicate_name(integer)
		if (!predicate_name.compare("possible_infinite_new_objects")){	// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false; break;
			}

			ss.clear();		// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, predicate_closing_parenthese - predicate_opening_parenthese - 1));
			int pno_type;
			if(!(ss >> pno_type)){								// set number_of_players attribute
				game_initialized = false; break;				// if there is problem with conversion, end while cycle
			}		

			// test if particular type of object exists in relevant collection - objectsTypes and also init_possible_new_objects
			if((pno_type < 0)||(pno_type >= (int)init_possible_new_objects.size())){
				game_initialized = false; break;
			}

			// if there is no predicate defining that particular type of object, it is 0 by default ( done in Game::init(), before Game::read_init() )
			if(init_possible_new_objects.at(pno_type) != 0){
				game_initialized = false; break;				// 0 is set by default, if another value, then it was already set, bad input
			}
			init_possible_new_objects.at(pno_type) = 777;		// set as infitity = 777
		
			continue;
		}
		

		// 6) predicate defining game squares and type of object which is on them    -    syntax: predicate_name(integer, integer)
		if (!predicate_name.compare("game_square")){			// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// this predicate should have parameters
			if(no_parameter_predicate){
				game_initialized = false; break;
			}

			// find some vital parts of predicate
			int comma = line.find_first_of(',',predicate_opening_parenthese + 1);
			
			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if(comma == string::npos){
				game_initialized = false; break;
			}
			
			// 1st parameter - game square - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(predicate_opening_parenthese + 1, comma - predicate_opening_parenthese - 1));

			int gs_index;
			if(!(ss >> gs_index)){
				game_initialized = false; break;
			}
			
			// 2nd parameter - type of object - player (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(line.substr(comma + 1, predicate_closing_parenthese - comma - 1));
			
			int gs_type;
			if(!(ss >> gs_type)){
				game_initialized = false; break;
			}

			// test to achieve ordered collection, where elements are listed in order by their type value, continuous	-	changed, not needed now
			// test to achieve ordered collection, where elements are listed in order by their type value				-	actual
			// test if particular type of object already exists in relevant collection - objectsTypes
			//if((init_game_squares.size() != gs_index)||(gs_type < 0)||(gs_type >= (int)objectsTypes.size())){
			if(((unsigned int)gs_index < init_game_squares.size())||(gs_type < 0)||(gs_type >= (int)objectsTypes.size())){
				game_initialized = false; break;
			}
				
			// predicate syntax and predicate order are OK => add new init game square with its object type
			// add new init game_state
			ChessboardSquare* chsq = env->chessboard_get_pointer_to_square(gs_index);
			
			if(chsq == NULL){
				game_initialized = false; break;
			}
			
			ObjectInfo* objtype = &objectsTypes.at(gs_type);

			// push new init game square at the end of collection "init_game_squares", it is defined as pair(pointer to chessboard_square, int defininf type of object)
			init_game_squares.push_back(make_pair(chsq,objtype));

			continue;
		}
	}

	// close file
	file_init.close();
	
	// check if while cycle end abruptly
	if(!game_initialized){
		statusMsg.append("Game init failed. Reason:\n Game don't have proper game settings e.g. predicate syntax or predicate order (specific - in file - \"");
		statusMsg.append(game_name); statusMsg.append("_init.lp\").\n");
		return false;
	}

	// everything went OK
	return true;
}

/* ------------------------------------------------------------- < create initial game_state > ------------------------------------------------------------- */

/* function gets pointer to GameState in root of GameTree and updates this GameState to initial state of game */
void Game::create_initial_state(GameState* state){
	/* first some straightforward settings */
	state->player_turn = init_player_turn;
	state->restricted_object_move_at_square = init_restricted_object_move_at_square;
	state->restricted_object_can_create_pass_move = false;
	state->player_must_move = ((init_player_must_move)||(player_must_move_each_turn));
//	state->only_add_new_objects_as_possible_move = init_only_add_new_objects_as_possible_move;

	/* now collections "gameObjects" and "gameSquares" */

		// iterate through collections gameSquares and initGameObjectsTypes, reSET data to initial vales, that means:
		// create Object if type is != 0, and link pointer to this new object to apropriate GameSquare in gameSquares
		// or set pointer to NULL if type is == 0	

	// first  -  erase all elements of collections - shouldn't be any, just to be sure
	state->gameObjects.clear();
	state->gameSquares.clear();

	// size of init_game_squares defines size of gameSquares
	state->gameSquares = vector<GameSquare>(init_game_squares.size(),GameSquare());

	// fill up collections from init collection
	int index_gs = 0;

	for(list<pair<ChessboardSquare*, ObjectInfo*>>::iterator it_init_gss = init_game_squares.begin(); it_init_gss != init_game_squares.end(); it_init_gss++){
		// set ChessboardSquare of GameSquare element
		state->gameSquares.at(index_gs).chessboardSq = it_init_gss->first;
		
		// if there is 'object' on 'game_square' defined by iterator, at initial state, create 'game_object'
		if(it_init_gss->second->type){						// if element at actual position it_init_gss_begin has type of object != 0
			// new 'game_object'
			state->gameObjects.push_back(GameObject(&(state->gameSquares.at(index_gs)), it_init_gss->second));

			state->gameSquares.at(index_gs).gameObj = &(state->gameObjects.back());
		}else{
			state->gameSquares.at(index_gs).gameObj = NULL;
		}

		index_gs++;
	}

	// copy of vector collection
	state->possible_new_objects = vector<int>(init_possible_new_objects);
};

/* ----------------------------------------------- < find possible moves, process and develope child nodes > ----------------------------------------------- */

/* develope game_tree_node means run dlv.solver, get possible_moves, check them and made resulting states from them */
bool Game::develope_game_tree_node(GameTreeNode* node){
	// check if node is already developed
	if(node == NULL) return false;
	if((node->isNodeDeveloped)) return true;

	// check if game_state in node was already evaluated as final or it is now
	if(node->isGameStateFinal()){
		return true;
	}else{
		if(check_if_game_state_is_final(node->get_game_state())) return true;
	}

	// remove previous state files - 'game_name'_stateofgame.lp and 'game_name'_rules.lp
	remove_state_files();

	// create actual game_state file
	if(!write_stateofgame_file(node->get_game_state())) return false;

	// create char* parameter which will be passed as shell command
	string dlv_files_prefix = "..\\games\\"; dlv_files_prefix.append(game_name); dlv_files_prefix.append("\\"); dlv_files_prefix.append(game_name);	// prefix to dlv files
	
	stringstream dlv_shell_command;
	dlv_shell_command << "..\\dlv\\dlv.exe -silent ";				// path to dlv executable, ('-silent') option to get only predicates as output, without junk info
	dlv_shell_command << "-pfilter=pman,pmmv,pmrm,asea,msea,rsea,asem,msem,rsem,aser,mser,rser ";	// only interested in this predicates
	dlv_shell_command << dlv_files_prefix << "_gameinfo.lp ";		// append 'game_name'_gameinfo.lp	 filepath to "dlv_shell_command" as input
	dlv_shell_command << dlv_files_prefix << "_pomocne.lp ";		// append 'game_name'_pomocne.lp	 filepath to "dlv_shell_command" as input
	dlv_shell_command << dlv_files_prefix << "_stateofgame.lp ";	// append 'game_name'_stateofgame.lp filepath to "dlv_shell_command" as input
	dlv_shell_command << dlv_files_prefix << "_rules.lp";			// append 'game_name'_rules.lp		 filepath to "dlv_shell_command" as input
	dlv_shell_command << " >> " << dlv_files_prefix << "_moves.lp";	// append 'game_name'_moves.lp		 filepath to "dlv_shell_command" as output

	// run dlv solver
	system(dlv_shell_command.str().c_str());						// commands ShellExecute or CreateProcess are another ways to run dlv solver

	// read from created "moves" file, if returned false, return false also
	if(!read_moves_file(node)) return false;

	// deal with restriction objects
	bool is_restricted_object_move_at_square_set = (node->get_game_state()->restricted_object_move_at_square == -1) ? false : true;

	// iterate through collection of "possible_moves", check if moves are legit and change resulting game_state according to this move
	// new edit: delete possible moves from collection, if not legit
//	for(unordered_map<string,GamePossibleMove>::iterator it = node->possible_moves.begin(); it != node->possible_moves.end(); it++){
	unordered_map<string,GamePossibleMove>::iterator it = node->possible_moves.begin();
	while(it != node->possible_moves.end()){
		// if there is restriction to only one possible object
		if(is_restricted_object_move_at_square_set){
			switch(it->second.type_of_move){
				case 0 :										// 'pass move' is handled separately
				case 1 : 
					{
						it = node->possible_moves.erase(it);	// new edit: delete possible move if not legit
						continue;								// no adding new objects if restriction is set
					}
				case 2 :
				case 3 :
					{
						if(it->second.gs_from == NULL) return false;
						if(it->second.gs_from->chessboardSq->index != node->get_game_state()->restricted_object_move_at_square){
							it = node->possible_moves.erase(it);	// new edit: delete possible move if not legit
							continue;								// if not that object, continue to next 'possible move'
						}
						break;
					}
				default : return false;
			}
		}
		
		// create new pair < string: possible move key value in map, child game_tree_node >, with appropriate properties and for start it will have the same game_state
		node->child_nodes.push_back(make_pair((*it).first,GameTreeNode(node->parent_game, node, node->depth + 1, (*it).first)));	// parent_game is same, node is parent for its child node and depth is +1

		if(!(process_possible_move_and_its_resulting_child_tree_node(&(*it).second, node))){
			node->child_nodes.pop_back();			// if this possible_move wasn't compatible with game_state, remove it
			it = node->possible_moves.erase(it);	// new edit: delete possible move if not legit
		}
		
		// next element in collection
		it++;
	}

	// if player_must_move is false, create 'pass move' which just change player on turn as possible_move
	// if player_must_move is true, even there is one possibilty to add 'pass move', that is if "restricted_object_move_at_square" is set, "restricted_object_can_make_pass_move" is also set true and no moves of restricted object are present
	if((!(node->get_game_state()->player_must_move))||
		((is_restricted_object_move_at_square_set)&&(node->get_game_state()->restricted_object_can_create_pass_move)&&(node->child_nodes.empty()))){
		/* create 'pass move' */ 
		GamePossibleMove gpm = GamePossibleMove(this);
		gpm.type_of_move = 0;								// type == 0  is 'pass move'
		gpm.player_continues_at_next_turn = false;
		gpm.player_must_move_at_next_turn = this->player_must_move_each_turn;
		
		/* try add it to possible moves */
		node->possible_moves.emplace(make_pair("0",gpm));

		/* create new pair < string: possible move key value in map, child game_tree_node >, with appropriate properties and for start it will have the same game_state */
		node->child_nodes.push_back(make_pair("0",GameTreeNode(node->parent_game, node, node->depth + 1, "0")));	// parent_game is same, node is parent for its child node and depth is +1

		/* and try create child game_state */
		if(!(process_possible_move_and_its_resulting_child_tree_node(&gpm, node))){
			node->child_nodes.pop_back();		// if this possible_move wasn't compatible with game_state, remove it
			node->possible_moves.erase("0");	// new edit: delete possible move if not legit
		}	
	}
	
	// check if there are any possible_moves, if not, continue according to given attribute "what_no_possible_moves_means"
	if(node->child_nodes.empty()){
		if(process_no_possible_moves(node)){	// if method "process_no_possible_moves" went ok, set node as developed and return true as good 
			node->isNodeDeveloped = true;
			return true;
		}
		return false;							// if method "process_no_possible_moves" went false, return false 
	}

	// everything went OK
	node->isNodeDeveloped = true;
	return true;
}

/* process game_tree_node which has no possible_moves, where "int what_no_possible_moves_means" - each value means another unique approach to evaluate state with no possible moves, default value (and therefore strategy) is 0 ( no moves => lose ) */
bool Game::process_no_possible_moves(GameTreeNode* node){
	switch(what_no_possible_moves_means){
	case 0 :				// no moves = lose of the player on turn
		{
			node->get_game_state()->isFinalState = true;
			node->get_game_state()->game_result = 2;		// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
			node->reward = (double)-1;						// this is value of reward for player, which is on turn at given game_tree_node
			return true;
		}
	case 1 :				// no moves = draw
		{
			node->get_game_state()->isFinalState = true;
			node->get_game_state()->game_result = 0;		// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
			node->reward = (double)0;						// this is value of reward for player, which is on turn at given game_tree_node
			return true;
		}
	case 2 :				// no moves = victory of the player on turn
		{
			node->get_game_state()->isFinalState = true;
			node->get_game_state()->game_result = 1;		// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
			node->reward = (double)1;						// this is value of reward for player, which is on turn at given game_tree_node
			return true;
		}
	default : return false; // if not known value
	}
}

/* process game_tree_node "possible_moves" collection together with "child_nodes" collection, if last added possible_move is legit, return true. Modify resulting 'game_state' to state after this 'possible_move'. */
bool Game::process_possible_move_and_its_resulting_child_tree_node(GamePossibleMove* gpm, GameTreeNode* node){
	// check if "child_nodes" is not empty
	if(node->child_nodes.empty()) return false;
	// get pointer to last added game_tree_nodes childs game_state
	GameState* child_gs = node->child_nodes.back().second.get_game_state();
	// it will have the same game_state as its parent node as init value
	(*child_gs).copy_collections_of_gs_for_child_gs(*(node->get_game_state()));

	// first check, if game_possible_move is inicialized
	if(gpm->type_of_move == -1) return false;							// thats all, wrong possible_move

	// second check, if game_possible_move is "pass move", just change player on turn, set if player must move according to global Game attribute "player_must_move_each_turn" and set "previous_move_was_pass_move" = true
	if(gpm->type_of_move == 0){
		child_gs->player_turn = !(node->get_game_state()->player_turn);	// player_turn is interpreted as bool value
		child_gs->player_must_move = this->player_must_move_each_turn;
		
		// set that it was created as 'pass_move'
		node->child_nodes.back().second.previous_move_was_pass_move = true;

		/* if previous 'game_state' was created as result of "pass_move" and now player on turn wants to also play "pass move" */
		if(node->previous_move_was_pass_move){							
			/* one more thing to check before setting 'game_state' as final ( first check pointers) */
			if(node->parent_node != NULL){
				/* if previous 'pass_move' was result of no possible moves for some 'restricted object' ( that means that particular object made a move and now can't move further, but it doesn't mean that another object of that player can't make some move ) */
				if(node->parent_node->get_game_state()->restricted_object_move_at_square != -1){
					node->child_nodes.back().second.previous_move_was_pass_move = true;					// it means that it is not final 'game_state' yet
					return true;
				}
			}
			/* it means that is final 'game state' */
			child_gs->isFinalState = true;								
			child_gs->game_result = 0;									// and it is DRAW
			node->child_nodes.back().second.reward = (double)0;	
		}
		
		// thats all, good possible_move
		return true;											
	}

	// change some game_state according to possible_move attributes
	child_gs->player_must_move = (this->player_must_move_each_turn) ? true : gpm->player_must_move_at_next_turn;
	child_gs->player_turn = (gpm->player_continues_at_next_turn) ? node->get_game_state()->player_turn : !(node->get_game_state()->player_turn);	// if "player_continues_at_next_turn" is false, change "player_turn", which is bool value
	
	child_gs->player1_score += gpm->reward_player_1;
	child_gs->player2_score += gpm->reward_player_2;

	// create collection of moves, first add -> side_effects
	list<GameMove_SideEffect> moves = gpm->side_effects;
	
	// add 'add_new_object' move according to main 'possible_move' to collection of "moves"
	if(gpm->type_of_move == 1){				// 1 = add new object
		// change some more game_state according to possible_move attributes
		if(gpm->restrict_move_only_to_this_object) child_gs->restricted_object_move_at_square = gpm->gs_to->chessboardSq->index;	// if "restricted_object_move_at_square" is true, set it as 'game_square' index where this new object is added
		if(gpm->restricted_object_can_make_pass_move) child_gs->restricted_object_can_create_pass_move = true;						// if "restricted_object_can_make_pass_move" is true, set it also in 'game_state'
		
		// create this new move and push it to end of collection "moves"
		moves.push_back(GameMove_SideEffect(gpm));
	}

	// add 'move_object' move according to main 'possible_move' to collection of "moves"
	if(gpm->type_of_move == 2){				// 2 = move object
		// change some more game_state according to possible_move attributes
		if(gpm->restrict_move_only_to_this_object) child_gs->restricted_object_move_at_square = gpm->gs_to->chessboardSq->index;	// if "restricted_object_move_at_square" is true, set it as game_square index where this new object is added
		if(gpm->restricted_object_can_make_pass_move) child_gs->restricted_object_can_create_pass_move = true;						// if "restricted_object_can_make_pass_move" is true, set it also in 'game_state'

		// create this new move and push it to end of collection "moves"
		moves.push_back(GameMove_SideEffect(gpm));
	}

	// add 'remove_object' move according to main 'possible_move' to collection of "moves"
	if(gpm->type_of_move == 3){				// 2 = move object
		// create this new move and push it to end of collection "moves"
		moves.push_back(GameMove_SideEffect(gpm));				// add_to_new_possible_moves is also preserved in this conversion from "GamePossibleMove" to "GameMove_SideEffect"
	}

	// iterate through collection "moves" and try to make the move
	list<GameMove_SideEffect>::iterator it = moves.begin();

	while(it != moves.end()){
		// try to make the move
		switch((*it).type_of_move){		
			/* move is 'add_new_object' */
			case 1 :
				{	
					/* check pointers 1) */
					if((*it).gs_to == NULL) return false;
					if((*it).gs_to->chessboardSq == NULL) return false;

					/* create local pointer, *it.gs_to is pointer within parent 'game_state' (in "node") */
					GameSquare* it_local_gs_to = child_gs->get_game_squares_with_chsq_index((*it).gs_to->chessboardSq->index);
	
					/* check pointers 2) */
					if((it_local_gs_to == NULL)||((*it).obj_after_type == NULL)) return false;
					if((*it).obj_after_type->type == 0) return false;							// add empty object ( type == 0) is not legit

					/* legit 'add_new_object' move must justify 2 conditions: */
					// 1.1) empty 'game_square' where to add 
					if(it_local_gs_to->gameObj != NULL) break;		// if 'game_square' where to add new object is not empty, exit switch block, and continue with next move

					// 1.2) there must be at least one new object in 'possible_new_objects' of given 'object_type'
					int temp_value = 0;																							// value of how many possible new objects are there for this type of 'object_type'	
					if(!child_gs->get_possible_new_objects_value_at_index((*it).obj_after_type->type,temp_value)) return false;	// set variable, if bad index, return false
					if(temp_value < 1) break;	// if there are no possible new object of this type, exit switch block, and continue with next move

					/* move is legit, do these things: */
					// change 'game_state' according to this move
					child_gs->gameObjects.push_back(GameObject(it_local_gs_to,(*it).obj_after_type));	// add new 'GameObject', which is in appropriate 'game_square' and 'ObjectInfo'
					it_local_gs_to->gameObj = &(child_gs->gameObjects.back());							// add pointer to this new 'GameObject' to 'game_square'->'object'						
					child_gs->change_possible_new_objects((*it).obj_after_type->type,-1);				// change value of possible new objects for that type of object
					
					// add it to "gpm"s "pm_move_for_roboArm" collection
					gpm->pm_move_for_roboArm.push_back(GameSimpleFinalMove(gpm->parent_game, NULL, it_local_gs_to, (*it).obj_after_type));
					
					// remove it from "moves" collection
					it = moves.erase(it);

					// continue from beggining of collection "moves"
					it = moves.begin();
					continue;
				}

			/* move is 'move_object' */
			case 2 :
				{	
					/* check pointers 1) */
					if(((*it).gs_from == NULL)||((*it).gs_to == NULL)) return false;
					if(((*it).gs_from->chessboardSq == NULL)||((*it).gs_to->chessboardSq == NULL)) return false;

					/* create local pointers, *it.gs_from and *it.gs_to are pointers within parent 'game_state' (in "node") */
					GameSquare* it_local_gs_from = child_gs->get_game_squares_with_chsq_index((*it).gs_from->chessboardSq->index);
					GameSquare* it_local_gs_to	 = child_gs->get_game_squares_with_chsq_index((*it).gs_to->chessboardSq->index);
					
					/* check pointers 2) */
					if((it_local_gs_from == NULL)||(it_local_gs_to == NULL)||((*it).obj_before_type == NULL)||((*it).obj_after_type == NULL)) return false;
					if((*it).obj_before_type == 0) return false;	// move empty object ( type == 0 ) is not legit, "obj_after_type->type" can be 0, that means it will be removed after move

					/* legit 'move_object' move must justify 3 conditions: */
					// 2.1) empty 'game_square' where to move 'object' 
					if(it_local_gs_to->gameObj != NULL) break;										// if 'game_square' where to add new object is not empty, exit switch block, and continue with next move
					
					// 2.2) objects 'type' at 'game_square' from where to move is the same as given in 'move'
					if(it_local_gs_from->gameObj == NULL) break;									// no object at 'game_square' from where to move, exit switch block, and continue with next move
					if(it_local_gs_from->gameObj->objInfo != (*it).obj_before_type) break;			// if objects 'type' at 'game_square' from where to move is not as it should be according to 'move' attributes, exit switch block, and continue with next move
					
					// 2.3) if 'ObjectType's "after" and "before" are not the same, there must be at least one new object in 'possible_new_objects' of given 'object_type' "after" ( if they are not the same, "before" 'type' 'object' is removed, and afterwards added new object of 'type' "after" )
					if(((*it).obj_before_type != (*it).obj_after_type)&&((*it).obj_after_type->type != 0)){							// if 'type's are not the same
						int temp_value = 0;																							// value of how many possible new objects are there for this type of 'object_type'
						if(!child_gs->get_possible_new_objects_value_at_index((*it).obj_after_type->type,temp_value)) return false;	// set variable, if bad index, return false
						if(temp_value < 1) break;	// if there are no possible new object of this type, exit switch block, and continue with next move
					}
	
					/* move is legit, do these things: */
					// change 'game_state' according to this move
					// move 'object' from 'old position' to 'new position' ( relations between 'object's and 'game_square's are represented by bilateral pointers )
					it_local_gs_from->gameObj->gameSq = it_local_gs_to;								// "GameObject" at "gs_from" will be linked to 'new position' 'game_square'
					it_local_gs_to->gameObj = it_local_gs_from->gameObj;							// 'new position' 'game_square' will get pointer to its 'new' "GameObject", ( (*it).gs_to->gameObj was already checked, that it was NULL, so no need to change anything more )
					it_local_gs_from->gameObj = NULL;												// 'old position' 'game_square' won't have any "GameObject"

					if((*it).obj_before_type != (*it).obj_after_type){								// if 'type's are not the same
						if((*it).obj_after_type->type != 0){										// if after type is not empty object
							it_local_gs_to->gameObj->objInfo = (*it).obj_after_type;				// 'new position' "GameObject" will get 'new type'
							child_gs->change_possible_new_objects((*it).obj_after_type->type,-1);	// change value of possible new objects for that type of object
						}else{																		// if after type is empty object
							it_local_gs_to->gameObj->objInfo = NULL;								// 'new position' "GameObject" will be empty (NULL)
						}
					}

					// add it to "gpm"s "pm_move_for_roboArm" collection
					gpm->pm_move_for_roboArm.push_back(GameSimpleFinalMove(gpm->parent_game, it_local_gs_from, it_local_gs_to, NULL));				// move 'old type' to 'new position'
					if((*it).obj_before_type != (*it).obj_after_type){																				// if 'type's are not the same
						gpm->pm_move_for_roboArm.push_back(GameSimpleFinalMove(gpm->parent_game, it_local_gs_to, NULL, NULL));						// remove 'old type' from 'new position'
						if((*it).obj_after_type->type != 0){																						// if 'new type' is not empty object
							gpm->pm_move_for_roboArm.push_back(GameSimpleFinalMove(gpm->parent_game, NULL, it_local_gs_to, (*it).obj_after_type));	// add 'new type' to 'new position'
						}
					}

					// remove it from "moves" collection
					it = moves.erase(it);

					// continue from beggining of collection "moves"
					it = moves.begin();
					continue;
				}

			/* move is 'remove_object' */
			case 3 :
				{	
					/* check pointers 1) */
					if((*it).gs_from == NULL) return false;
					if((*it).gs_from->chessboardSq == NULL) return false;

					/* create local pointer, *it.gs_from is pointer within parent 'game_state' (in "node") */
					GameSquare* it_local_gs_from = child_gs->get_game_squares_with_chsq_index((*it).gs_from->chessboardSq->index);
										
					/* check pointers 2) */
					if((it_local_gs_from == NULL)||((*it).obj_before_type == NULL)) return false;
					if((*it).obj_before_type->type == 0) return false;	// remove empty object ( type == 0) is not legit
					
					/* legit 'remove_object' move must justify 2 conditions: */
					// 3.1) not empty 'game_square' from where the object is removed in 'move'
					if(it_local_gs_from->gameObj == NULL) break;								// if 'game_square' from where the object is removed is empty, exit switch block, and continue with next 'move'
					
					// 3.2) objects 'type' at 'game_square' from where the object is removed is the same as given in 'move' 
					if(it_local_gs_from->gameObj->objInfo != (*it).obj_before_type) break;		// if objects 'type' at 'game_square' from where the object is removed is not as it should be according to 'move' attributes, exit switch block, and continue with next 'move'
						
					/* move is legit, do these things: */
					// change 'game_state' according to this move
					// move 'object' from 'old position' to 'new position' ( relations between 'object's and 'game_square's are represented by bilateral pointers )
					it_local_gs_from->gameObj = NULL;											// "GameObject" at "gs_from" will be removed at next step, so set its pointer to NULL first
					child_gs->remove_object_at_game_square(it_local_gs_from);					// remove "GameObject" from collection "gameObjects" in "GameState"
					
					if((*it).add_to_new_objects_after_remove){									// if according to 'move' this removed objects type should be added to 'new possible objects' types
						child_gs->change_possible_new_objects((*it).obj_before_type->type,1);	// change value of possible new objects for that type of object
					}

					// add it to "gpm"s "pm_move_for_roboArm" collection
					gpm->pm_move_for_roboArm.push_back(GameSimpleFinalMove(gpm->parent_game, it_local_gs_from, NULL, NULL));
					
					// remove it from "moves" collection
					it = moves.erase(it);

					// continue from beggining of collection "moves"
					it = moves.begin();
					continue;
				}
			default : return false;			// should not be another type of move here at this time, so if so then return false
		}
		
		// move couldn't be made at this state og 'game_state', try next move in collection "moves"
		it++;
	}
	
	// there are 2 possibilities after previous 'while cycle':
	// - either collection "moves" is empty, so every 'move' could be performed and thus given 'possible_move' and resulting 'game_state' are legit
	// - or otherwise, so there is at least one 'move' that could not be performed, so given 'possible_move' and resulting 'game_state' are not legit
	return moves.empty();
}

/* takes 'game_state' as parameter, return bool value if it is final according to attribute "how_to_check_final_state" */
bool Game::check_if_game_state_is_final(GameState* gs){
	switch(how_to_check_final_state){
		case 0 : return false;		// final if no moves, so this function does nothing, in this case, 'game state' is set by function "process_no_possible_moves"
		case 1 :					// for game 'frogs'
			{
				if(gs->gameSquares.size() % 2 != 1) return false;			// playable_area size must be odd number
				unsigned int middle_position = gs->gameSquares.size()/2;

				for(unsigned int i = 0; i < gs->gameSquares.size(); i++){
					if(gs->gameSquares[i].gameObj != NULL){
						if((i < middle_position)&&(gs->gameSquares[i].gameObj->objInfo->type != 2)) return false;
						if((i > middle_position)&&(gs->gameSquares[i].gameObj->objInfo->type != 1)) return false;		
					}else{if(i != middle_position) return false;}		// no object is represented as NULL
				}

				// 'game_state' is final
				gs->game_result = 1;		// win
				gs->isFinalState = true;
				return true;
			}
		case 2 :				// for game 'connect4'
			{		
				// some init values
				int temp_count = 0, goal_count = 4, current_row = 0, current_column = 0, temp_index = 0;
				bool previous_check_player = false, is_object_on_previous_square = false;

				// chceck if playable_area is set and find top_left_corner as beggining of playable rectangle
				if((playable_area.width < 0)||(playable_area.height < 0)) return (double)0;
				if((playable_area.width < goal_count)||(playable_area.height < goal_count)) return (double)0;
				// chceck if enough objects to reach goal_state and if playable_area is set
				if(gs->gameObjects.size() < (unsigned int)goal_count) return false;
				
				// check values for players in vertical (bottom to top) direction
				for(int i = 0; i < playable_area.width; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height-1) + i;
					// current row at playable area starts at bottom one
					current_row =  playable_area.height - 1;
					// if no object continue to next (right+1) 'game_square'
					if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;
					// set initial values
					previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
					temp_count = 0;

					while(true){
						if((temp_index < 0)||(current_row < 0)) break;
						if(gs->gameSquares.at(temp_index).gameObj == NULL) break;

						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						
						// chceck one 'game_square' up
						temp_index -= playable_area.width;
						current_row -= 1;
					}
				}
				
				// check values for players in horizontal (left to right) direction - starting from positions on 'left column' except the ones, which can't reach goal_count
				for(int i = 0; i < playable_area.height; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = i*playable_area.width;					
					current_row = i; current_column = 0;	// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index += 1;
						current_column += 1;
						// conditions to break while cycle
						if(current_column >= playable_area.width) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// check values for players in diagonal (left to right) direction - starting from positions on 'left column' except bottom one and the ones, which can't reach goal_count
				for(int j = goal_count - 1; j < playable_area.height - 1; j++){
					// temp_index is index to bottom 'game_squares'
					temp_index = j*playable_area.width;					
					current_row = j; current_column = 0;	// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width - 1;
						current_row -= 1; current_column += 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (left to right) direction - starting from positions on 'bottom' except the ones, which can't reach goal_count
				for(int i = 0; i <= playable_area.width - goal_count; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height-1) + i;
					current_row = playable_area.height - 1; current_column = i;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width - 1;
						current_row -= 1; current_column += 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (right to left) direction - starting from positions on 'right column' except bottom one and the ones, which can't reach goal_count
				for(int j = goal_count - 1; j < playable_area.height - 1; j++){
					// temp_index is index to bottom 'game_squares'
					temp_index = (playable_area.width - 1) + j*playable_area.width;
					current_row = j; current_column = playable_area.width - 1;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width + 1;
						current_row -= 1; current_column -= 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
							is_object_on_previous_square = true; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (left to right) direction - starting from positions on 'bottom' except the ones, which can't reach goal_count
				for(int i = goal_count - 1; i < playable_area.width; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height - 1) + i;
					current_row = playable_area.height - 1; current_column = i;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width + 1;
						current_row -= 1; current_column -= 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
							if(temp_count >= goal_count){
								gs->game_result = (gs->player_turn == previous_check_player) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
								gs->isFinalState = true;
								return true;	
							}
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}
							
				// not a final 'game_state'
				return false;
			}
		case 3 :
			{
				bool isPlayer_1_present = false, isPlayer_2_present = false;

				for(list<GameObject>::iterator it = gs->gameObjects.begin(); it != gs->gameObjects.end(); it++){
					if(it->objInfo != NULL){
						if(it->objInfo->player){isPlayer_2_present = true;}else{isPlayer_1_present = true;}
						if(isPlayer_1_present&&isPlayer_2_present) return false;
					}
				}

				if(!(isPlayer_1_present||isPlayer_2_present)){		// if empty objects
					gs->game_result = 0;							// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
					gs->isFinalState = true;
					return true;	
				}

				if(isPlayer_2_present){								// if player 2 wins
					gs->game_result = (gs->player_turn) ? 1 : 2;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
					gs->isFinalState = true;
					return true;
				}else{												// if player 1 wins
					gs->game_result = (gs->player_turn) ? 2 : 1;	// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
					gs->isFinalState = true;
					return true;
				}				
			}

		default : return false;
	}
};

/* takes 'game_state' as parameter, return double value of heuristic aproximation of game 'reward', according to attribute "type_of_heuristics"
   each value means another unique approach to evaluate heuristic value of 'game state', default is 0 (this default value will return 0, median of <-1,1>) */
double Game::evaluate_heuristic_value(GameState* gs, bool according_to_player, int type_of_heuristics = 0){
	switch(type_of_heuristics){
		case 0 : return (double)0;		// default strategy is 0 (this default value will return 0, median of <-1,1>))
		/* for game 'frogs' */
		case 1 :				
			{
				// en.wikipedia.org/wiki/Hamming_distance
				int count_of_errors = 0;
				
				if(gs->gameSquares.size() % 2 != 1) return (double)-2;		// playable_area size must be odd number
				unsigned int middle_position = gs->gameSquares.size()/2;

				for(unsigned int i = 0; i < gs->gameSquares.size(); i++){
					if(gs->gameSquares[i].gameObj != NULL){
						if((i < middle_position)&&(gs->gameSquares[i].gameObj->objInfo->type != 2)) count_of_errors++;
						if((i > middle_position)&&(gs->gameSquares[i].gameObj->objInfo->type != 1)) count_of_errors++;		
					}else{if(i != middle_position) count_of_errors++;}		// no object is represented as NULL
				}

				return (double)(gs->gameSquares.size() - 2*count_of_errors) / (double)gs->gameSquares.size();
			}
		/* for game 'connect4' , heuristic approach 1 */
		case 2 :				
			{
				/* IDEA: for each possibility of completing 'line' of appropriate length (a.k.a. "goal_count") player will get 
				   amount of reward which is adequate to that possibility (a.k.a. temp length of given 'line' ) */

				// some init values
				int temp_count = 0, goal_count = 4, current_row = 0, current_column = 0, temp_index = 0;
				double player1_value = 1, player2_value = 1, partial_reward = 5;			// player_values init as 1, so no possibility of dividing by 0
				bool previous_check_player = false, is_object_on_previous_square = false;

				// chceck if playable_area is set and find top_left_corner as beggining of playable rectangle
				if((playable_area.width < 0)||(playable_area.height < 0)) return (double)0;
				if((playable_area.width < goal_count)||(playable_area.height < goal_count)) return (double)0;
				
				// set values for players in vertical (bottom to top) direction
				for(int i = 0; i < playable_area.width; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height-1) + i;
					// current row at playable area starts at bottom one
					current_row = playable_area.height - 1;
					// if no object continue to next (right+1) 'game_square'
					if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;
					// set initial values
					previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
					temp_count = 0;

					while(true){
						if((temp_index < 0)||(current_row < 0)) break;
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							break;
						}

						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						
						// chceck one 'game_square' up
						temp_index -= playable_area.width;
						current_row -= 1;
					}
				}
				
				// add values for players in horizontal (left to right) direction - starting from positions on 'left column' except the ones, which can't reach goal_count
				for(int i = 0; i < playable_area.height; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = i*playable_area.width;					
					current_row = i; current_column = 0;	// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index += 1;
						current_column += 1;
						// conditions to break while cycle
						if(current_column >= playable_area.width) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (left to right) direction - starting from positions on 'left column' except bottom one and the ones, which can't reach goal_count
				for(int j = goal_count - 1; j < playable_area.height - 1; j++){
					// temp_index is index to bottom 'game_squares'
					temp_index = j*playable_area.width;					
					current_row = j; current_column = 0;	// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width - 1;
						current_row -= 1; current_column += 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (left to right) direction - starting from positions on 'bottom' except the ones, which can't reach goal_count
				for(int i = 0; i <= playable_area.width - goal_count; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height-1) + i;
					current_row = playable_area.height - 1; current_column = i;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width - 1;
						current_row -= 1; current_column += 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (right to left) direction - starting from positions on 'right column' except bottom one and the ones, which can't reach goal_count
				for(int j = goal_count - 1; j < playable_area.height - 1; j++){
					// temp_index is index to bottom 'game_squares'
					temp_index = (playable_area.width - 1) + j*playable_area.width;
					current_row = j; current_column = playable_area.width - 1;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width + 1;
						current_row -= 1; current_column -= 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}

				// add values for players in diagonal (left to right) direction - starting from positions on 'bottom' except the ones, which can't reach goal_count
				for(int i = goal_count - 1; i < playable_area.width; i++){
					// temp_index is index to bottom 'game_squares'
					temp_index = playable_area.width*(playable_area.height - 1) + i;
					current_row = playable_area.height - 1; current_column = i;				// set current row and column values
					// set initial values
					is_object_on_previous_square = gs->gameSquares.at(temp_index).gameObj != NULL;
					if(is_object_on_previous_square){
						previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player; temp_count = 1;
					}else{ temp_count = 0; }
					
					while(true){
						// check one 'game_square' up and right
						temp_index -= playable_area.width + 1;
						current_row -= 1; current_column -= 1;
						// conditions to break while cycle
						if((temp_index < 0)||(current_row < 0)||(current_column >= playable_area.width)) break;
						// if no object at 'game_square'
						if(gs->gameSquares.at(temp_index).gameObj == NULL){
							if(temp_count > 0){
								if(previous_check_player){player2_value += std::pow(partial_reward, temp_count);}else{player1_value += std::pow(partial_reward, temp_count);}
							}
							is_object_on_previous_square = false; temp_count = 0; continue;
						}
						// if no object at previous 'game_square'
						if(!is_object_on_previous_square){
							previous_check_player = gs->gameSquares.at(temp_index).gameObj->objInfo->player;
							is_object_on_previous_square = true; temp_count = 1; continue;
						}
						//
						if(gs->gameSquares.at(temp_index).gameObj->objInfo->player == previous_check_player){
							temp_count++;
							// if "goal_count" or more, then this should be final state and player "previous_check_player" should win
//							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
							if(temp_count >= goal_count) return (according_to_player == previous_check_player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						}else{
							previous_check_player = !previous_check_player;
							temp_count = 1;
						}
						is_object_on_previous_square = true;
					}
				}
				
				// return heuristic value
				if(!according_to_player)	// if player1 (player == false) on turn
					return (player1_value >= player2_value) ? std::min((player1_value/player2_value - (double)1)/(double)9, (double)0.995) : std::max(((double)-1*player2_value/player1_value + (double)1)/(double)9, (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
				if(according_to_player)		// if player2 (player == true) on turn
					return (player2_value >= player1_value) ? std::min((player2_value/player1_value - (double)1)/(double)9, (double)0.995) : std::max(((double)-1*player1_value/player2_value + (double)1)/(double)9, (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
				break;
			}			
		/* for game 'connect4', heuristic approach 2 */
		case 3 :				
			{
				/* IDEA: for each possibility of completing 'line' of appropriate length (a.k.a. "goal_count") player will get 
				   amount of reward which is adequate to that possibility (a.k.a. temp length of given 'line' ), now also taking gap between object into account */

				// some init values
				int goal_count = 4;
				double player1_value = 1, player2_value = 1, partial_reward = 5;	// player_values init as 1, so no possibility of dividing by 0
				
				// chceck if playable_area is set and find top_left_corner as beggining of playable rectangle
				if((playable_area.width < 0)||(playable_area.height < 0)) return (double)0;
				if((playable_area.width < goal_count)||(playable_area.height < goal_count)) return (double)0;
				
				int pa_tlc_row    = playable_area.left_top_corner / env->chessboard_get_width();		// get row    position of PA-TLC
				int pa_tlc_column = playable_area.left_top_corner % env->chessboard_get_width();		// get column position of PA-TLC

				int obj_row, obj_column, obj_pa_row, obj_pa_column, obj_pa_index, obj_pa_row_temp, obj_pa_column_temp, obj_pa_index_temp;
				int player_count, empty_space_before_object, consecutive_count, max_consecutive_count, possible_space_for_objects_of_player;
				bool first_in_possible_solution;

				// check every object if it can be 'start' of one possible solution for given player
				for(list<GameObject>::iterator it_obj = gs->gameObjects.begin(); it_obj != gs->gameObjects.end(); it_obj++){
					if(it_obj->objInfo == NULL) continue;
					
					// objects starting 'game_Square' index informations
					obj_row    = it_obj->gameSq->chessboardSq->index / env->chessboard_get_width();	// get row    position of 'object'
					obj_column = it_obj->gameSq->chessboardSq->index % env->chessboard_get_width();	// get column position of 'object'
					
					obj_pa_row	  = obj_row - pa_tlc_row;											// position of 'object' accordingly to 'playable_area'
					obj_pa_column = obj_column - pa_tlc_column;										// position of 'object' accordingly to 'playable_area'
					obj_pa_index  = obj_pa_row*playable_area.width + obj_pa_column;					// index of 'object' in vector of 'game_squares'

					/* check 1 - solution if object is first vertically of this player kind of object (from bottom to top) */
					obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;	player_count = 1;
					first_in_possible_solution = false;
					
					if(obj_pa_row == (playable_area.height -1)){first_in_possible_solution = true;}
					else{
						obj_pa_index_temp += playable_area.width;
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL) return 0;				// wrong 'game_state'
						first_in_possible_solution = (gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player != it_obj->objInfo->player);
					}

					if(first_in_possible_solution){
						obj_pa_index_temp = obj_pa_index;
						for(int i = 0; i < (goal_count - 1); i++){
							// one row up
							obj_pa_row_temp--;
							if(obj_pa_row_temp < 0){player_count = 0; break;}

							obj_pa_index_temp -= playable_area.width;
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL) break;
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){ player_count++; continue; }
								else{ player_count = 0; break; }
						}
//						if(player_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						if(player_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						if(player_count > 0){
							if(it_obj->objInfo->player){player2_value += std::pow(partial_reward, player_count);}else{player1_value += std::pow(partial_reward, player_count);}
						}
					}	
					
					/* check 2 - solution in horizontal direction (from left to right) */
					// first check if this would be beginning of new solution
					obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
					empty_space_before_object = 0; first_in_possible_solution = true;

					for(int i = 0; i < (goal_count - 1); i++){
						// set new column to check, one left
						obj_pa_column_temp--;
						// if out of playable area, break
						if(obj_pa_column_temp < 0) break;
						// check game_square one left
						obj_pa_index_temp -= 1;
						// set player_count appropriately
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ empty_space_before_object++; continue; }
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){ first_in_possible_solution = false; break;}
							else{ break; }
					}

					if(first_in_possible_solution){
						// check if this could be first object in any new solution
						obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
						consecutive_count = 1; max_consecutive_count = 1; possible_space_for_objects_of_player = -1;
						
						for(int i = 0; i < (goal_count - 1); i++){
							// set new row and column to check, one up and one left
							obj_pa_column_temp++;
							// if out of playable area, break
							if(obj_pa_column_temp >= playable_area.width){ possible_space_for_objects_of_player = i; break; }
							// check game_square one up and one left
							obj_pa_index_temp += 1;
							// set player_count appropriately
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ consecutive_count = 0; continue;}
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){
								 consecutive_count++; max_consecutive_count = std::max(consecutive_count, max_consecutive_count);
							}else{ possible_space_for_objects_of_player = i; break; }
						}
						// if it's final state
//						if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						// if there is enough space to fit
						if(((possible_space_for_objects_of_player + 1 + empty_space_before_object) >= goal_count)||(possible_space_for_objects_of_player == -1)){
							if(it_obj->objInfo->player){player2_value += std::pow(partial_reward, max_consecutive_count);}else{player1_value += std::pow(partial_reward, max_consecutive_count);}
						}
					}

					/* check 3 - solution in diagonal direction (from right to left, from bottom to top) */
					// first check if this would be beginning of new solution
					obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
					empty_space_before_object = 0; first_in_possible_solution = true;
					
					for(int i = 0; i < (goal_count - 1); i++){
						// set new row and column to check, one down and one to right
						obj_pa_row_temp++; obj_pa_column_temp++;
						// if out of playable area, break
						if((obj_pa_row_temp >= playable_area.height)||(obj_pa_column_temp >= playable_area.width)) break;
						// check game_square one down and one right
						obj_pa_index_temp += playable_area.width + 1;
						// set player_count appropriately
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ empty_space_before_object++; continue; }
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){ first_in_possible_solution = false; break; }
							else{ break; }
					}
					
					if(first_in_possible_solution){
						// check if this could be first object in any new solution
						obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
						consecutive_count = 1; max_consecutive_count = 1; possible_space_for_objects_of_player = -1;

						for(int i = 0; i < (goal_count - 1); i++){
							// set new row and column to check, one up and one to left
							obj_pa_row_temp--; obj_pa_column_temp--;
							// if out of playable area, break
							if((obj_pa_row_temp < 0)||(obj_pa_column_temp < 0)){ possible_space_for_objects_of_player = i; break; };
							// check game_square one up and one left
							obj_pa_index_temp -= playable_area.width + 1;
							// set player_count appropriately
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ consecutive_count = 0;  continue; }
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){
								 consecutive_count++; max_consecutive_count = std::max(consecutive_count, max_consecutive_count);
							}else{ possible_space_for_objects_of_player = i; break; }
						}
						// if it's final state
	//					if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						// if there is enough space to fit
						if(((possible_space_for_objects_of_player + 1 + empty_space_before_object) >= goal_count)||(possible_space_for_objects_of_player == -1)){
							if(it_obj->objInfo->player){player2_value += std::pow(partial_reward, max_consecutive_count);}else{player1_value += std::pow(partial_reward, max_consecutive_count);}
						}
					}

					/* check 4 - solution in diagonal direction (from left to right, from bottom to top) */
					// first check if this would be beginning of new solution
					obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
					empty_space_before_object = 0; first_in_possible_solution = true;
					
					for(int i = 0; i < (goal_count - 1); i++){
						// set new row and column to check, one down and one to left
						obj_pa_row_temp++; obj_pa_column_temp--;
						// if out of playable area, break
						if((obj_pa_row_temp >= playable_area.height)||(obj_pa_column_temp < 0)) break;
						// check game_square one down and one right
						obj_pa_index_temp += playable_area.width - 1;
						// set player_count appropriately
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ empty_space_before_object++; continue; }
						if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){ first_in_possible_solution = false; break; }
							else{ break; }
					}

					if(first_in_possible_solution){
						// check if this could be first object in any new solution
						obj_pa_row_temp = obj_pa_row; obj_pa_column_temp = obj_pa_column; obj_pa_index_temp = obj_pa_index;
						consecutive_count = 1; max_consecutive_count = 1; possible_space_for_objects_of_player = -1;

						for(int i = 0; i < (goal_count - 1); i++){
							// set new row and column to check, one up and one to right
							obj_pa_row_temp--; obj_pa_column_temp++;
							// if out of playable area, break
							if((obj_pa_row_temp < 0)||(obj_pa_column_temp >= playable_area.width)){ possible_space_for_objects_of_player = i; break; };
							// check game_square one up and one left
							obj_pa_index_temp -= playable_area.width - 1;
							// set player_count appropriately
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj == NULL){ consecutive_count = 0;  continue; }
							if(gs->gameSquares.at(obj_pa_index_temp).gameObj->objInfo->player == it_obj->objInfo->player){
								 consecutive_count++; max_consecutive_count = std::max(consecutive_count, max_consecutive_count);
							}else{ possible_space_for_objects_of_player = i; break; }
						}
						// if it's final state
	//					if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)0.999 : (double)-0.999;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						if(max_consecutive_count >= goal_count) return (according_to_player == it_obj->objInfo->player) ? (double)1 : (double)-1;	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state, 0.995 is if heuristic function is not sure, but thinks it might be best approach
						// if there is enough space to fit
						if(((possible_space_for_objects_of_player + 1 + empty_space_before_object) >= goal_count)||(possible_space_for_objects_of_player == -1)){
							if(it_obj->objInfo->player){player2_value += std::pow(partial_reward, max_consecutive_count);}else{player1_value += std::pow(partial_reward, max_consecutive_count);}
						}
					}
				}

				// return heuristic value
				if(!according_to_player)	// if player1 (player == false) on turn
					return (player1_value >= player2_value) ? std::min((player1_value/player2_value - (double)1)/(double)9, (double)0.995) : std::max(((double)-1*player2_value/player1_value + (double)1)/(double)9, (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state
				if(according_to_player)		// if player2 (player == true) on turn
					return (player2_value >= player1_value) ? std::min((player2_value/player1_value - (double)1)/(double)9, (double)0.995) : std::max(((double)-1*player1_value/player2_value + (double)1)/(double)9, (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state
				break;
			}
		/* for game 'alquerque', heuristic approach 1 */
		case 4 :		
			{
				int player_1_count = 0, player_2_count = 0;

				for(list<GameObject>::iterator it = gs->gameObjects.begin(); it != gs->gameObjects.end(); it++){
					if(it->objInfo == NULL) continue;
					
					if(it->objInfo->player){player_2_count++;}else{player_1_count++;}
				}
				
				if(!according_to_player)
					return (player_1_count >= player_2_count) ? std::min((player_1_count/(double)player_2_count - (double)1)/(double)(gs->gameObjects.size() - 1), (double)0.995) : std::max(((double)-1*player_2_count/(double)player_1_count + (double)1)/(double)(gs->gameObjects.size() - 1), (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.995 is if heuristic function knows it's winning state
				if(according_to_player)
					return (player_2_count >= player_1_count) ? std::min((player_2_count/(double)player_1_count - (double)1)/(double)(gs->gameObjects.size() - 1), (double)0.995) : std::max(((double)-1*player_1_count/(double)player_2_count + (double)1)/(double)(gs->gameObjects.size() - 1), (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.995 is if heuristic function knows it's winning state
				break;
			}
		/* for game 'alquerque', heuristic approach 2 */
		case 5 :		
			{
				int player_1_count = 1, player_2_count = 1;												// init = 1, so there will be no division by 0
				int player_1_not_endangered_count = 1, player_2_not_endangered_count = 1;				// init = 1, so there will be no division by 0
				int player_1_total_row_sum = 0, player_2_total_row_sum = 0;								// total sum of distance for every object for given player, in order to support forward move for players
				int pa_tlc_row    = playable_area.left_top_corner / env->chessboard_get_width();		// get row    position of PA-TLC
				int pa_tlc_column = playable_area.left_top_corner % env->chessboard_get_width();		// get column position of PA-TLC

				for(list<GameObject>::iterator it = gs->gameObjects.begin(); it != gs->gameObjects.end(); it++){
					if(it->objInfo == NULL) continue;
					
					int obj_row    = it->gameSq->chessboardSq->index / env->chessboard_get_width();		// get row    position of 'object'
					int obj_column = it->gameSq->chessboardSq->index % env->chessboard_get_width();		// get column position of 'object'
					
					int obj_pa_row	  = obj_row - pa_tlc_row;											// position of 'object' accordingly to 'playable_area'
					int obj_pa_column = obj_column - pa_tlc_column;										// position of 'object' accordingly to 'playable_area'
					int obj_pa_index  = obj_pa_row*playable_area.width + obj_column;					// index of 'object' in vector of 'game_squares'
					
					if(it->objInfo->player){
						player_2_count++;
						player_2_total_row_sum += obj_pa_row;
					}else{
						player_1_count++;
						player_1_total_row_sum += obj_pa_row;
					}

					// check if object is endangerd from above
					int temp_index = obj_pa_index, temp_row = obj_pa_row, temp_column = obj_pa_column;					
					temp_row--; temp_index -= playable_area.width;
					if(temp_row >= 0){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row += 2; temp_index += 2*playable_area.width;
								if(temp_row < playable_area.height)
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from below
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;					
					temp_row++; temp_index += playable_area.width;
					if(temp_row < playable_area.height){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row -= 2; temp_index -= 2*playable_area.width;
								if(temp_row >= 0)
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from right
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_column++; temp_index++;
					if(temp_column < playable_area.width){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_column -= 2; temp_column -= 2;
								if(temp_column >= 0)
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from left
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_column--; temp_index--;
					if(temp_column >= 0){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_column += 2; temp_column += 2;
								if(temp_column < playable_area.width)
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}

					// check if object is endangered from diagonal directions
					if((obj_pa_row + obj_pa_column) %2 != 0){
						if(it->objInfo->player){player_2_not_endangered_count++;}else{player_1_not_endangered_count++;}
						continue;
					}
					// check if object is endangerd from above and left
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_row--; temp_column--; temp_index -= playable_area.width + 1;
					if((temp_column >= 0)&&(temp_row >= 0)){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row += 2; temp_column += 2; temp_index += 2*(playable_area.width + 1);
								if((temp_column < playable_area.width)&&(temp_row < playable_area.height))
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from above and right
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_row--; temp_column++; temp_index -= playable_area.width - 1;
					if((temp_column < playable_area.width)&&(temp_row >= 0)){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row += 2; temp_column -= 2; temp_index += 2*(playable_area.width - 1);
								if((temp_column >= 0)&&(temp_row < playable_area.height))
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from below and left
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_row++; temp_column--; temp_index += playable_area.width - 1;
					if((temp_column >= 0)&&(temp_row < playable_area.height)){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row -= 2; temp_column += 2; temp_index -= 2*(playable_area.width - 1);
								if((temp_column < playable_area.width)&&(temp_row >= 0))
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}
					// check if object is endangerd from below and right
					temp_index = obj_pa_index; temp_row = obj_pa_row; temp_column = obj_pa_column;
					temp_row++; temp_column++; temp_index += playable_area.width + 1;
					if((temp_column < playable_area.width)&&(temp_row < playable_area.height)){
						if(gs->gameSquares.at(temp_index).gameObj != NULL)
							if(gs->gameSquares.at(temp_index).gameObj->objInfo != it->objInfo){
								temp_row -= 2; temp_column -= 2; temp_index -= 2*(playable_area.width + 1);
								if((temp_column < playable_area.width)&&(temp_row >= 0))
									if(gs->gameSquares.at(temp_index).gameObj == NULL) continue;			// object could be taken by opponent
							}
					}

					// ok, object is not endangered
					if(it->objInfo->player){player_2_not_endangered_count++;}else{player_1_not_endangered_count++;}
				}

				double ratio1, ratio2, ratio3;
				if(!according_to_player){
					// ratio1 deal with ratio of present objects count for each player
					ratio1 = (player_1_count >= player_2_count) ? (((double)player_1_count/(double)player_2_count - (double)1)/(double)player_1_count) : (((double)-1*player_2_count/(double)player_1_count + (double)1)/(double)player_2_count);
					// ratio2 deal with ratio of object, which are not safe against opponent attack, for each player
					ratio2 = (player_1_not_endangered_count >= player_2_not_endangered_count) ? (((double)player_1_not_endangered_count/(double)player_2_not_endangered_count - (double)1)/(double)player_1_not_endangered_count) : (((double)-1*player_2_not_endangered_count/(double)player_1_not_endangered_count + (double)1)/(double)player_2_not_endangered_count);
					// ratio3 deal with situations, when player try to 'play safe' and don't move forward
					ratio3 = (player_1_total_row_sum > 0) ? (((double)1 - ((double)player_1_total_row_sum/(double)((playable_area.height - 1)*player_1_count)))/(double)10) : 0;

					return ((ratio1 + ratio2 + ratio3) >= 0) ? std::min((ratio1 + ratio2 + ratio3), (double)0.995) : std::max((ratio1 + ratio2 + ratio3), (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state
				}
				if(according_to_player){
					// ratio1 deal with ratio of present objects count for each player
					ratio1 = (player_2_count >= player_1_count) ? (((double)player_2_count/(double)player_1_count - (double)1)/(double)player_2_count) : (((double)-1*player_1_count/(double)player_2_count + (double)1)/(double)player_1_count);
					// ratio2 deal with ratio of object, which are not safe against opponent attack, for each player
					ratio2 = (player_2_not_endangered_count >= player_1_not_endangered_count) ? (((double)player_2_not_endangered_count/(double)player_1_not_endangered_count - (double)1)/(double)player_2_not_endangered_count) : (((double)-1*player_1_not_endangered_count/(double)player_2_not_endangered_count + (double)1)/(double)player_1_not_endangered_count);
					// ratio3 deal with situations, when player try to 'play safe' and don't move forward
					ratio3 = (player_2_total_row_sum > 0) ?(((double)player_2_total_row_sum/(double)((playable_area.height - 1)*player_2_count))/(double)10) : 0;

					return ((ratio1 + ratio2 + ratio3) >= 0) ? std::min((ratio1 + ratio2 + ratio3), (double)0.995) : std::max((ratio1 + ratio2 + ratio3), (double)-0.995);	// 1 and -1 are reserved for final_state of game, 0.999 is if heuristic function knows it's winning state
				}
				break;
			}
		default : return 0;
	}
	return 0;
}

/* write predicates to file 'game'_stateofgame.lp */
bool Game::write_stateofgame_file(GameState* state){
	// stateofgame.lp  file    -    create or rewrite (game)_stateofgame.lp, with predicates representing state of game
	fstream file_stateofgame;
	
	string file_stateofgame_path = "..\\games\\" + game_name + "\\" + game_name + "_stateofgame.lp";
	
	// open file to write and discard content which was there
	file_stateofgame.open(file_stateofgame_path.c_str(), ios::out | ios::trunc);
	
	// if open method failed
	if(!file_stateofgame){
		// statusMsg.append("Game failed. Reason:\nFile \""); statusMsg.append(game_name); statusMsg.append("_moves.lp\" wasn't created...\n");
		cout << "Game failed. Reason:\nFile \"" << game_name << "_stateofgame.lp\" wasn't created...\n";
		return false;
	}

	// all predicates to write to output files
	stringstream ss;

	// 1st predicate) - player_turn(integer).
	ss << "player_turn(";
	(state->player_turn) ? ss << 2 : ss << 1;		// if player_must_move = false --> 1st player turn => 1 , if player_turn = true --> 2nd player turn => 2
	ss << ")." << endl;
	
	
	// 2nd predicate) - player_must_move(integer).
	if(state->player_must_move){
		ss << "player_must_move_this_turn." << endl;
	}
		 
	// 3rd predicate) - restricted_object_move_at_square(integer).
	if(state->restricted_object_move_at_square >= 0){			// -1 == no restriction for particular object to move => no predicate
		ss << "restricted_object_move_at_square(" << state->restricted_object_move_at_square << ")." << endl;
	}
	
	// 4th predicate) - only_add_new_objects_as_possible_move.
/*	if(state->only_add_new_objects_as_possible_move){			// if true => create predicate    ,    if false => no predicate
		ss << "only_add_new_objects_as_possible_move." << endl;
	}		
*/
	// 5th type of predicate) - game_square(integer,integer).
	for(unsigned int i = 0; i < state->gameSquares.size(); i++){
		// put one predicate "game_square(integer,integer)." in stringstream
		ss << "game_square(" << state->gameSquares.at(i).chessboardSq->index << ",";
		(state->gameSquares.at(i).gameObj == NULL) ? ss << 0 : ss << state->gameSquares.at(i).gameObj->objInfo->type; 
		ss << ")." << endl;
	}
	
	// 6th and 7th types of predicates) - possible_new_objects(integer,integer)	or possible_infinite_new_objects(integer)
	for(unsigned int i = 0; i < state->possible_new_objects.size(); i++){
		if(state->possible_new_objects.at(i) == 777){					// 777 means infinity
			ss << "possible_infinite_new_objects(" << i << ")." << endl;
		}else{
			ss << "possible_new_objects(" << i << "," << state->possible_new_objects.at(i) << ")." << endl;
		}
	}
			
	// write stringstream data to file
	int ss_length = ss.tellp();
	file_stateofgame.write(ss.str().c_str(),ss_length);

	// close file
	file_stateofgame.close();

	return true;
}

/* read predicates from file 'game'_moves.lp */
bool Game::read_moves_file(GameTreeNode* node){
	// possible_moves.lp  file	-	open file with informations about possible moves
	fstream file_moves;
	
	string file_moves_source = "..\\games\\" + game_name + "\\" + game_name + "_moves.lp";
	
	file_moves.open(file_moves_source.c_str(), ios::in);
	
	// if open method failed
	if(!file_moves){
		//statusMsg.append("Warning in Game class. Reason:\nFile \""); statusMsg.append(game_name); statusMsg.append("_moves.lp\" not found or didn't open...\n");
		cout << "Warning in Game class. Reason:\nFile \"" << game_name << "_moves.lp\" not found or didn't open...\n";
		return false;
	}

	// file open OK -----------
	// first: get length of file data
	file_moves.seekg(0,file_moves.end);
	unsigned int file_length = file_moves.tellg();
	file_moves.seekg(0,file_moves.beg);
	
	if(file_length == 0){
		return true;
	}

	// second: get file data to string
	string file_moves_data;
	file_moves_data.resize(file_length);
	file_moves.read(&file_moves_data[0], file_moves_data.size());


	unsigned int input_predicate_start = 0;			// first char after previous predicate_dot
	unsigned int input_predicate_end = -1;			// predicate dot is index of dot after predicate. Predicate syntax 'predicate_name'+optional[(,...,)]+'.'
	unsigned int input_find_comma = 0;
	unsigned int input_find_opening_parentheses = 0;
	unsigned int input_find_closing_parentheses = 0;
	bool no_parameter_predicate = false;

	stringstream ss;

	// mostly used to represent if syntax of the predicates are admissible
	bool good_input = true;

	// temporary list of side_effects, which after being handled, didn't have yet appropriate possible_move at that time. Stored in list and tried inserted to possible_move one more time after end of file.
	list<pair<string, GameMove_SideEffect>> temp_stored_side_effects = list<pair<string, GameMove_SideEffect>>();		// string is key value of "possible_move" at its map collection

	unsigned int max_iteration = 0;
	while(true){
		// just to be sure, break while(true) cycle even if syntax is corrupted
		if(++max_iteration > 1000){good_input = false; break;}

		// cond 1
		if((input_predicate_end == string::npos)&&(input_predicate_start > 0)) break;

		input_predicate_start = file_moves_data.find_first_not_of("{, \n", input_predicate_end + 1);

		// conditions when to end otherwise infinite while cycle -> break
		// cond 2 - this should be legit end of while cycle
		if((input_predicate_start == string::npos)||(input_predicate_start >= file_length)||(file_moves_data.at(input_predicate_start) == '}'))	break;
		
		// find boundaries and attributes for next predicate
		input_find_comma			   = file_moves_data.find_first_of(',', input_predicate_start);
		input_find_opening_parentheses = file_moves_data.find_first_of('(', input_predicate_start);

		string input_predicate, predicate_name;

		// if last predicate in file, without parameters
		if((input_find_comma == string::npos)&&(input_find_opening_parentheses == string::npos)){
			no_parameter_predicate = true;
			input_predicate_end = file_moves_data.find_last_of('}') - 1;
			input_predicate = file_moves_data.substr(input_predicate_start, input_predicate_end - input_predicate_start + 1);
			unsigned int input_predicate_temp_last_non_empty_char = input_predicate.find_last_not_of(" \n");
			if(input_predicate_temp_last_non_empty_char == string::npos) break; 
			predicate_name = input_predicate.substr(0,input_predicate_temp_last_non_empty_char + 1);
		}else{
			// if predicate with parameters
			if(input_find_opening_parentheses < input_find_comma){
				no_parameter_predicate = false;
				input_predicate_end = file_moves_data.find_first_of(')', input_predicate_start);
				if(input_predicate_end == string::npos) break;	// bad syntax
				input_predicate = file_moves_data.substr(input_predicate_start, input_predicate_end - input_predicate_start + 1);
				predicate_name = file_moves_data.substr(input_predicate_start, input_find_opening_parentheses - input_predicate_start);
			}else{
				// if predicate without parameters
				input_predicate_end = (input_find_comma - 1);
				no_parameter_predicate = true;
				input_predicate = file_moves_data.substr(input_predicate_start, input_predicate_end - input_predicate_start + 1);
				predicate_name = input_predicate;
			}
		}

		// match predicate_name against given predicates
		/*
		  1 - 3) predicates defining side effects of possible move	( a = add_new_object, m = move_object, r = remove_object )
				- pman - Possible_Move-Add_New_object - predicate defining pm - add new object - syntax: predicate_name(int, int, int, int, int, int, int, int)			8 int
				- pmmv - Possible_Move-MoVe_object	  - predicate defining pm - move object    - syntax: predicate_name(int, int, int, int, int, int, int, int, int)	9 int
				- pmrm - Possible_Move-ReMove_object  - predicate defining pm - remove object  - syntax: predicate_name(int, int, int, int, int, int)					6 int
		*/
		if ((!predicate_name.compare("pman"))||(!predicate_name.compare("pmmv"))||(!predicate_name.compare("pmrm"))){	// negation of compare method, because compare returns 0 if strings are equal, !0 => true
			// these predicates should have parameters
			if(no_parameter_predicate){
				good_input = false; break;
			}

			// find_out which predicate it is
			bool is_pman = !predicate_name.compare("pman"); bool is_pmmv = !predicate_name.compare("pmmv"); bool is_pmrm = !predicate_name.compare("pmrm");
			
			// find another vital parts of predicate
			unsigned int predicate_opening_parenthese = input_predicate.find_first_of('(');
			unsigned int predicate_closing_parenthese = input_predicate.find_last_of(')');
			unsigned int first_comma	= input_predicate.find_first_of(',',predicate_opening_parenthese + 1);
			unsigned int second_comma	= input_predicate.find_first_of(',',first_comma + 1);
			unsigned int third_comma	= input_predicate.find_first_of(',',second_comma + 1);
			unsigned int fourth_comma	= input_predicate.find_first_of(',',third_comma + 1);
			unsigned int fifth_comma	= input_predicate.find_first_of(',',fourth_comma + 1);
			unsigned int sixth_comma	= 0;
			unsigned int seventh_comma  = 0;
			unsigned int eighth_comma	= 0;

			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if((first_comma == string::npos)||(second_comma == string::npos)||(third_comma == string::npos)||
			   (fourth_comma == string::npos)||(fifth_comma == string::npos)||(sixth_comma == string::npos)){
				good_input = false;	break;
			}

			// find another vital parts of predicate
			if((is_pman)||(is_pmmv)){				
				sixth_comma = input_predicate.find_first_of(',',fifth_comma + 1);
				seventh_comma  = input_predicate.find_first_of(',',sixth_comma + 1);
				// test if there is problem with predicate syntax	-	if this is equal to string::npos means its not in "line" string
				if((sixth_comma == string::npos)||(seventh_comma == string::npos)){
					good_input = false;	break;
				}	
				if(is_pmmv){
					eighth_comma  = input_predicate.find_first_of(',',seventh_comma + 1);
					// test if there is problem with predicate syntax	-	if this is equal to string::npos means its not in "line" string
					if(eighth_comma == string::npos){
						good_input = false;	break;
					}
				}				
			}			
			
			// retrieve informations from predicate
			int parameter1, parameter2, parameter3, parameter4, parameter5, parameter6;
				
			// 1st parameter - index (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(predicate_opening_parenthese + 1, first_comma - predicate_opening_parenthese - 1));
			if(!(ss >> parameter1)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}		
			
			// 2nd parameter - index (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(first_comma + 1, second_comma - first_comma - 1));
			if(!(ss >> parameter2)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}
			
			// 3rd parameter - index (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(second_comma + 1, third_comma - second_comma - 1));
			if(!(ss >> parameter3)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 4th parameter - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(third_comma + 1, fourth_comma - third_comma - 1));
			if(!(ss >> parameter4)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}
			
			// 4th parameter check - if predicate "pmrm", parameter4 should be BOOL type of info, so integer {0,1}
			if((is_pmrm)&&(!((parameter4 == 0)||(parameter4 == 1)))){
				good_input = false; break;
			}

			// 5th parameter - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(fourth_comma + 1, fifth_comma - fourth_comma - 1));
			if(!(ss >> parameter5)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 5th parameter check - if predicate "pmrm" or "pman", parameter5 should be BOOL type of info, so integer {0,1}
			if((is_pmrm||is_pman)&&(!((parameter5 == 0)||(parameter5 == 1)))){
				good_input = false; break;
			}
			
			// 6th parameter - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter6 = (is_pmrm) ? predicate_closing_parenthese : sixth_comma;
			ss.str(input_predicate.substr(fifth_comma + 1, parameter_input_upper_bound_parameter6 - fifth_comma - 1));
			if(!(ss >> parameter6)){ 
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 6th parameter check - for all predicates, parameter6 should be BOOL type of info, so integer {0,1}
			if(!((parameter6 == 0)||(parameter6 == 1))){
				good_input = false; break;
			}
	
			/* possible_move-remove_object */
			if(is_pmrm){			
				// get pointers
				GameSquare* gs_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "pmrm" is possible_moves index_from of game_square
				ObjectInfo* oi_before = (gs_from == NULL) ? NULL : (gs_from->gameObj == NULL) ? NULL : gs_from->gameObj->objInfo;
				
				// check pointer, if NULL, skip this and continue to next predicate in file
				if(oi_before == NULL) continue;										// "gs_from" is checked in getting "oi_before"
				
				// everything is OK, create GamePossibleMove, it'll be value in unordered_map "possible_moves"
				GamePossibleMove gpm = GamePossibleMove(this);

				gpm.type_of_move = 3;												// type_of_move = 3 ( 3 = remove )
				gpm.gs_from = gs_from;
				gpm.obj_before_type = oi_before;
				gpm.reward_player_1 = parameter2;									// parameter2 in "pmrm" is possible_moves reward for player 1
				gpm.reward_player_2 = parameter3;									// parameter3 in "pmrm" is possible_moves reward for player 2
				gpm.player_continues_at_next_turn   = (parameter4) ? true : false;	// parameter4 in "pmrm" is possible_moves player_continues_at_next_turn
				gpm.player_must_move_at_next_turn   = (parameter5) ? true : false;	// parameter5 in "pmrm" is possible_moves player_must_move_at_next_turn
				gpm.add_to_new_objects_after_remove = (parameter6) ? true : false;	// parameter6 in "pmrm" is possible_moves add_to_new_objects_after_remove ( ! this is different to "pman" and "pmmv" predicates )

				// create key to GamePossibleMove in unordered_map "possible_moves"
				string str_hash = gpm.string_hash();
								
				// insert pair<key,value>, result.first returns iterator to inserted value, result.second is bool if value was inserted
				pair<unordered_map<string,GamePossibleMove>::iterator, bool> result_of_insert = node->possible_moves.emplace(make_pair(str_hash,gpm));

				// if GamePossibleMove wasn't inserted (e.g. already exists value with its hash key), return false
				if(!result_of_insert.second){
					cout << endl << "Possible problem in function Game::readmoves(), check if all possible moves are present.";
				}						
				
				/* possible_move-remove_object added, continue next iteration of while cycle*/
				continue;
			}

			// retrieve next information from predicate
			int parameter7, parameter8;

			// 7th parameter - type (integer) - only for predicates "pman" and "pmmv"
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter7 = (is_pman) ? predicate_closing_parenthese : seventh_comma;
			ss.str(input_predicate.substr(sixth_comma + 1, parameter_input_upper_bound_parameter7 - sixth_comma - 1));
			if(!(ss >> parameter7)){ 
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 7th parameter check - for predicates "pman" and "pmmv", parameter7 should be BOOL type of info, so integer {0,1}
			if(!((parameter7 == 0)||(parameter7 == 1))){
				good_input = false; break;
			}

			// 8th parameter - type (integer) - only for predicates "pman" and "pmmv"
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter8 = (is_pman) ? predicate_closing_parenthese : eighth_comma;
			ss.str(input_predicate.substr(seventh_comma + 1, parameter_input_upper_bound_parameter8 - seventh_comma - 1));
			if(!(ss >> parameter8)){ 
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 8th parameter check - for predicates "pman" and "pmmv", parameter8 should be BOOL type of info, so integer {0,1}
			if(!((parameter8 == 0)||(parameter8 == 1))){
				good_input = false; break;
			}

			/* possible_move-add_new_object */
			if(is_pman){			
				// get pointers
				GameSquare* gs_to    = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "pman" is possible_moves index_from of game_square
				ObjectInfo* oi_after = get_objectInfo_at_index(parameter2);					// parameter2 in "pman" is possible_moves index_after of new object type

				// check pointers, if NULL, skip this and continue to next predicate in file
				if((gs_to == NULL)||(oi_after == NULL)) continue;							// "gs_from" is checked in getting "oi_before"
				
				// everything is OK, create GamePossibleMove, it'll be value in unordered_map "possible_moves"
				GamePossibleMove gpm = GamePossibleMove(this);

				gpm.type_of_move = 1;														// type_of_move = 1 ( 1 = add new )
				gpm.gs_to = gs_to;
				gpm.obj_after_type = oi_after;
				gpm.reward_player_1 = parameter3;											// parameter3 in "pman" is possible_moves reward for player 1
				gpm.reward_player_2 = parameter4;											// parameter4 in "pman" is possible_moves reward for player 2
				gpm.player_continues_at_next_turn = (parameter5) ? true : false;			// parameter5 in "pman" is possible_moves player_continues_at_next_turn
				gpm.player_must_move_at_next_turn = (parameter6) ? true : false;			// parameter6 in "pman" is possible_moves player_must_move_at_next_turn
				gpm.restrict_move_only_to_this_object = (parameter7) ? true : false;		// parameter7 in "pman" is possible_moves restrict_move_only_to_this_object
				gpm.restricted_object_can_make_pass_move = (parameter8) ? true : false;		// parameter8 in "pman" is possible_moves restrict_move_only_to_this_object

				// create key to GamePossibleMove in unordered_map "possible_moves"
				string str_hash = gpm.string_hash();
				
				// insert pair<key,value>, result.first returs iterator to inserted value, result.second is bool if value was inserted
				pair<unordered_map<string,GamePossibleMove>::iterator, bool> result_of_insert = node->possible_moves.emplace(make_pair(str_hash,gpm));

				// if GamePossibleMove wasn't inserted (e.g. already exists value with its hash key), return false
				if(!result_of_insert.second){
					cout << endl << "Possible problem in function Game::readmoves(), check if all possible moves are present.";
				}						

				/* possible_move-add_new_object added, continue next iteration of while cycle*/
				continue;
			}

			// retrieve next information from predicate
			int parameter9;

			// 9th parameter - type (integer) - only for predicate "pmmv"
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(eighth_comma + 1, predicate_closing_parenthese - eighth_comma - 1));
			if(!(ss >> parameter9)){ 
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 9th parameter check - for predicate "pmmv", parameter9 should be BOOL type of info, so integer {0,1}
			if(!((parameter9 == 0)||(parameter9 == 1))){
				good_input = false; break;
			}

			/* possible_move-move_object */
			if(is_pmmv){			
				// get pointers
				GameSquare* gs_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "pmmv" is possible_moves index_from of game_square
				ObjectInfo* oi_before = (gs_from == NULL) ? NULL : (gs_from->gameObj == NULL) ? NULL : gs_from->gameObj->objInfo;				
				GameSquare* gs_to     = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "pmmv" is possible_moves index_to of game_square
				ObjectInfo* oi_after  = get_objectInfo_at_index(parameter3);				// parameter3 in "pmmv" is possible_moves index_after of objects type

				// check pointers, if NULL, skip this and continue to next predicate in file
				if((gs_to == NULL)||(oi_before == NULL)||(oi_after == NULL)) continue;		// "gs_from" is checked in getting "oi_before"
				
				// everything is OK, create GamePossibleMove, it'll be value in unordered_map "possible_moves"
				GamePossibleMove gpm = GamePossibleMove(this);

				gpm.type_of_move = 2;														// type_of_move = 2 ( 2 = move existing object )
				gpm.gs_from = gs_from;
				gpm.gs_to = gs_to;
				gpm.obj_before_type = oi_before;
				gpm.obj_after_type = oi_after;
				gpm.reward_player_1 = parameter4;											// parameter4 in "pman" is possible_moves reward for player 1
				gpm.reward_player_2 = parameter5;											// parameter5 in "pman" is possible_moves reward for player 2
				gpm.player_continues_at_next_turn = (parameter6) ? true : false;			// parameter6 in "pmmv" is possible_moves player_continues_at_next_turn
				gpm.player_must_move_at_next_turn = (parameter7) ? true : false;			// parameter7 in "pmmv" is possible_moves player_must_move_at_next_turn
				gpm.restrict_move_only_to_this_object = (parameter8) ? true : false;		// parameter8 in "pmmv" is possible_moves restrict_move_only_to_this_object
				gpm.restricted_object_can_make_pass_move = (parameter9) ? true : false;		// parameter9 in "pmmv" is possible_moves restrict_move_only_to_this_object

				// create key to GamePossibleMove in unordered_map "possible_moves"
				string str_hash = gpm.string_hash();

				// insert pair<key,value>, result.first returs iterator to inserted value, result.second is bool if value was inserted
				pair<unordered_map<string,GamePossibleMove>::iterator, bool> result_of_insert = node->possible_moves.emplace(make_pair(str_hash,gpm));

				// if GamePossibleMove wasn't inserted (e.g. already exists value with its hash key), return false
				if(!result_of_insert.second){
					cout << endl << "Possible problem in function Game::readmoves(), check if all possible moves are present.";
				}						

				/* possible_move-add_new_object added, continue next iteration of while cycle*/
				continue;
			}
		} // end of predicates for possible_moves

		/*
		  4 - 12) predicates defining side effects of possible move	( a = add_new_object, m = move_object, r = remove_object )
					- AseA - Add_new_object-as_Side_Effect_of-Add_new_object	- syntax: predicate_name(int, int, int, int)			4 int
					- MseA - Move_object-as_Side_Effect_of-Add_new_object		- syntax: predicate_name(int, int, int, int, int)		5 int
					- RseA - Remove_object-as_Side_Effect_of-Add_new_object		- syntax: predicate_name(int, int, int, int)			4 int
					- AseM - Add_new_object-as_Side_Effect_of-Move_object		- syntax: predicate_name(int, int, int, int, int)		5 int
					- MseM - Move_object-as_Side_Effect_of-Move_object			- syntax: predicate_name(int, int, int, int, int, int)	6 int
					- RseM - Remove_object-as_Side_Effect_of-Move_object		- syntax: predicate_name(int, int, int, int, int)		5 int
					- AseR - Add_new_object-as_Side_Effect_of-Remove_object		- syntax: predicate_name(int, int, int)					3 int
					- MseR - Move_object-as_Side_Effect_of-Remove_object		- syntax: predicate_name(int, int, int, int)			4 int	
					- RseR - Remove_object-as_Side_Effect_of-Remove_object		- syntax: predicate_name(int, int, int,)				3 int
		*/
		if ((!predicate_name.compare("asea"))||(!predicate_name.compare("msea"))||(!predicate_name.compare("rsea"))||
			(!predicate_name.compare("asem"))||(!predicate_name.compare("msem"))||(!predicate_name.compare("rsem"))||
			(!predicate_name.compare("aser"))||(!predicate_name.compare("mser"))||(!predicate_name.compare("rser")))
		{
			// these predicates should have parameters
			if(no_parameter_predicate){
				good_input = false;	break;
			}
			
			// find_out which predicate it is ( string::compare returns 0 if strings are equal, > 0 or < 0 if not equal)
			bool is_asea = !predicate_name.compare("asea"); bool is_msea = !predicate_name.compare("msea"); bool is_rsea = !predicate_name.compare("rsea");
			bool is_asem = !predicate_name.compare("asem"); bool is_msem = !predicate_name.compare("msem"); bool is_rsem = !predicate_name.compare("rsem");
			bool is_aser = !predicate_name.compare("aser"); bool is_mser = !predicate_name.compare("mser"); bool is_rser = !predicate_name.compare("rser");

			// find another vital parts of predicate
			unsigned int predicate_opening_parenthese = input_predicate.find_first_of('(');
			unsigned int predicate_closing_parenthese = input_predicate.find_last_of(')');
			unsigned int first_comma  = input_predicate.find_first_of(',',predicate_opening_parenthese + 1);
			unsigned int second_comma = input_predicate.find_first_of(',',first_comma + 1);
			unsigned int third_comma  = 0;
			unsigned int fourth_comma = 0; 
			unsigned int fifth_comma  = 0;

			// test if there is problem with predicate syntax	-	if any of these is equal to string::npos means its not in "line" string
			if((first_comma == string::npos)||(second_comma == string::npos)){
				good_input = false;	break;
			}

			// find another vital parts of predicate
			if((is_asea)||(is_msea)||(is_rsea)||(is_asem)||(is_msem)||(is_rsem)||(is_mser)){				
				third_comma = input_predicate.find_first_of(',',second_comma + 1);
				// test if there is problem with predicate syntax	-	if this is equal to string::npos means its not in "line" string
				if(third_comma == string::npos){
					good_input = false;	break;
				}
				if((is_msea)||(is_asem)||(is_msem)||(is_rsem)){
					fourth_comma = input_predicate.find_first_of(',',third_comma + 1);
					// test if there is problem with predicate syntax	-	if this is equal to string::npos means its not in "line" string
					if(fourth_comma == string::npos){
						good_input = false;	break;
					}
					if(is_msem){
						fifth_comma  = input_predicate.find_first_of(',',fourth_comma + 1);
						// test if there is problem with predicate syntax	-	if this is equal to string::npos means its not in "line" string
						if(fifth_comma == string::npos){
							good_input = false;	break;
						}
					}	
				}				
			}			
			
			// retrieve informations from predicate		- ASER and RSER are predicates with 4 parameters
			int parameter1, parameter2, parameter3;

			// 1st parameter - index (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(predicate_opening_parenthese + 1, first_comma - predicate_opening_parenthese - 1));
			if(!(ss >> parameter1)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}		

			// 2nd parameter - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(first_comma + 1, second_comma - first_comma - 1));
			if(!(ss >> parameter2)){		
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			// 3nd parameter - type (integer)
			ss.clear();											// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter3 = ((is_aser)||(is_rser)) ? predicate_closing_parenthese : third_comma;
			ss.str(input_predicate.substr(second_comma + 1, parameter_input_upper_bound_parameter3 - second_comma - 1));
			if(!(ss >> parameter3)){ 
				good_input = false;	break;						// if there is problem with conversion, end while cycle			
			}

			/* add_new_object as side effect of remove_object */
			if(is_aser){
				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "aser" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_se_to     = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "aser" is side_effects index_to of game_square
				ObjectInfo* oi_se_after  = get_objectInfo_at_index(parameter3);					// parameter3 in "aser" is side_effects objects object_type after side_effect	

				// check pointers
				if((oi_pm_before == NULL)||(gs_se_to == NULL)) continue;	// gs_pm_from is already checked in process of getting "oi_pm_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 3 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type;	// possible_move is 'remove' = 3

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 1;										// type_of_move = 1 ( 1 = add new )
				gm_se.gs_to = gs_se_to;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			/* remove_object as side effect of remove_object */
			if(is_rser){
				// parameter3 check syntax, parameter3 in "rser" should be BOOL type of info, so integer {0,1}
				if(!((parameter3 == 0)||(parameter3 == 1))){
					good_input = false; break;
				}

				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "rser" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "rser" is side_effects index_from of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;

				// check pointers
				if((oi_pm_before == NULL)||(oi_se_before == NULL)) continue;	// "gs_pm_from" and "gs_se_from" are already checked in process of getting "oi_pm_from" and "oi_se_from"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 3 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type;	// possible_move is 'remove' = 3

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 3;											// type_of_move = 3 ( 3 = remove )
				gm_se.gs_from = gs_se_from;
				gm_se.obj_before_type = oi_se_before;
				gm_se.add_to_new_objects_after_remove = (parameter3 == 1);		// parameter3 is checked and contains only values 0 and 1, which are representation of bool variable

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			// retrieve informations from predicate		- ASEA, RSEA and MSER are predicates with 4 parameters
			int parameter4;

			// 4th parameter - type (integer)
			ss.clear();										// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter4 = ((is_asea)||(is_rsea)||(is_mser)) ? predicate_closing_parenthese : fourth_comma;
			ss.str(input_predicate.substr(third_comma + 1, parameter_input_upper_bound_parameter4 - third_comma - 1));
			if(!(ss >> parameter4)){ 
				good_input = false;	break;					// if there is problem with conversion, end while cycle			
			}

			/* add_new_object as side effect of add_new_object */
			if(is_asea){
				// get pointers
				GameSquare* gs_pm_to    = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "asea" is possible_moves index_to of game_square 
				ObjectInfo* oi_pm_after = get_objectInfo_at_index(parameter2);					// parameter2 in "asea" is possible_moves objects object_type after side_effect
				GameSquare* gs_se_to    = node->get_game_square_with_chsq_index(parameter3);	// parameter3 in "asea" is side_effects index_to of game_square
				ObjectInfo* oi_se_after = get_objectInfo_at_index(parameter4);					// parameter4 in "asea" is side_effects objects object_type after side_effect
				
				// check pointers
				if((gs_pm_to == NULL)||(oi_pm_after == NULL)||(gs_se_to == NULL)||(oi_se_after == NULL)) continue;
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 1 << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'add new' = 1

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 1;										// type_of_move = 1 ( 1 = add new )
				gm_se.gs_to = gs_se_to;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			/* remove_object as side effect of add_new_object */
			if(is_rsea){
				// parameter4 check syntax, parameter4 in "rsea" should be BOOL type of info, so integer {0,1}
				if(!((parameter4 == 0)||(parameter4 == 1))){
					good_input = false; break;
				}
				
				// get pointers
				GameSquare* gs_pm_to     = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "rsea" is possible_moves index_to of game_square 
				ObjectInfo* oi_pm_after  = get_objectInfo_at_index(parameter2);					// parameter2 in "rsea" is possible_moves objects object_type after side_effect
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter3);	// parameter3 in "rsea" is side_effects index_to of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;

				// check pointers
				if((gs_pm_to == NULL)||(oi_pm_after == NULL)||(oi_se_before == NULL)) continue;			// "gs_se_from"is already checked in process of getting "oi_se_from"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 1 << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'add new' = 1

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 3;										// type_of_move = 3 ( 3 = remove )
				gm_se.gs_from = gs_se_from;
				gm_se.obj_before_type = oi_se_before;
				gm_se.add_to_new_objects_after_remove = (parameter4 == 1);	// parameter4 is checked and contains only values 0 and 1, which are representation of bool variable

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			/* move_object as side effect of add_new_object */
			if(is_mser){
				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "mser" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "mser" is side_effects index_from of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;
				GameSquare* gs_se_to     = node->get_game_square_with_chsq_index(parameter3);	// parameter3 in "mser" is side_effects index_to of game_square
				ObjectInfo* oi_se_after  = get_objectInfo_at_index(parameter4);					// parameter4 in "mser" is side_effects objects object_type after side_effect

				// check pointers
				if((oi_pm_before == NULL)||(oi_se_before == NULL)||(gs_se_to == NULL)||(oi_se_after == NULL)) continue;	// "gs_pm_from" and "gs_se_from" are already checked in process of getting "oi_pm_before" and "oi_se_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 3 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type;	// possible_move is 'remove' = 3

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 2;										// type_of_move = 2 ( 2 = move )
				gm_se.gs_from = gs_se_from;
				gm_se.gs_to = gs_se_to;
				gm_se.obj_before_type = oi_se_before;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			// retrieve informations from predicate		-  MSEA, ASEM and RSEM are predicates with 5 parameters
			int parameter5;
			
			// 5th parameter - type (integer)
			ss.clear();										// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			unsigned int parameter_input_upper_bound_parameter5 = ((is_msea)||(is_asem)||(is_rsem)) ? predicate_closing_parenthese : fifth_comma;
			ss.str(input_predicate.substr(fourth_comma + 1, parameter_input_upper_bound_parameter4 - fourth_comma - 1));
			if(!(ss >> parameter5)){ 
				good_input = false;	break;					// if there is problem with conversion, end while cycle			
			}

			/* move_object as side effect of add_new_object */
			if(is_msea){
				// get pointers
				GameSquare* gs_pm_to     = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "msea" is possible_moves index_to of game_square 
				ObjectInfo* oi_pm_after  = get_objectInfo_at_index(parameter2);					// parameter2 in "msea" is possible_moves objects object_type after side_effect		
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter3);	// parameter3 in "msea" is side_effects index_from of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;
				GameSquare* gs_se_to     = node->get_game_square_with_chsq_index(parameter4);	// parameter4 in "msea" is side_effects index_to of game_square
				ObjectInfo* oi_se_after  = get_objectInfo_at_index(parameter5);					// parameter5 in "msea" is side_effects objects object_type after side_effect
				
				// check pointers
				if((gs_pm_to == NULL)||(oi_pm_after == NULL)||(oi_se_before == NULL)||(gs_se_to == NULL)||(oi_se_after == NULL)) continue;	// "gs_se_from" is already checked in process of getting "oi_se_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 1 << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'add new' = 1

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 2;										// type_of_move = 2 ( 2 = move )
				gm_se.gs_from = gs_se_from;
				gm_se.gs_to = gs_se_to;
				gm_se.obj_before_type = oi_se_before;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			/* add_new_object as side effect of move_object */
			if(is_asem){
				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "asem" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_pm_to     = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "asem" is possible_moves index_to of game_square
				ObjectInfo* oi_pm_after  = get_objectInfo_at_index(parameter3);					// parameter3 in "asem" is possible_moves objects object_type after possible_moves				
				GameSquare* gs_se_to     = node->get_game_square_with_chsq_index(parameter4);	// parameter4 in "asem" is side_effects index_to of game_square
				ObjectInfo* oi_se_after  = get_objectInfo_at_index(parameter5);					// parameter5 in "asem" is side_effects objects object_type after side_effect

				// check pointers
				if((oi_pm_before == NULL)||(gs_pm_to == NULL)||(oi_pm_after == NULL)||(gs_se_to == NULL)||(oi_se_after == NULL)) continue;	// "gs_pm_from" is already checked in process of getting "oi_pm_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 2 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'move' = 2

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 1;										// type_of_move = 1 ( 1 = add new )
				gm_se.gs_to = gs_se_to;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			/* add_new_object as side effect of move_object */
			if(is_rsem){
				// parameter5 check syntax, parameter5 in "rsem" should be BOOL type of info, so integer {0,1}
				if(!((parameter5 == 0)||(parameter5 == 1))){
					good_input = false; break;
				}

				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "rsem" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_pm_to     = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "rsem" is possible_moves index_to of game_square
				ObjectInfo* oi_pm_after  = get_objectInfo_at_index(parameter3);					// parameter3 in "rsem" is possible_moves objects object_type after possible_moves				
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter4);	// parameter4 in "rsem" is side_effects index_to of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;

				// check pointers
				if((oi_pm_before == NULL)||(gs_pm_to == NULL)||(oi_pm_after == NULL)||(oi_se_before == NULL)) continue;	// "gs_pm_from" and "gs_se_from" are already checked in process of getting "oi_pm_before" and "oi_se_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 2 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'move' = 2

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 3;										// type_of_move = 3 ( 3 = add new )
				gm_se.gs_from = gs_se_from;
				gm_se.obj_before_type = oi_se_before;
				gm_se.add_to_new_objects_after_remove = (parameter5 == 1);	// parameter5 is checked and contains only values 0 and 1, which are representation of bool variable

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}

			// retrieve informations from predicate		-  MSEM is predicate with 6 parameters
			int parameter6;
			
			// 6th parameter - type (integer)
			ss.clear();										// previous strinstream>> set oef error flag, there have to be clear() in order to reset stringstream content for next use
			ss.str(input_predicate.substr(fifth_comma + 1, predicate_closing_parenthese - fifth_comma - 1));
			if(!(ss >> parameter6)){ 
				good_input = false;	break;					// if there is problem with conversion, end while cycle			
			}

			/* move_object as side effect of move_object */
			if(is_msem){
				// get pointers
				GameSquare* gs_pm_from   = node->get_game_square_with_chsq_index(parameter1);	// parameter1 in "msem" is possible_moves index_from of game_square
				ObjectInfo* oi_pm_before = (gs_pm_from == NULL) ? NULL : (gs_pm_from->gameObj == NULL) ? NULL : gs_pm_from->gameObj->objInfo;
				GameSquare* gs_pm_to     = node->get_game_square_with_chsq_index(parameter2);	// parameter2 in "msem" is possible_moves index_to of game_square
				ObjectInfo* oi_pm_after  = get_objectInfo_at_index(parameter3);					// parameter3 in "msem" is possible_moves objects object_type after possible_moves				
				GameSquare* gs_se_from   = node->get_game_square_with_chsq_index(parameter4);	// parameter4 in "msem" is side_effects index_from of game_square
				ObjectInfo* oi_se_before = (gs_se_from == NULL) ? NULL : (gs_se_from->gameObj == NULL) ? NULL : gs_se_from->gameObj->objInfo;
				GameSquare* gs_se_to     = node->get_game_square_with_chsq_index(parameter5);	// parameter5 in "msem" is side_effects index_to of game_square
				ObjectInfo* oi_se_after  = get_objectInfo_at_index(parameter6);					// parameter6 in "msem" is side_effects objects object_type after side_effect

				// check pointers
				if((oi_pm_before == NULL)||(gs_pm_to == NULL)||(oi_pm_after == NULL)||(oi_se_before == NULL)||(gs_se_to == NULL)||(oi_se_after == NULL)) continue;	// "gs_pm_from" and "gs_se_from" are already checked in process of getting "oi_pm_before" and "oi_se_before"
				
				// create key to GamePossibleMove in unordered_map "possible_moves"
				stringstream str_hash_pm;
				str_hash_pm << 2 << "-" << gs_pm_from->chessboardSq->index << "-" << oi_pm_before->type << "-" << gs_pm_to->chessboardSq->index << "-" << oi_pm_after->type;	// possible_move is 'move' = 2

				// create GameMove_SideEffect
				GameMove_SideEffect gm_se = GameMove_SideEffect(this);
				gm_se.type_of_move = 2;										// type_of_move = 2 ( 2 = move )
				gm_se.gs_from = gs_se_from;
				gm_se.gs_to = gs_se_to;
				gm_se.obj_before_type = oi_se_before;
				gm_se.obj_after_type = oi_se_after;

				// try find 'possible_move' and push_back side_effect or store it in temporary collection 
				GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value(str_hash_pm.str());
				if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
					pm_of_se->side_effects.push_back(gm_se);
				}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
					temp_stored_side_effects.push_back(make_pair(str_hash_pm.str(),gm_se));
				}

				// continue to next predicate in file
				continue;
			}
		} // end of side effets predicates read
	} // end of while cycle, end of file input

	// close file
	file_moves.close();
	
	// check if while cycle end abruptly
	if(!good_input){
		cout << endl << "Read file " << game_name << "_moves.lp ends abruptly, probably bad syntax." << endl;
		return false;
	}

	// temporary list of side_effects, which after being handled, didn't have yet appropriate possible_move at that time. Stored in list and tried inserted to possible_move one more time after end of file.
	for(list<pair<string, GameMove_SideEffect>>::iterator it = temp_stored_side_effects.begin(); it != temp_stored_side_effects.end(); it++){
		// try again find 'possible_move' and push_back side_effect or return false
		GamePossibleMove* pm_of_se = node->get_possible_move_by_key_value((*it).first);
		if(pm_of_se != NULL){	// 'possible_move' whom this 'side_effect' belongs to
			pm_of_se->side_effects.push_back((*it).second);
		}else{					// this kind of 'possible_move' is not in collection "possible_moves" in GameTreeNode
			return false;
		}
	}

	// everything went OK
	return true;
}

/* remove files 'game'_stateofgame.lp and 'game'_results.lp */
void Game::remove_state_files(){
	// delete file with last gamestate.lp and moves.lp
	string file_name = "..\\games\\" + game_name + "\\" + game_name + "_stateofgame.lp";
	if(remove(file_name.c_str())){
//		cout << "File \"" << game_name << "_stateofgame.lp\" wasn't deleted." << endl;
	}
	
	file_name = "..\\games\\" + game_name + "\\" + game_name + "_moves.lp";
	if(remove(file_name.c_str())){
//		cout << "File \"" << game_name << "_moves.lp\" wasn't deleted." << endl;
	}
}

/* ------------------------------------------------------------------- < play the game > ------------------------------------------------------------------- */

/* make one move from "actual_node", return false if something went wrong */
bool Game::make_one_move_in_game_by_AI(Player* player, bool console_output){
	// check pointers
	if(player == NULL) return false;																// at least 1st player must be present
	if(player->human_or_AI) return false;															// this is function for AI
	if(actual_node == NULL) return false;

	// who is on turn at actual node
	if(get_who_is_on_turn() != player->which_player_in_game) return false;							// check if player is turn
		
	// check if actual_node is not already final
	if(actual_node->isGameStateFinal()) return true;												// if already set final

	// check if node is already developed
	if(!(actual_node->isNodeDeveloped)){																				
		if(!(develope_game_tree_node(actual_node))) return false;									// if not, develope node, node is also checked if final in Game::develope_game_tree_node()
	}
	
	// check if actual_node is not already final now
	if(actual_node->isGameStateFinal()) return true;												// if already set final
	// if(check_if_game_state_is_final(actual_node->get_game_state())) return true;					// check if it's not final now
	
	// text output of possible moves from actual 'game_state'
	if(console_output){cout << endl << endl; cout << create_readable_info_about_possible_moves_from_actual_game_node();}

	// resulting value is pair < expected value for chosen move (e.g. in minimax from heuristic function) , iterator to node created by chosen move >
	pair<double,list<pair<string,GameTreeNode>>::iterator> result = make_pair((double)-3, actual_node->child_nodes.end());	// -3 is enough out of range

	// check if only one possible move is present, if yes, choose this move without further calculations (can be done, because it has been already verified that this node is developed and not final, and this cannot be changed by chosing move)
	//											   if no, proceed with chosen AI algorithm
	if(actual_node->child_nodes.size() == 1){
		result.second = actual_node->child_nodes.begin();	
	}else{
		// choose AI approach
		switch(player->ai_approach){
			/* random move */
			case 0 : result.second = move_random_choice();	break;
			/* minimax */
			case 1 :
				{ 
					if(player->minimax_depth < 1) return false;														// depth at least 1
					result = minimax(actual_node, player->minimax_depth, player);									// results from heuristic are <-1,1>
					break;
				}
			case 2 :
				{ 
					if(player->minimax_depth < 1) return false;														// depth at least 1
					result = minimax_alpha_beta(actual_node, player->minimax_depth,(double)-2,(double)2, player);	// results from heuristic are <-1,1>, so -2 and 2 are like -inf, +inf
					break;
				}
			
			default : return false;
		};
	}

	// check result
	if(actual_node->isGameStateFinal()) return true;					// if node became final, due to process in node->develope..., needs to be checked before " if(result.second == actual_node->child_nodes.end()) return false; "
	if(result.second == actual_node->child_nodes.end()) return false;
	
	// store hash information about resulting 'possible_move' for further use
	string result_key = result.second->first;
		
	// delete all child 'game_tree_nodes' but the one pushed by parameter, delete all 'possible_moves' but the one that created given child 'game_tree_node'
	if(!process_chosen_move_for_actual_node(result.second)) return false;

	// create readable information about this move
	unordered_map<string,GamePossibleMove>::iterator it_pm2 = actual_node->possible_moves.find(result_key);
	if(it_pm2 != actual_node->possible_moves.end()){
		if(console_output){
			cout << create_readable_info_about_chosen_move(player->ai_approach, actual_node->get_game_state(), &(it_pm2->second), result.first, player->minimax_depth);
		}		
	}else{ return false; }
	
	// set appropriate attribute and change "actual_node" to chosen 'node'
	actual_node->isMoveChosen = true;
	actual_node = &(actual_node->child_nodes.front().second);

	// create readable information about new actual 'game_state'
	if(console_output){cout << endl << endl << " results into -->" << endl << endl << create_readable_info_about_actual_node_game_state();}

	return true;
}

/* make one move from "actual_node" by human player, return false if something went wrong */
bool Game::make_one_move_in_game_by_human(Player* player){
	// check pointers
	if(player == NULL) return false;
	if(!(player->human_or_AI)) return false;											// this is function for Human
	if(actual_node == NULL) return false;

	// who is on turn at actual node
	if(get_who_is_on_turn() != player->which_player_in_game) return false;				// if given player is on turn

	// check if actual_node is not already final
	if(actual_node->isGameStateFinal()) return true;									// if already set final
	
	// check if node is already developed
	if(!(actual_node->isNodeDeveloped)){																				
		if(!(develope_game_tree_node(actual_node))) return false;						// if not, develope node, node is also checked if final in Game::develope_game_tree_node()
	}
	
	// check if actual_node is not already final now
	if(actual_node->isGameStateFinal()) return true;									// if already set final
	// if(check_if_game_state_is_final(actual_node->get_game_state())) return true;		// check if it's not final now
	
	// text output of possible moves from actual 'game_state'	
	cout << endl << endl; cout << create_readable_info_about_possible_moves_from_actual_game_node();
	cout << endl << endl << " Please choose one of the possible move listed above.\n Write index of move ( first one have index 0, not 1) :  ";

	unsigned int index_of_chosen_move = actual_node->possible_moves.size();
	while(true){
		cin >> index_of_chosen_move;
		
		if((index_of_chosen_move < 0)||(index_of_chosen_move >= actual_node->possible_moves.size())){
			cout << endl << "Index must be between 0 and " << (actual_node->possible_moves.size() - 1) << endl;
		}else{ break; }
	}

	// find given move by index in collection
	list<pair<string,GameTreeNode>>::iterator it = actual_node->child_nodes.begin();
	for(unsigned int i = 0; i < index_of_chosen_move; i++){
		it++;
		if(it == actual_node->child_nodes.end()) return false;							// check boundaries
	}
	
	// store hash information about resulting 'possible_move' for further use
	string result_key = it->first;
		
	// delete all child 'game_tree_nodes' but the one pushed by parameter, delete all 'possible_moves' but the one that created given child 'game_tree_node'
	if(!process_chosen_move_for_actual_node(it)) return false;

	// create readable information about this move
	unordered_map<string,GamePossibleMove>::iterator it_pm2 = actual_node->possible_moves.find(result_key);
	if(it_pm2 != actual_node->possible_moves.end()){
		cout << endl << " Chosen move: \"" << result_key;
	}else{ return false; }
	
	if(it_pm2->second.side_effects.size() > 0){
		cout << "\" with these side_effects:";
		for(list<GameMove_SideEffect>::iterator it = it_pm2->second.side_effects.begin(); it != it_pm2->second.side_effects.end(); it++){
			cout << endl << "   - " << it->human_readable_info();
		}
	}else{cout << "\" - with no side_effects.";}

	// set appropriate attribute and change "actual_node" to chosen 'node'
	actual_node->isMoveChosen = true;
	actual_node = &(actual_node->child_nodes.front().second);

	// create readable information about new actual 'game_state'
	cout << endl << endl << " results into -->" << endl << endl << create_readable_info_about_actual_node_game_state();

	return true;
}

/* delete all child 'game_tree_nodes' but the one pushed by parameter, delete all 'possible_moves' but the one that created given child 'game_tree_node' */
bool Game::process_chosen_move_for_actual_node(list<pair<string,GameTreeNode>>::iterator it){
	// check pointers
	if(actual_node == NULL) return false;							
	if(it == actual_node->child_nodes.end()) return false;

	string it_result_key = it->first;

	// remove all but resulting child node from collection
	if(actual_node->child_nodes.size() > 1){
		if(it != actual_node->child_nodes.begin()){													// if not first
			it = actual_node->child_nodes.erase(actual_node->child_nodes.begin(), it);				// remove everything before that element
		}

		if(it != actual_node->child_nodes.end()) it++;

		if(it != actual_node->child_nodes.end()){													// if not last
			actual_node->child_nodes.erase(it, actual_node->child_nodes.end());						// remove everything behind that element
		}
	}
	// there should be only one 'chosen' 'child game_tree_node'
	if(actual_node->child_nodes.size() != 1) return false;
	
	// remove all but chosen possible move from collection
	unordered_map<string,GamePossibleMove>::iterator it_pm = actual_node->possible_moves.find(it_result_key);
	if(it_pm == actual_node->possible_moves.end()) return false;

	if(actual_node->possible_moves.size() > 1){
		if(it_pm != actual_node->possible_moves.begin()){											// if not first
			it_pm = actual_node->possible_moves.erase(actual_node->possible_moves.begin(), it_pm);	// remove everything before that element
		}

		if(it_pm != actual_node->possible_moves.end()) it_pm++;

		if(it_pm != actual_node->possible_moves.end()){											// if not last
			actual_node->possible_moves.erase(it_pm, actual_node->possible_moves.end());		// remove everything behind that element
		}
	}
	// there should be only one 'chosen' 'possible_move'
	if(actual_node->possible_moves.size() != 1) return false;

	// OK
	return true;
}

/* checks 'game' if 'game_state' in "actual_node" is 'final state', if yes, parameter "result" is 0 = draw, 1 = 1st player is winner, 2 = 2nd player is winner */
bool Game::check_if_actual_node_is_set_as_final(int& result){
	if(actual_node == NULL) return false;
	if(actual_node->isGameStateFinal()){
		switch(actual_node->get_game_state()->game_result){		// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing
			case 0 : result = 0; return true;
			case 1 : result = (actual_node->get_game_state()->player_turn) ? 2 : 1; return true;
			case 2 : result = (actual_node->get_game_state()->player_turn) ? 1 : 2; return true;
			default : return false;
		}
	}
	return false;
}

/* creates string output for text visualization of actual 'game_state' */
string Game::create_readable_info_about_actual_node_game_state(){
	if(actual_node == NULL) return string("");

	stringstream ss_output_1, ss_output_2;
	GameObject* obj;
	int type;

	ss_output_1 << " ----- <actual node> ------------- <map of indexes> ----- ";
	if(playable_area.width == -1) ss_output_1 << endl;
	for(unsigned int i = 0; i < actual_node->get_game_state()->gameSquares.size(); i++){
		if(playable_area.width != -1){
			if(i % playable_area.width == 0){
				ss_output_1 << "\t\t" << ss_output_2.str() << endl;
				ss_output_2.str(""); ss_output_2.clear(); ss_output_2.flush();
			}
		}

		ss_output_1.width(2);
		ss_output_1.fill(' ');
		ss_output_2.width(3);
		ss_output_2.fill(' ');

		obj = actual_node->get_game_state()->gameSquares.at(i).gameObj;
		type = (obj == NULL) ? 0 : actual_node->get_game_state()->gameSquares.at(i).gameObj->objInfo->type;
		ss_output_1 << type; 

		ss_output_2 << actual_node->get_game_state()->gameSquares.at(i).chessboardSq->index;
	}

	if(playable_area.width == -1){ ss_output_1 << endl; }else{ ss_output_1 << "\t\t"; }
	ss_output_1 << ss_output_2.str();	

	ss_output_1 << endl << " ----- </actual node> -----------------------------------";

	return ss_output_1.str();
}

string Game::create_readable_info_about_possible_moves_from_actual_game_node(){
	if(actual_node == NULL) return string("");
	
	stringstream ss_output;
	
	if(actual_node->possible_moves.size() > 0){
		cout << " Possible moves to chose from:";
		
		unsigned int i = 0;
		for(unordered_map<string,GamePossibleMove>::iterator it_2 = actual_node->possible_moves.begin(); it_2 != actual_node->possible_moves.end(); it_2++){
			// previous version cout << endl << "   - " << i << ")    " << it_2->first;
			cout << endl << "  - " << i << ")  " << it_2->second.human_readable_info();
			i++;
		}
	}else{cout << " No possible moves to chose from in actual game state.";}
	
	return ss_output.str();
}

/* creates string output for text visualization of chose move by some AI algorithm or another approach - last move which made 'actual_nove' */
string Game::create_readable_info_about_chosen_move(unsigned int ai_approach, GameState* gs, GamePossibleMove* gpm, double value, int depth){
	if((gs == NULL)||(gpm == NULL)) return string("");
	
	stringstream ss_output;
	ss_output << endl << endl << " ------------------------------------------------------------------------------- ";
	
	switch(ai_approach){
		case 0 : ss_output << endl << " -- Random move"; break;
		case 1 : ss_output << endl << " -- Minimax algorithm with depth limitation (at level " << depth << ")"; break;
		case 2 : ss_output << endl << " -- Minimax algorithm with alpha beta prunning and depth limitation (at level " << depth << ")"; break;
		default : ss_output << endl << " -- Unknown approach"; break;
	};
		
	ss_output << endl << " Gamestate:   ";
	if(gs->player_turn){ss_output << "2nd player turn";}else{ss_output << "1st player turn";}
	if((gs->player_must_move)||player_must_move_each_turn){ss_output << "   (player had to move this turn)";}; 
	
	if(gs->restricted_object_move_at_square != -1){
		ss_output << endl << "  - move restricted only to object on chessboard square: " << gs->restricted_object_move_at_square;
	}else{ss_output << endl << "  - no restrictions for particular objects";}

	ss_output << endl << endl << " Chosen move: \"" << gpm->string_hash() << "\""; 
	if((value>=(-2))&&(value<=2)) ss_output << " with expected value: '" << value << "'";
	
	if(gpm->side_effects.size() > 0){
		ss_output << " with these side_effects:";
		for(list<GameMove_SideEffect>::iterator it = gpm->side_effects.begin(); it != gpm->side_effects.end(); it++){
			ss_output << endl << "  - " << it->human_readable_info();
		}
	}else{ss_output << " with no side_effects.";}

	return ss_output.str();
}

/* ---------------------------------------------------------- < play the game - random approach > ---------------------------------------------------------- */

/* choose one random move, try once again if 'pass move' was chosen at first */
list<pair<string,GameTreeNode>>::iterator Game::move_random_choice(){
	// check pointer
	if(actual_node == NULL) return list<pair<string,GameTreeNode>>().end();

	// check if node is already developed, if not, develope it
	if(!(actual_node->isNodeDeveloped)){																	
		if(!(develope_game_tree_node(actual_node))) return list<pair<string,GameTreeNode>>().end();
	}

	// simple answers
	if(actual_node->isGameStateFinal()) return actual_node->child_nodes.end();
	if(actual_node->child_nodes.empty()) return actual_node->child_nodes.end();
	if(actual_node->child_nodes.size() == 1) return actual_node->child_nodes.begin();

	// random answer
	list<pair<string,GameTreeNode>>::iterator it = actual_node->child_nodes.begin();

	int rand_position = 0;
	srand(time(NULL));

	rand_position = rand() % actual_node->child_nodes.size();
	for(int i = 0; i < rand_position; i++){
		it++;
		if(it == actual_node->child_nodes.end()) return it;		// check boundaries
	}

	if(it->first.compare("0") == 0){							// if 'pass move' was selected first time, try one more time
		it = actual_node->child_nodes.begin();

		rand_position = rand() % actual_node->child_nodes.size();
		for(int i = 0; i < rand_position; i++){
			it++;
			if(it == actual_node->child_nodes.end()) return it;	// check boundaries
		}
	}

	return it;
}	

/* ---------------------------------------------------------- < play the game - minimax section > ---------------------------------------------------------- */

/* result of function "minimax" is reward of minimax and iterator to chosen child game tree node */
pair<double,list<pair<string,GameTreeNode>>::iterator> Game::minimax(GameTreeNode* node, int depth, Player* maximizingPlayer){
	/* set statistical data */
	maximizingPlayer->minimax_tree_visited_nodes++;

	/* minimax tree leafs */
	if(depth == 0){											// if depth == 0
		return make_pair(evaluate_heuristic_value(node->get_game_state(), maximizingPlayer->which_player_in_game, maximizingPlayer->type_of_heuristic),node->child_nodes.end());				// if depth == 0  => leaf
	}
	
	if(!(node->isNodeDeveloped)){							// if node is already developed, or even chosen move if there were no possible moves
		if(!(develope_game_tree_node(node)))				// if not, develope node
			return make_pair(-3,node->child_nodes.end());
	}
	
	if(node->isGameStateFinal()){							// if already set as final
		double temp = 0;
		switch(node->get_game_state()->game_result){
			case 0 : temp = 0; break;						// 0 means draw at 'game_state'
			case 1 : temp = (node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game) ? (1 + (double)depth*(double)0.005) : (-1 - (double)depth*(double)0.005); break;	// 1 means player on turn at 'game_state' is winner, higher depth (acts as lower depth value) means lower bonus ( 
			case 2 : temp = (node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game) ? (-1 - (double)depth*(double)0.005) : (1 + (double)depth*(double)0.005); break;	// 2 means player not on turn at 'game_state' is winner, higher depth (acts as lower depth value) means lower bonus ( 
			default : temp = 0; break;
		};
		return make_pair(temp,node->child_nodes.end());
	}

	/* minimax recursion */
	list<pair<string,GameTreeNode>>::iterator it_result = node->child_nodes.end();
	
	if(node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game){
		/* maximizing player node in minimax tree */
		double actual_value = (double)-2;					// values <-1,1>, -2 is therefore -inf
		
		for(list<pair<string,GameTreeNode>>::iterator it = node->child_nodes.begin(); it != node->child_nodes.end(); it++){
			/* recursion */
			pair<double,list<pair<string,GameTreeNode>>::iterator> minmax_result = minimax(&(it->second), depth-1, maximizingPlayer);

			/* choosing move with maximizing profit */
			if(minmax_result.first > actual_value){
				it_result = it;
				actual_value = minmax_result.first;
			}
		}
												
		/* return value and iterator to maximizing 'child game_tree_node' */
		return make_pair(actual_value, it_result);				
	}else{
		/* minimizing player node in minimax tree */
		double actual_value = (double)2;					// values <-1,1>, 2 is therefore +inf

		for(list<pair<string,GameTreeNode>>::iterator it = node->child_nodes.begin(); it != node->child_nodes.end(); it++){
			/* recursion */
			pair<double,list<pair<string,GameTreeNode>>::iterator> minmax_result = minimax(&(it->second), depth-1, maximizingPlayer);

			/* choosing move with minimizing profit */
			if(minmax_result.first < actual_value){
				it_result = it;
				actual_value = minmax_result.first;
			}				
		}

		/* return beta and iterator to minimizing 'child game_tree_node' */
		return make_pair(actual_value, it_result);					
	}
}

/* result of function "minimax_alpha_beta" is reward of minimax_alpha_beta and iterator to chosen child game tree node */
pair<double,list<pair<string,GameTreeNode>>::iterator> Game::minimax_alpha_beta(GameTreeNode* node, int depth, double alpha, double beta, Player* maximizingPlayer){
	/* set statistical data */
	maximizingPlayer->minimax_tree_visited_nodes++;
	
	/* minimax tree leafs */
	if(depth == 0){											// if depth == 0
		return make_pair(evaluate_heuristic_value(node->get_game_state(), maximizingPlayer->which_player_in_game, maximizingPlayer->type_of_heuristic),node->child_nodes.end());				// if depth == 0  => leaf
	}
	
	if(!(node->isNodeDeveloped)){							// if node is already developed, or even chosen move if there were no possible moves
		if(!(develope_game_tree_node(node)))				// if not, develope node
			return make_pair(-3, node->child_nodes.end());
	}
	
	if(node->isGameStateFinal()){							// if already set as final
		double temp = 0;
		switch(node->get_game_state()->game_result){
			case 0 : temp = 0; break;						// 0 means draw at 'game_state'
			case 1 : temp = (node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game) ? (1 + (double)depth*(double)0.005) : (-1 - (double)depth*(double)0.005); break;	// 1 means player on turn at 'game_state' is winner, higher depth (acts as lower depth value) means lower bonus ( 
			case 2 : temp = (node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game) ? (-1 - (double)depth*(double)0.005) : (1 + (double)depth*(double)0.005); break;	// 2 means player not on turn at 'game_state' is winner, higher depth (acts as lower depth value) means lower bonus ( 
			default : temp = 0; break;
		};
		return make_pair(temp,node->child_nodes.end());
	}

	/* minimax recursion */
	list<pair<string,GameTreeNode>>::iterator it_result = node->child_nodes.end();
	pair<double,list<pair<string,GameTreeNode>>::iterator> minmax_result_for_pass_move;
	bool is_pass_move_present = false;

	if(node->get_game_state()->player_turn == maximizingPlayer->which_player_in_game){
		/* maximizing player node in minimax tree */
		for(list<pair<string,GameTreeNode>>::iterator it = node->child_nodes.begin(); it != node->child_nodes.end(); it++){
			/* recursion */
			pair<double,list<pair<string,GameTreeNode>>::iterator> minmax_result = minimax_alpha_beta(&(it->second), depth-1, alpha, beta, maximizingPlayer);

			/* dealing with 'pass move' if present, in order to use it as last resort, after all others move considered (bcs alpha_beta prunning takes first 'best' solution as result */
			if(it->first.compare("0") == 0){
				is_pass_move_present = true;
				minmax_result_for_pass_move = make_pair(minmax_result.first, it);	// little hack, this is not what parameter is intended for, but I will use it so there is no need to create new variable to store this information
				continue;
			}	
			
			/* choosing move with maximizing profit */
			if(minmax_result.first > alpha){
				it_result = it;
				alpha = minmax_result.first;
//				if(abs(alpha - 1) < 0.0000001) break;				// if alpha is 1, no need to look further, that's maximum to achieve - edit: changed in order to gain bonus from depth of this final solution, bonus is given above value 1
			}

			/* "beta cut off" */
			if(beta <= alpha) break;
		}

		/* dealing with 'pass move' if present, "afterpart" */
		if(is_pass_move_present){
			if(minmax_result_for_pass_move.first > alpha){
				alpha = minmax_result_for_pass_move.first;
				it_result = minmax_result_for_pass_move.second;		// little hack used above was for this one time use only
			}
		}
												
		/* return alpha and iterator to maximizing 'child game_tree_node' */
		return make_pair(alpha, it_result);				
	}else{
		/* minimizing player node in minimax tree */
		for(list<pair<string,GameTreeNode>>::iterator it = node->child_nodes.begin(); it != node->child_nodes.end(); it++){
			/* recursion */
			pair<double,list<pair<string,GameTreeNode>>::iterator> minmax_result = minimax_alpha_beta(&(it->second), depth-1, alpha, beta, maximizingPlayer);

			/* dealing with 'pass move' if present, in order to use it as last resort, after all others move considered (bcs alpha_beta prunning takes first 'best' solution as result */
			if(it->first.compare("0") == 0){
				is_pass_move_present = true;
				minmax_result_for_pass_move = make_pair(minmax_result.first, it);	// little hack, this is not what parameter pair.second (iterator) is intended for, but I will use it so there is no need to create new variable to store this information
				continue;
			}

			/* choosing move with minimizing profit */
			if(minmax_result.first < beta){
				it_result = it;
				beta = minmax_result.first;
//				if(abs(beta + 1) < 0.0000001) break;				// if beta is -1, no need to look further, that's maximum to achieve - edit: changed in order to gain bonus from depth of this final solution, bonus is given above value -1
			}

			/* "alpha cut-off" */
			if(beta <= alpha) break;					
		}

		/* dealing with 'pass move' if present, "afterpart" */
		if(is_pass_move_present){
			if(minmax_result_for_pass_move.first < beta){
				beta = minmax_result_for_pass_move.first;
				it_result = minmax_result_for_pass_move.second;		// little hack used above was for this one time use only
			}
		}

		/* return beta and iterator to minimizing 'child game_tree_node' */
		return make_pair(beta, it_result);				
	}
}

/* ------------------------------------------------------------------- < get functions > ------------------------------------------------------------------- */

const vector<ObjectInfo>* Game::get_objectsTypes(){
	return &objectsTypes;
}

ObjectInfo* Game::get_objectInfo_at_index(int index){
	return ((index >= 0)&&((unsigned int)index < objectsTypes.size())) ? &objectsTypes.at(index) : NULL ;
}

const bool Game::get_number_of_players(){
	return number_of_players;
}

GameTree* Game::get_game_tree(){
	return game_tree;
}

bool Game::get_who_is_on_turn(){
	return (actual_node == NULL) ? false : actual_node->get_game_state()->player_turn;
}

string Game::get_game_name(){
	return game_name;
}
#endif