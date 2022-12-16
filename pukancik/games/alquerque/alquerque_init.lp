% --  which player is on turn ( 1 - first player , 2 - 2nd player )
player_turn(1).

% player_must_move_this_turn - if predicate is present value = TRUE, if not = FALSE  
player_must_move_this_turn.

% deprecated at this version of project.  ( if predicate is present value = TRUE, if not = FALSE )
% only_add_new_objects_as_possible_move.

% restricted_object_move_at_square(n) - if predicate is present value = TRUE, if not = FALSE
% restricted_object_move_at_square(1). 

% game_squares
game_square(0,2).
game_square(1,2).
game_square(2,2).
game_square(3,2).
game_square(4,2). 
game_square(8,2).
game_square(9,2).
game_square(10,2).
game_square(11,2).
game_square(12,2).
game_square(16,2).
game_square(17,2).
game_square(18,0).
game_square(19,1).
game_square(20,1).
game_square(24,1).
game_square(25,1).
game_square(26,1).
game_square(27,1).
game_square(28,1).
game_square(32,1).
game_square(33,1).
game_square(34,1).
game_square(35,1).
game_square(36,1).

%new objects     possible_new_object(type,count)    - if predicate is present value = count, if not = 0
%                possible_infinite_new_object(type) - if predicate is present value = infinity ( internally represented as 777 )
%possible_new_objects(0,0).
%possible_new_objects(1,0).
%possible_new_objects(2,0).

