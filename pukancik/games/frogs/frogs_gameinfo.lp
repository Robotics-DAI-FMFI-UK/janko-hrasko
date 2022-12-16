#maxint = 65.

% number of players - 1 or 2
number_of_players(1).

% predicate "what_no_possible_moves_means", if not present, default value is 0 
what_no_possible_moves_means(0).    % 0 = LOSE, 1 = DRAW, 2 = WIN

% predicate "player_must_move_every_turn", if present = TRUE, if not = FALSE              
player_must_move_every_turn.

% predicate "how_to_check_final_state", if not present, default value is 0 
how_to_check_final_state(1).

% if playable_area(width,height,left_corner) is present, if game area is rectangle, left_corner of playable_area have parameter of index of chessboard square 
playable_area(7,1,0).

% Objects informations
type_of_objects(0,0,"empty",0,0).   
type_of_objects(1,1,"frog_to_right",1,0).
type_of_objects(2,1,"frog_to_left",2,0).