#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "protos.h"
#include "globals.h"


#define ERROR_MOVE 9999



int pgn2pv(char *pgn, char * pv)
{
    unsigned fromMoves, toMoves;
    unsigned k;
    char *c;
    Move move;

    bool testPiece = true;
    bool testFrom_AH = true;
    bool testFrom_18 = true;
    bool testTo_AH = false;
    bool testTo_18 = false;
    bool testPromotion = false;

    char piece = 0;
    char from_AH = 0;
    char from_18 = 0;
    char to_AH = 0;
    char to_18 = 0;
    char promotion = 0;
    int to = 0;

    c = pgn;

    while (*c){
        if (testPiece) {
            if(*c == 'O'){
                piece = 'K';
                from_AH = 'e';
                to_AH = (strlen(pgn)==3) ? 'g':'c';
                from_18 = (board.color) ? '8':'1';
                to_18 = from_18;
                break;
            }
            if (strchr("KQRBN", *c)) {
               piece = *c;
               testPiece = false;
               c++;
               continue;
            }
            piece = 'P';
            testPiece = false;
        }
        if (testFrom_AH) {
           if (strchr("abcdefgh", *c)) {
                from_AH = *c;
                testFrom_AH = false;
                testTo_AH = true;
                c++;
                continue;
           }
        }
        if (testFrom_18) {
           if (strchr("12345678", *c)) {
                from_18 = *c;
                testFrom_AH = false;
                testFrom_18 = false;
                testTo_AH = true;
                c++;
                continue;
           }
        }
        if (testTo_AH) {
            if (strchr("abcdefgh", *c)) {
                to_AH = *c;
                testFrom_18 = false;
                testTo_AH = false;
                testTo_18 = true;
                c++;
                continue;
            }
        }
        if (testTo_18) {
            if (strchr("12345678", *c)) {
                to_18 = *c;
                testTo_18 = false;
                c++;
                continue;
            }
        }
        if (*c == 'x') {
            testFrom_18 = false;
            testFrom_AH = false;
            testTo_AH = true;
            c++;
            continue;
        }
        if (*c == '=') {
            testFrom_18 = false;
            testFrom_AH = false;
            testPromotion = true;
            c++;
            continue;
        }
        if (testPromotion) {
           if (strchr("QRBN", *c)) {
               promotion = *c;
               testPromotion = false;
               c++;
               continue;
           }
        }
        if (strchr(" +#\n\r", *c)) {
            c++;
            continue;
        }
        return ERROR_MOVE;
    }

    if ( !to_18 ) {
        to_18 = from_18;
        to_AH = from_AH;
        from_18 = 0;
        from_AH = 0;
    }


    to = (to_AH-'a') + (to_18-'1')*8;

    fromMoves = board.ply_moves[board.ply - 1];
    toMoves = board.ply_moves[board.ply];

    if(board.color){
        piece +=  'a' - 'A';
        if( promotion ) promotion += 'a' - 'A';
    }

    // printf( "%s, %d-%d ply(%d)\n", pgn, fromMoves, toMoves-1, board.ply);
    for (k = fromMoves; k < toMoves; k++) {
        move = board.moves[k];

                // printf("[%c %s%s ", NAMEPZ[move.piece], POS_AH[move.from], POS_AH[move.to]);
        // if (move.capture) {
            // printf("x%c ", NAMEPZ[move.capture]);
        // }
        // if (move.promotion) {
            // printf("prom%c ", NAMEPZ[move.promotion]);
        // }
        // if (move.is_ep) {
            // printf("ep ");
        // }
        // if (move.is_2p) {
            // printf("2p ");
        // }
        // if (move.is_castle) {
            // printf("castle %d ", move.is_castle);
        // }
        // printf("]");

        if( NAMEPZ[move.piece] == piece && move.to == to){
            // printf( "\n%s: %d, %d=%d, %d\n", pgn, from_AH, move.from, move.from%8, from_AH-'a' );
            if(from_AH && (move.from%8 != (from_AH-'a'))) continue;
            if(from_18 && (move.from/8 != (from_18-'1'))) continue;
            if( move.promotion && NAMEPZ[move.promotion] != promotion ) continue;
            if( promotion && !move.promotion ) continue;
            sprintf(pv, "%s%s", POS_AH[move.from], POS_AH[move.to]);
            if( promotion ) sprintf(pv, "%s%c", pv, promotion);
            // printf("..resp=%d\n",k);
            return k;
        }
    }

    return ERROR_MOVE;
}

int make_nummove(int num)
{
    make_move(board.moves[num]);
    return movegen();
}

char * playFen( char * fen, int depth, int time )
{
    fen_board( fen );
    return play( depth, time );
}

int numMoves(  )
{
    int fromMoves, toMoves;

    fromMoves = board.ply_moves[board.ply - 1];
    toMoves = board.ply_moves[board.ply];

    return toMoves-fromMoves;

}

int numBaseMove( )
{
    return board.ply_moves[board.ply - 1];
}

void getMove( int num, char * pv )
{
    Move move;
    move = board.moves[num];
    sprintf(pv, "%c%s%s", NAMEPZ[move.piece], POS_AH[move.from], POS_AH[move.to]);
    if( move.promotion ) sprintf(pv, "%s%c", pv, tolower(NAMEPZ[move.promotion]));
}

int searchMove( char *desde, char *hasta, char * promotion )
{
    int from, to, i;
    int fromMoves, toMoves;
    Move move;

    fromMoves = board.ply_moves[board.ply - 1];
    toMoves = board.ply_moves[board.ply];

    from = ah_pos(desde);
    to = ah_pos(hasta);

    for (i = fromMoves; i < toMoves; i++) {
        move = board.moves[i];
        if ( move.from == from && move.to == to ) {
            if( move.promotion && tolower(NAMEPZ[move.promotion]) != tolower(promotion[0]) ) continue;
            return i;
        }
    }
    return -1;
}

void getMoveEx( int num, char * info )
{
    Move move;
    char castle, en_passant;
    char promotion;

    move = board.moves[num];

    sprintf(info, "%c%s%s", NAMEPZ[move.piece], POS_AH[move.from], POS_AH[move.to]);
    if( move.promotion ) promotion = tolower(NAMEPZ[move.promotion]);
    else promotion = ' ';

    if( move.is_castle == CASTLE_OO ) castle = 'K';
    else if( move.is_castle == CASTLE_OOO ) castle = 'Q';
    else castle = ' ';

    if( move.is_ep ) en_passant = 'E';
    else en_passant = ' ';

    sprintf(info, "%s%c%c%c", info, promotion, castle, en_passant);
}

char * toSan(int num, char *sanMove)
{
    Move move, movet;
    int i;
    int fromMoves, toMoves;
    bool is_amb_ah, is_amb_18;

    fromMoves = board.ply_moves[board.ply - 1];
    toMoves = board.ply_moves[board.ply];

    move = board.moves[num];

    // Castle
    if( move.is_castle ){
        if( move.is_castle == CASTLE_OO ) sprintf(sanMove,"O-O");
        else sprintf(sanMove,"O-O-O");
    }

    // Pawns
    else if( (move.piece == WHITE_PAWN || move.piece == BLACK_PAWN)) {
        if( move.capture ) {
            sprintf(sanMove, "%cx%s", POS_AH[move.from][0], POS_AH[move.to]);
        }
        else  {
            sprintf(sanMove, "%s", POS_AH[move.to]);
        }
//        if( move.is_ep ){
//            sprintf(sanMove, "%s e.p.", sanMove);
//        }
        if( move.promotion ) {
            sprintf(sanMove, "%s=%c", sanMove, toupper(NAMEPZ[move.promotion]));
        }
    }

    // Pieces
    else {
        is_amb_ah = false;
        is_amb_18 = false;
        for(i=fromMoves; i<toMoves;i++ ){
            if( i != num ){
                movet = board.moves[i];
                if(move.to == movet.to && move.piece == movet.piece) {
                    if( COLUMNA(move.from) != COLUMNA(movet.from) ) is_amb_ah = true;
                    else if( FILA(move.from) != FILA(movet.from) ) is_amb_18 = true;
                }
            }
        }
        sprintf(sanMove,"%c", toupper(NAMEPZ[move.piece]));
        if( is_amb_ah ) sprintf(sanMove,"%s%c", sanMove, POS_AH[move.from][0]);
        if( is_amb_18 ) sprintf(sanMove,"%s%c", sanMove, POS_AH[move.from][1]);
        if(move.capture) sprintf(sanMove,"%sx", sanMove);
        sprintf(sanMove,"%s%s", sanMove, POS_AH[move.to]);
    }

    // Check + Mate
    make_move(move);
    if( inCheck() ){
        if(!movegen()){
            sprintf(sanMove,"%s#", sanMove);
        } else {
            sprintf(sanMove,"%s+", sanMove);
        }
    }
    unmake_move();
    return sanMove;
}
