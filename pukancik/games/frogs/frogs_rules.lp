% defines objects types which belong to player currently on turn ( specific secondary predicate for particular game )
object_type_turn(Type):-type_of_objects(Type,Player,_,_,_),player_turn(Player).

% these types of move result with attributes 0,0,1,1,0,0 , what means (0 and 0 are scores for player1 and player2, 1 = given player does continue in resulting game_state, 1 = player on turn in resulting state doesn't have to make a move, 0 = there is no such restriction if only this object can move in resulting game_state, 0 = there is not a possibility to create pass move in resulting game_state (even if player_must_move is set) if restricted object cannot move )
% frog can move to empty game_square, frog type 1 can move 1 position right
pmmv(Index_actual,Index_new,1,0,0,1,1,0,0):-game_square(Index_actual,1),object_type_turn(1),chsq_index(Row_actual,Column_actual,Index_actual),+(Column_actual,1,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0).

% frog can jump to empty game_square over opponents frog, frog type 1 can jump 2 positions right over frog type 2
pmmv(Index_actual,Index_new,1,0,0,1,1,0,0):-game_square(Index_actual,1),object_type_turn(1),chsq_index(Row_actual,Column_actual,Index_actual),+(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),+(Column_actual,1,Column_test),chsq_index(Row_actual,Column_test,Index_test_new),game_square(Index_test_new,2).

% frog can move to empty game_square, frog type 2 can move 1 position left  
pmmv(Index_actual,Index_new,2,0,0,1,1,0,0):-game_square(Index_actual,2),object_type_turn(2),chsq_index(Row_actual,Column_actual,Index_actual),-(Column_actual,1,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0).

% frog can jump to empty game_square over opponents frog, frog type 2 can jump 2 positions left over frog type 2
pmmv(Index_actual,Index_new,2,0,0,1,1,0,0):-game_square(Index_actual,2),object_type_turn(2),chsq_index(Row_actual,Column_actual,Index_actual),-(Column_actual,2,Column_new),chsq_index(Row_actual,Column_new,Index_new),game_square(Index_new,0),-(Column_actual,1,Column_test),chsq_index(Row_actual,Column_test,Index_test_new),game_square(Index_test_new,1).