% --  which player is on turn ( 1 - first player , 2 - 2nd player )
player_turn(1).

% player_must_move_this_turn - if predicate is present value = TRUE, if not = FALSE  
player_must_move_this_turn.

% deprecated at this version of project.  ( if predicate is present value = TRUE, if not = FALSE )
% only_add_new_objects_as_possible_move.

% restricted_object_move_at_square(n) - if predicate is present value = TRUE, if not = FALSE
% restricted_object_move_at_square(1). 

% game_squares
game_square(0,0).
game_square(1,0).
game_square(2,0).
game_square(3,0).
game_square(4,0).
game_square(5,0).
game_square(6,0).
game_square(7,0).
game_square(8,0).
game_square(9,0).
game_square(10,0).
game_square(11,0).
game_square(12,0).
game_square(13,0).
game_square(14,0).
game_square(15,0).
game_square(16,0).
game_square(17,0).
game_square(18,0).
game_square(19,0).
game_square(20,0).
game_square(21,0).
game_square(22,0).
game_square(23,0).
game_square(24,0).
game_square(25,0).
game_square(26,0).
game_square(27,0).
game_square(28,0).
game_square(29,0).
game_square(30,0).
game_square(31,0).
game_square(32,0).
game_square(33,0).
game_square(34,0).
game_square(35,0).
game_square(36,0).
game_square(37,0).
game_square(38,0).
game_square(39,0).
game_square(40,0).
game_square(41,0).
game_square(42,0).
game_square(43,0).
game_square(44,0).
game_square(45,0).
game_square(46,0).
game_square(47,0).

%new objects     possible_new_object(type,count)  ak nebude predikat pre dany typ tak = 0
%                possible_infinite_new_object(type) ak je dany predikat, tento Type of object bude nekonecno, vnutorne reprezentovane ako 777
%possible_new_objects(0,0).
possible_infinite_new_objects(1).
possible_infinite_new_objects(2).

