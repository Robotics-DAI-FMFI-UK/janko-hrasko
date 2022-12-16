:- player_turn(P1), player_turn(P2), P1 != P2.
:- game_square(GS,T1), game_square(GS,T2), T1 != T2. 
:- type_of_objects(Type,P1,_,_,_), type_of_objects(Type,P2,_,_,_), P1 != P2.
:- type_of_objects(Type,_,St1,_,_), type_of_objects(Type,_,St2,_,_), St1 != St2.
:- type_of_objects(Type,_,_,C1,_), type_of_objects(Type,_,_,C2,_), C1 != C2.
:- type_of_objects(Type,_,_,_,Sh1), type_of_objects(Type,_,_,_,Sh2), Sh1 != Sh2.