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

% secondary predicate - index of lowest row of playable area on chessboard area                               
lowest_row_index(Row) :- playable_area(PA_w,PA_h,PA_tlc), chsq_index(PA_tlc_row, PA_tlc_column, PA_tlc), -(PA_h,1,PA_h_temp), +(PA_h_temp, PA_tlc_row, Row).
                                    