% defines objects types which belong to player currently on turn ( specific secondary predicate for particular game )
object_type_turn(Type):-type_of_objects(Type,Player,_,_,_),player_turn(Player).

% these types of move result with attributes 0,0,0,1,0,0 , what means (0 and 0 are scores for player1 and player2, 0 = given player doesn't continue in resulting game_state, 1 = player on turn in resulting state does have to make a move, 0 = there is no such restriction if only this object can move in resulting game_state, 0 = there is not a possibility to create pass move in resulting game_state (even if player_must_move is set) if restricted object cannot move )
% empty game_squares in lowest row of playable area
pman(Index_to,Type,0,0,0,1,0,0):-game_square(Index_to,0),object_type_turn(Type),chsq_index(Row,Column,Index_to),lowest_row_index(Row).

% empty game_squares which have non-empty game_squares right below them
pman(Index_to,Type,0,0,0,1,0,0):-game_square(Index_to,0),object_type_turn(Type),chsq_index(Row_to,Column_to,Index_to),+(Row_to,1,Row_test),chsq_index(Row_test,Column_to,Index_test),lowest_row_index(Row_last),Row_last>Row_to,not game_square(Index_test,0).