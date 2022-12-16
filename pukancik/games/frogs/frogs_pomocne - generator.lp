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