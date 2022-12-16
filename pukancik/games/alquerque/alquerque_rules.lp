% defines objects types which belong to player currently on turn ( specific secondary predicate for particular game )
object_type_turn(Type):-type_of_objects(Type,Player,_,_,_),player_turn(Player),Type>0.

% defines objects types which belong to player currently not on turn ( specific secondary predicate for particular game )
object_type_not_on_turn(Type):-type_of_objects(Type,Player,_,_,_),not player_turn(Player),Type>0.

% if restricted_object is set, allow only moves that cross opponent object 
restriction_is_set :- restricted_object_move_at_square(_).

% if there is some move that can take opponents object, set this predicate to suppress simple type of move, only taking opponents object is then permited
rsem_is_set :- rsem(_,_,_,_,_).

% these types of move result with attributes 0,0,0,0,0,0 , what means (0 and 0 are scores for player1 and player2, 0 = given player doesn't continue in resulting game_state, 0 = player on turn in resulting state doesn't have to make a move, 0 = there is no such restriction if only this object can move in resulting game_state, 0 = there is not a possibility to create pass move in resulting game_state (even if player_must_move is set) if restricted object cannot move )
% try to move object - one square to right
pmmv(Index_actual,Index_new,Type,0,0,0,0,0,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Column_actual,1,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% try to move object - one square to left
pmmv(Index_actual,Index_new,Type,0,0,0,0,0,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Column_actual,1,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% try to move object - one square up - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,1,0,0,0,0,0,0):-game_square(Index_actual,1),object_type_turn(1),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% try to move object - one square down - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,2,0,0,0,0,0,0):-game_square(Index_actual,2),object_type_turn(2),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% if object on game_square, where can go also diagonal, try to move object - one square up and to right - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),+(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,1,0,0,0,0,0,0):-game_square(Index_actual,1),star_gs(Index_actual),object_type_turn(1),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),+(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% if object on game_square, where can go also diagonal, try to move object - one square up and to left - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),-(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,1,0,0,0,0,0,0):-game_square(Index_actual,1),star_gs(Index_actual),object_type_turn(1),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,1,Row_new),-(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% if object on game_square, where can go also diagonal, try to move object - one square down and to right - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),+(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,2,0,0,0,0,0,0):-game_square(Index_actual,2),star_gs(Index_actual),object_type_turn(2),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),+(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% if object on game_square, where can go also diagonal, try to move object - one square down and to left - edited version: objects can move only up or down with this type of simple moves - type 1 starts from down and can go up, type 2 starts on top and can move down
%pmmv(Index_actual,Index_new,Type,0,0,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),-(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.
pmmv(Index_actual,Index_new,2,0,0,0,0,0,0):-game_square(Index_actual,2),star_gs(Index_actual),object_type_turn(2),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,1,Row_new),-(Column_actual,1,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0), not restriction_is_set, not rsem_is_set.

% ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
% these types of move result with attributes 0,0,1,1,1,1 , what means (0 and 0 are scores for player1 and player2, 1 = given player does continue in resulting game_state, 1 = player on turn in resulting state does have to make a move, 1 = there is restriction that only this object can move in resulting game_state, 1 = there is possibility to create pass move in resulting game_state (even if player_must_move is set) if restricted object cannot move )
% try to move object over object of opposing player - two square to right
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_actual,Column_test,Index_test),+(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects 
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_actual,Column_test,Index_test),+(Column_actual,1,Column_test).

% try to move object over object of opposing player - two square to left
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_actual,Column_test,Index_test),-(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects 
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_actual,Column_test,Index_test),-(Column_actual,1,Column_test).

% try to move object over object of opposing player - two square up
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_actual,Index_test),-(Row_actual,1,Row_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects 
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_actual,Index_test),-(Row_actual,1,Row_test).

% try to move object over object of opposing player - two square down
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_actual,Index_test),+(Row_actual,1,Row_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects 
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),chsq_index(Row_new,Column_actual,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_actual,Index_test),+(Row_actual,1,Row_test).


% if object on game_square, where can go also diagonal, try to move object - two square up and two to right
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),+(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),-(Row_actual,1,Row_test),+(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),+(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),-(Row_actual,1,Row_test),+(Column_actual,1,Column_test).

% if object on game_square, where can go also diagonal, try to move object - two square up and two to left
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),-(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),-(Row_actual,1,Row_test),-(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),-(Row_actual,2,Row_new),-(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),-(Row_actual,1,Row_test),-(Column_actual,1,Column_test).

% if object on game_square, where can go also diagonal, try to move object - two square down and two to right
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),+(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),+(Row_actual,1,Row_test),+(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),+(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),+(Row_actual,1,Row_test),+(Column_actual,1,Column_test).

% if object on game_square, where can go also diagonal, try to move object - two square down and two to left
pmmv(Index_actual,Index_new,Type,0,0,1,1,1,1):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),-(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),+(Row_actual,1,Row_test),-(Column_actual,1,Column_test).
% this possible_move has side_effect of removing that object , 0 as attribute means that removed object is not added to possible_new_objects
rsem(Index_actual,Index_new,Type,Index_test,0):-game_square(Index_actual,Type),star_gs(Index_actual),object_type_turn(Type),chsq_index(Row_actual,Column_actual,Index_actual),+(Row_actual,2,Row_new),-(Column_actual,2,Column_new),chsq_index(Row_new,Column_new,Index_new),game_square(Index_new,0),game_square(Index_test,Type_opponent),object_type_not_on_turn(Type_opponent),chsq_index(Row_test,Column_test,Index_test),+(Row_actual,1,Row_test),-(Column_actual,1,Column_test).



