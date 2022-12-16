chessboard_width(8).
chessboard_height(6).

% ------------------------------------------------------------------------------
% chessboard indices
chsq_index(Row,Column,Index) :- #int(Temp),#int(Index),#int(Row),#int(Column),
                                 chessboard_height(H), Row < H,
                                 chessboard_width(W), Column < W,
                                 *(Row,W,Temp), 
                                 +(Temp,Column,Index).
                                        
% ------------------------------------------------------------------------------

% secondary predicates
even(0).
even(N) :- #int(N), #int(M), *(M,2,N).
odd(N) :- even(M), +(M,1,N).
 
% 'star_gs'  - will define game_square indices, which could lead to all 8 directions
% 'cross_gs' - will define game_square indices, which could lead to only 4 main directions
star_gs(Index) :- playable_area(_,_,PA_tlc), chsq_index(PA_tlc_row,PA_tlc_column,PA_tlc), +(PA_tlc_row,PA_tlc_column,Check), even(Check), game_square(Index,_), chsq_index(Index_row,Index_column,Index), +(Index_row,Index_column,Check_2), even(Check_2).
star_gs(Index) :- playable_area(_,_,PA_tlc), chsq_index(PA_tlc_row,PA_tlc_column,PA_tlc), +(PA_tlc_row,PA_tlc_column,Check), odd(Check), game_square(Index,_), chsq_index(Index_row,Index_column,Index), +(Index_row,Index_column,Check_2), odd(Check_2).

cross_gs(Index) :- game_square(Index,_), not star_gs(Index).