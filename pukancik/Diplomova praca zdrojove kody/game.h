#ifndef _PP_GAME_H
#define _PP_GAME_H

#ifndef _PP_CHESSBOARD_H
#include "chessboard.h"
#endif

#ifndef _PP_ENVIRONMENT_H
class Environment;				// forward declaration	
#endif

using namespace std;
using namespace cv;

//--------------------------------------------------------------------------------------------------------------

class Game;						// forward declaration
class GameSquare;				// forward declaration

//--------------------------------------------------------------------------------------------------------------

// Informations about object - this informations are declared in gameinfo.lp file for each particular game
class ObjectInfo{
  public:
	int type;					// index of objects Type	// (e.g. in chess - tower, queen, ...) type (of object) values for particular int values will be assaigned in parent Game class object
	bool player;				// false (0) = 1st player   ,   true (1) = 2nd player
	
	string name;				// name of the particular object - optional - just to be easier identified

	// info for camera and image processing
	int colour;					// colour (of object) values for particular int values will be assaigned in parent Game class object
	int shape;					// (e.q. circle, square,...) shapes (of object) values for particular int values will be assaigned in parent Game class object
};

//--------------------------------------------------------------------------------------------------------------

class GameObject{
  public:
	GameSquare* gameSq;			// pointer to GameSquare where's the object located at the time, which is stored in collection in Game class
	ObjectInfo* objInfo;		// pointer to ObjectInfo which is stored in collection in Game class

	GameObject();
	GameObject(GameSquare* gs, ObjectInfo* oi);
	/* maybe in some next versions
		bool active;				// if the object is already in use in game
		bool started OR destroyed;	
	*/
};

//--------------------------------------------------------------------------------------------------------------

class GameSquare{
  public:
	ChessboardSquare* chessboardSq;			// pointer to chessboard square which represent space on the chessboard ( particular game doesn't have to use all chessboard squares )
	GameObject* gameObj;					// pointer to object which is located on particular GameSquare at the time
	
	GameSquare();
	GameSquare(ChessboardSquare* chSq, GameObject* gO);
};

//--------------------------------------------------------------------------------------------------------------

/* this should be information about move, when simple instruction for RoboticArm is needed */
class GameSimpleFinalMove{
  public:
	Game* parent_game;							// pointer to containing class Game

	GameSquare* gs_from;						// pointer to game square, where is object located before move, if NULL then it means it's new object and it should be located on Chessboard::new_object_position_RW_coords point
	GameSquare* gs_to;							// pointer to game square, where should be object located after move, if NULL then it means it's object meant to be removed from chessboard and it should be moved to point Chessboard::remove_object_position_RW_coords

	ObjectInfo* obj_after_type;					// type of object to add if after_type != NULL, this should be only if move means add_new_object
	
	GameSimpleFinalMove();
	GameSimpleFinalMove(Game* game);
	GameSimpleFinalMove(Game* game, GameSquare* gs_f, GameSquare* gs_t, ObjectInfo* obj_t);
};

class GameMove{
  public:
	int type_of_move;							// what type of move it is -1 = non defined, 0 = no move -> pass to another player, 1 = add new object, 2 = move existing object, 3 = remove existing object
												// 0 will change "player_turn" value after turn
	
	GameSquare* gs_from;						// pointer to game square, where is object located before move, if NULL then it means it's new object and it should be located on Chessboard::new_object_position_RW_coords point
	GameSquare* gs_to;							// pointer to game square, where should be object located after move, if NULL then it means it's object meant to be removed from chessboard and it should be moved to point Chessboard::remove_object_position_RW_coords

	ObjectInfo* obj_before_type;				// if type will change during transition, there will be some internal actions, e.q if object is to move, first it will be moved, than removed and added new object with appropriate type 
	ObjectInfo* obj_after_type;					// 
	
	Game* parent_game;							// pointer to containing class Game

	// info score1 a score2
	int reward_player_1, reward_player_2;		// reward for players gained by this move

	// info 4 - applicable to type  3 ( remove object )	- rest of this type of infos are in GamePossibleMove
	bool add_to_new_objects_after_remove;		// what to do with object after remove, true -> add (or sum up) it to new_objects, false -> nope

	// constructor
	GameMove();
	GameMove(Game* game);

	string string_hash();						// creates 'string hash' of given 'game_move'
	/*virtual*/ string human_readable_info();
};

class GamePossibleMove;		// forward declaration

class GameMove_SideEffect : public GameMove {
  public:
	// constructor
	GameMove_SideEffect();
	GameMove_SideEffect(Game* game);
	GameMove_SideEffect(GamePossibleMove* gpm);
};

class GamePossibleMove : public GameMove {
  public:
	list<GameMove_SideEffect> side_effects;			// collection of all side effect, which are result to this possible move, are not sorted, order as it was read in moves.lp file											

	list<GameSimpleFinalMove> pm_move_for_roboArm;	// collection of all side effects + possible_move, will be sorted by chronological order
													// when performed in real world by roboarm, move_object with change of types must be internally defined by move_obj -> remove_obj -> add_new_obj, bcs these are side effects of side effect :)

	/* attributes in resulting game_state */
	// info 1
	bool player_continues_at_next_turn;				// sets if player on turn continues in resulting state
	// info 2
	bool player_must_move_at_next_turn;				// sets if player on turn at resulting state must move (so no pass_move available)
	// info 3 - applicable to types 1 ( add new object) and 2 ( move object )
	bool restrict_move_only_to_this_object;			// sets if this object will have to be only one to move in resulting state
	// info 4 - applicable to type  3 ( remove object )
// transefered to GameMove	bool add_to_new_objects_after_remove;		// what to do with object after remove, true -> add (or sum up) it to new_objects, false -> nope
	// info 5 - applicable to types 1 ( add new object) and 2 ( move object )
	bool restricted_object_can_make_pass_move;		// (if restriction to this object is set in info3) sets if can be broken 'player_must_move' in resulting 'game_state', in that way that if restricted object can't move then create 'pass_move'
	
	//	another infos not yet needed ... bool only_add_new_objects_as_possible_move;

	// constructor
	GamePossibleMove();
	GamePossibleMove(Game* game);
};

//--------------------------------------------------------------------------------------------------------------

class GameState{
  public:
	Game* parent_game;								// pointer to containing class Game

	list<GameObject> gameObjects;					// collection of objects in game at particular state
	vector<GameSquare> gameSquares;					// collection of playable squares of chessboard in game
	vector<int> possible_new_objects;				// collection of objects, which can be added to game as new objects ( index in collection is type of object, value is how many can be added during play, 999 means infinite )

	bool player_turn;								// false (0) = 1st player turn    ,    true (1) = 2nd player turn
//	bool only_add_new_objects_as_possible_move;		// false/true sets if only add some new object can be performed as possible move (e.q as start in some type of game), it means choose only add new object from collection "possible_moves"

	// there are cobinations of these next properties with different results
	bool player_must_move;							// false = player on turn must move ,  true = player can pass turn if he wants to 
	int restricted_object_move_at_square;			// -1 not set = no restriction for particular object to move , 0 - n only particular object on square with that index can move,
	bool restricted_object_can_create_pass_move;	// if restricted_object_move_at_square is set and also player_must_move is set true, then if this attribute is set true, player can create 'pass_move' if there is no 'possible move' by that object ( that means by none of the objects, because there was restriction only to it )
	
	bool isFinalState;								// sets if state is final, one posibility from { win , draw , lose }
	int game_result;								// 0 = draw, 1 = player currently on turn is winner, 2 = player currently not on turn is winner, -1 = nothing

	int player1_score, player2_score;				// score for each player

	/* constructors */
	GameState();
	GameState(Game* game);
	// GameState(GameState& gs);

	void copy_collections_of_gs_for_child_gs(GameState& gs);			// copy collections "gameObjects", "gameSquares" and "possible_new_objects" as new vectors, for child 'game_square', so it needs to change pointers also

	void change_possible_new_objects(unsigned int index, int change);

	bool remove_object_at_game_square(GameSquare* gs);					// remove "GameObject" from collection "gameObjects" which is on given 'game_square'

	bool check_and_set_if_game_state_is_final();						// check if 'game_state' is final, set attributes "isFinalState" and "game_result"

	/* get function */
	GameSquare* get_game_square_at_index(unsigned int index);
	GameSquare* get_game_squares_with_chsq_index(unsigned int index);	// return pointer to GameSquare in local collection "gameSquare", which have 'chessboard_square' index given as parameter, otherwise NULL
	bool get_possible_new_objects_value_at_index(unsigned int index, int& value);	
};

//--------------------------------------------------------------------------------------------------------------

class GameTreeNode{
  public:

	Game* parent_game;										// pointer to containing Game class
	GameTreeNode* parent_node;								// pointer to parent game_tree, NULL for game_tree root

	/* possible_moves and child_nodes are interconnected, possible_move at index ID in collection "possible_moves" results in game_state defining child_node at index ID (same index) in collection "game_states" */
	GameState game_state;									// actual game_state in game_tree_node
	unordered_map<string,GamePossibleMove> possible_moves;	// unordered_map of possible_moves from game_state in game_tree_node
	list<pair<string,GameTreeNode>> child_nodes;			// list of game_tree_nodes resulting from possible_moves

	int depth;												// depth of game_tree_node in game_tree, 0 for tree_root
	
	string result_of_move;									// identificator of move, which created given game_tree_node

	bool isNodeDeveloped;									// node is developed, if there are developed possible_moves in collection "possible_moves" and correspondend game_tree_nodes in collection "game_tree_nodes" for these moves
	bool isMoveChosen;										// define if move from tree_node was already chosen
	
	bool previous_move_was_pass_move;						// true if previous move was just 'pass move', means only change player on turn ( interesting only previous player use this 'move' and current player wants to use it also, what results to something set in ... )
	
	double reward;											// reward <-1,1> is either result from heuristic or ... doplnit :)

	/* constructor */
	GameTreeNode();
	GameTreeNode(Game* game, GameTreeNode* parent, int node_depth, string move_from_parent);

	/* init root game tree node */
	bool init_tree_root_node();

	/* get functions */
	GameState* get_game_state();
	GameSquare* get_game_square_at_index(int index);
	GameSquare* get_game_square_with_chsq_index(int index);

	GamePossibleMove* get_possible_move_by_key_value(string& key_value);

	bool isGameStateFinal();								// gets if game_state is final, means no possible_moves or just its state is in final configuration
};

//--------------------------------------------------------------------------------------------------------------

class GameTree{
	Game* parent_game;
	GameTreeNode root;

public:	
	/* constructor */
	GameTree();
	GameTree(Game* game);

	/* init */
	bool init();				// init class GameTree means init root GameTreeNode -> only if not before

	/* get function */
	GameTreeNode* get_root();
	const Game* get_game();
};

//--------------------------------------------------------------------------------------------------------------

class Player{
  public:
	Game* game;						// parent game
	bool human_or_AI;				// if Player is AI (computer) = false, if Player is human = true
	bool which_player_in_game;		// if false = Player is 1st player in "game", if true = Player is 2nd player in "game"
	int ai_approach;				// if Player is AI (computer), sets what kind of approach to use, 0 = random, 1 = minimax, 2 = minimax_alfa_beta_prunning, ...
	int type_of_heuristic;			// what kind of heuristic will Player use					( if "computer_approach" is 1 or 2 (minimax or minimax_alpha_beta_prunning) )
	int minimax_depth;				// how many moves will Player look forward in minimax tree	( if "computer_approach" is 1 or 2 (minimax or minimax_alpha_beta_prunning) )
	int minimax_tree_visited_nodes; // how many game_tree_nodes were visited in minimax tree	( if "computer_approach" is 1 or 2 (minimax or minimax_alpha_beta_prunning) )
	
	/* constructor */
	Player();
	Player(Game* g, bool hORai, bool wpin, int ai_approach, int heu, int md);	// default values will be "ai_approach" = 0 (random), "heu" = 0 (every state have heuristic value equal to zero), "md" = 0, 
};

//--------------------------------------------------------------------------------------------------------------

/* just structure used in Game class */
class Playable_Area{
  public:
	int width;
	int height;
	unsigned int left_top_corner;		// "left_corner" is index of 'chessboard' square

	Playable_Area();
	Playable_Area(int w, int h, unsigned int ltc);
};

//--------------------------------------------------------------------------------------------------------------

class Game{
	/* pointer to containing class */
	Environment* env;
	
	/* info about game */
	string game_name;					// name and path to txt file with informations about of game
	bool number_of_players;				// false (0) = one player game    ,    true (1) = two player game
	bool player_must_move_each_turn;	// set if player must move each turn, no matter about value of GameState::player_must_move
	int what_no_possible_moves_means;	// each value means another unique approach to evaluate 'game state' with no possible moves, default value (and therefore strategy) is 0 ( no moves => lose )
	int how_to_check_final_state;		// each value means another unique approach to evaluate if 'game state' is final, default value (and therefore strategy) is 0 ( 'game_state' is final if there are no possible moves, so this function does nothing, in this case, 'game state' is set by function "process_no_possible_moves" )

	/* game space and objects */
	Playable_Area playable_area;		// playable area is rectangle portion of chessboard, which is area of 'game_squares' on which is played particular game
	vector<ObjectInfo> objectsTypes;	// collection of informations about particular type of object in the game
	
	/* game states structure */
	GameTree* game_tree;				// game_tree is structure, which describes 1 iteration of Game, root_node game_state is initialized each time from Game attributes
	GameTreeNode* actual_node;			// pointer to game_tree_node, which is actual position of game progress

	/* data defining inital state of game, or after restart -> new game */
	list<pair<ChessboardSquare*, ObjectInfo*>> init_game_squares;	// collection describing game squares and types of object belonging to them, at initial (e.g. new game) state of game
	vector<int> init_possible_new_objects;							// collection defining possible new objects, index in collections defining type of object, value is number of possible new objects of that type, -1 means infinity, and size must be equal to collection "objectsTypes"
	bool init_player_turn;											// false (0) = 1st player turn  ,   true (1) = 2nd player turn
	bool init_player_must_move;										// false = player which starts doesn't have to move ,  true = player which starts doesn't have to move
//	bool init_only_add_new_objects_as_possible_move;				// false any object can move  ,  true = only new objects can be added as possible move
	int init_restricted_object_move_at_square;						// if there is restriction on which particular object must move at initial state, -1 = nope    ,    0 - n = particular object at that defined square index
	
	/* bool constraints */
	bool game_initialized;											// don't do anything with game if this is not true

	/* another informations about game */
	/* collection of pairs (integer_value, colour(resp. shape) value) for particular game WHERE integer value is index of colour value in collection - first object in collection is 0... etc etc */
	vector<Scalar> obj_colours;										// colours in openCV - Scalar(a,b,c) where Red = c, Green = b, Blue = a as RGB - so Scalar(b,g,r)
	vector<int> obj_shapes;											// e.g. 0 -> circle, 1 -> square, ... -- Treba domysliet, napr. zadane v init file nejako ... hmmmmmmmmm   -------   -------   -------   -------   -------   -------   -------   -------

  public:
	/* constructor */
	Game();

	/* destructor */
	~Game();

	/* initialization */
	bool init(Environment* e, string gameName, string& statusMsg);							// read informations about game from txt LP ASP files, set class atributes, if is all OK then set atribute -> game_initialized = true
	bool restart();																			// restarts game
	
	/* read gameinfo and init files */
	bool read_gameinfo_file(string& statusMsg);
	bool read_init_file(string& statusMsg);

	/* create initial game_state */
	void create_initial_state(GameState* state);											// sets GameState, given as parameter, to initial state from Game init attributes
	
	/* find possible moves, process and develope child nodes */
	bool develope_game_tree_node(GameTreeNode* node);										// develope game_tree_node given by parameter. That means, run dlv solver, find and get possible_move, ... 
	bool process_no_possible_moves(GameTreeNode* node);										// process game_tree_node which has no possible_moves
	bool process_possible_move_and_its_resulting_child_tree_node(GamePossibleMove* gpm,	GameTreeNode* node); 
	bool check_if_game_state_is_final(GameState* gs);										// takes 'game_state' as parameter, return bool value if it is final according to attribute "how_to_check_final_state"
	double evaluate_heuristic_value(GameState* gs, bool according_to_player, int type_of_heuristics);	// heuristic aproximation of players "according_to_player" expecting profit from 'game_state', according to attribute "type_of_heuristics" (default strategy is 0 (this default value will return 0, median of <-1,1>))

	bool write_stateofgame_file(GameState* state);											// create or rewrite (game)_stateofgame.lp, with predicates representing state of game
	bool read_moves_file(GameTreeNode* node);												// read file which contains resulting predicates defining possible moves, moves side effects, ...
	void remove_state_files();																// remove (delete) files, which are different for each state

	/* play the game */
	bool make_one_move_in_game_by_AI(Player* player, bool console_output);					// make one move from "actual_node" by AI (computer), return false if something went wrong
	bool make_one_move_in_game_by_human(Player* player);									// make one move from "actual_node" by human player, return false if something went wrong
	bool process_chosen_move_for_actual_node(list<pair<string,GameTreeNode>>::iterator it);	// delete all child 'game_tree_nodes' but the one pushed by parameter, delete all 'possible_moves' but the one that created given child 'game_tree_node'

	bool check_if_actual_node_is_set_as_final(int& result);									// checks 'game' if 'game_state' in "actual_node" is 'final state', if yes, parameter "result" is 0 = draw, 1 = 1st player is winner, 2 = 2nd player is winner
	string create_readable_info_about_actual_node_game_state();
	string create_readable_info_about_possible_moves_from_actual_game_node();	
	string create_readable_info_about_chosen_move(unsigned int ai_approach, GameState* gs, GamePossibleMove* gpm, double value, int depth);		// create readable informations about chosen possible move, ai_approach is the same as for Player info, value is reward for minimax approach, if < -2 or > 2, it is not in output, depth as also only for minimax approach

	/* play the game - random approach */
	list<pair<string,GameTreeNode>>::iterator move_random_choice();							// make one move from "actual_node", return false if something went wrong

	/* play the game - minimax section */
	pair<double,list<pair<string,GameTreeNode>>::iterator> minimax(GameTreeNode* node, int depth, Player* maximizingPlayer);										// result of function "minimax" is reward of minimax and iterator to chosen child game tree node
	pair<double,list<pair<string,GameTreeNode>>::iterator> minimax_alpha_beta(GameTreeNode* node, int depth, double alpha, double beta, Player* maximizingPlayer);	// result of function "minimax_alpha_beta" is reward of minimax_alpha_beta and iterator to chosen child game tree node
	
	/* get functions */
	const vector<ObjectInfo>* get_objectsTypes();
	ObjectInfo* get_objectInfo_at_index(int index);
	const bool get_number_of_players();
	GameTree* get_game_tree();
	bool get_who_is_on_turn();
	string get_game_name();
};
#endif