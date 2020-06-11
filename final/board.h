#pragma once
#include <array>
#include <iostream>
#include <vector>
#include "utils.h"

#define F_LAYER 0x0055005500550055ULL
#define S_LAYER 0x0000333300003333ULL
#define T_LAYER 0x000000000F0F0F0FULL
#define BORDER  0x7e8181818181817eULL
#define CORNER  0x8100000000000081ULL

/**
 * bitboard
 *  (0)  (1)  (2)  (3)  (4)  (5)  (6)  (7)
 *  (8)  (9) (10) (11) (12) (13) (14) (15)
 * (16) (17) (18) (19) (20) (21) (22) (23)
 * (24) (25) (26) (27) (28) (29) (30) (31)
 * (32) (33) (34) (35) (36) (37) (38) (39)
 * (40) (41) (42) (43) (44) (45) (46) (47)
 * (48) (49) (50) (51) (52) (53) (54) (55)
 * (56) (57) (58) (59) (60) (61) (62) (63)
 */

class Board {
public:
    typedef uint64_t data;

public:
    Board() : board_white(0x0ULL), board_black(0x0ULL) {}
    Board(data black, data white) : board_white(white), board_black(black) {}
    Board(const Board& b) = default;
    Board& operator =(const Board& b) = default;
    bool operator !=(const Board& b) {
        return !((board_white == b.board_white) &&
                 (board_black == b.board_black));
    }

    void set_black(data black)  { board_black = black; }
    void set_white(data white)  { board_white = white; }

    data& get_board(unsigned int i) {
        return (i) ? board_white : board_black;
    }
    const data& get_board(unsigned int i) const {
        return (i) ? board_white : board_black;
    }

    const bool game_over() const {
        return (board_white | board_black) == 0x7effffffffffff7eULL;
    }

    const void print_board() const {
        std::cout << "  ";
        for (int j = 0; j < 8; j++) {
            std::cout << j << " ";
        }
        std::cout << "\n";
        for (int i = 0; i < 8; i++) {
            std::cout << i << " ";
            for (int j = 0; j < 8; j++) {
                if (board_black & (1ULL << (i * 8 + j))) std::cout << "O ";
                else if (board_white & (1ULL << (i * 8 + j))) std::cout << "X ";
                else std::cout << "  ";
            }
            std::cout << "\n";
        }
        std::cout << "===================\n";
    }

public:
    void get_possible_action(std::vector<unsigned> &eats, std::vector<unsigned> &moves, int color) const {
        data mine = color ? board_white : board_black;
        data theirs = (color ^ 1) ? board_white : board_black;
        data occupied = mine | theirs | CORNER;
        data empty = ~occupied;
        data empty_border = empty & BORDER;
        data empty_normal = empty & ~empty_border;

        eats.clear(); moves.clear();
        for (int i = 0; i < 64; i++) {
            if (empty_border & (1ULL << i)) {
                if (get_eaten_pieces(i, color) > 0) eats.push_back(i);
            }
            if (empty_normal & (1ULL << i)) {
                if (get_eaten_pieces(i, color) > 0) eats.push_back(i);
                else                                moves.push_back(i);
            }
        }
    }

    data get_eaten_pieces(int pos, int color) const {
        data mine = color ? board_white : board_black;
        data theirs = (color ^ 1) ? board_white : board_black;
        data pieces = 0, tmp;
        int posx = pos >> 3, posy = pos & 0b111;

        tmp = 0;
        for (int x = posx - 1, y = posy - 1; x >= 0 && y >= 0; x--, y--) {
            int i = (x << 3 | y);
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }
        tmp = 0;
        for (int x = posx + 1, y = posy + 1; x < 8 && y < 8; x++, y++) {
            int i = (x << 3 | y);
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }

        tmp = 0;
        for (int x = posx - 1, y = posy + 1; x >= 0 && y < 8; x--, y++) {
            int i = (x << 3 | y);
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }
        tmp = 0;
        for (int x = posx + 1, y = posy - 1; x < 8 && y >= 0; x++, y--) {
            int i = (x << 3 | y);
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }

        tmp = 0;
        for (int i = pos - 8; i >= 0; i -= 8) {
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }
        tmp = 0;
        for (int i = pos + 8; i < 64; i += 8) {
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }

        tmp = 0;
        int limit = pos - pos % 8;
        for (int i = pos - 1; i >= limit; i -= 1) {
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }
        tmp = 0;
        limit = pos - pos % 8 + 8;
        for (int i = pos + 1; i < limit; i += 1) {
            if (mine & (1ULL << i)) pieces |= tmp;
            if (!(theirs & (1ULL << i)))   break;
            tmp |= (1ULL << i);
        }

        return pieces;
    }

public:
    int eat(unsigned destination, int color) {
        if (color) {
            data pieces = get_eaten_pieces(destination, color);
            board_black &= ~pieces;
            board_white |= pieces;
            board_white |= (1ULL << destination);
        }
        else {
            data pieces = get_eaten_pieces(destination, color);
            board_white &= ~pieces;
            board_black |= pieces;
            board_black |= (1ULL << destination);
        }
        return 1;
    }

    int move(unsigned destination, int color) {
        if (color)  board_white |= (1ULL << destination);
        else        board_black |= (1ULL << destination);
        return 1;
    }

public:
    void rotate(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: break;
            case 1: board_operation(1, 8); break; // rotate right
            case 2: board_operation(9, 7); break; // reverse
            case 3: board_operation(8, -1);break; // rotate left
        }
    }

    void transpose() {
        board_operation(0, 7);
    }

    //rotate then transpose
    void rotate_tran(int r = 1) {
        switch (((r % 4) + 4) % 4) {
            default:
            case 0: board_operation(0, 7); break; // same as transpose
            case 1: board_operation(8, 8); break;
            case 2: board_operation(9, 0); break;
            case 3: board_operation(1, -1);break;
        }
    }

protected:
    /**
     * Board Operation
     *
     * A.The whole 8*8 board could be devided into four 4*4 boards, 
     *   and each of them contains four 2*2 boards. Show as below.
     * 
     *     (0)  (1) |  (2)  (3)  
     *     (8)  (9) | (10) (11)    <- top-left 4*4 board
     *    --------------------
     *    (16) (17) | (18) (19)
     *    (24) (25) | (26) (27)
     * 
     * B.The 8*8 board clockwise rotation could be seen as a total of the three actions below:
     *    First, rotate the cells in every 2*2 board clockwise.
     *       (8)  (0) | (10)  (2)  
     *       (9)  (1) | (11)  (3)
     *      --------------------
     *      (24) (16) | (26) (18)
     *      (25) (17) | (27) (19)
     *
     *    Second, move the position of four 2*2 clockwise in every 4*4 boards. 
     *      (24) (16) |  (8)  (0)
     *      (25) (17) |  (9)  (1)
     *      --------------------
     *      (26) (18) | (10)  (2)
     *      (27) (19) | (11)  (3)
     *
     *    Last, move the four 4*4 clockwise in the 8*8 board
     *      -The 4*4 above is now at top-right of the 8*8 board
     * 
     * C.On bitboard, the first action could be done by the union of four cells bit shifting.
     *
     * D.Furthermore, if we view a 2*2 as a cell, the second action is similar to the first with
     *   two times the shifting digits at the corresponding position.As well as the third with 
     *   4*4 taken. That is, the position of the cell (no matter it is a 1*1, 2*2, or 4*4) is the  
     *   multiplicand of the digits to shift, while the size is the multiplier.
     *
     * E.The digits shift of four positions are required parameters. However, the cell diagonal
     *   with will always be the same, so two parameters are enough.
     *
     * F.The operation of transposing, rotating several times, and the merge of them are similar.
     *   So all of them can be done in the three actions (three bit-operations) with two parameters.
     */

    void board_operation(const int& a, const int& b) {
        if (b >= 0) {
            board_white = ((board_white &  F_LAYER       ) << a |
                           (board_white & (F_LAYER <<  1)) << b |
                           (board_white & (F_LAYER <<  8)) >> b |
                           (board_white & (F_LAYER <<  9)) >> a);

            board_white = ((board_white &  S_LAYER       ) << (a << 1) |
                           (board_white & (S_LAYER <<  2)) << (b << 1) |
                           (board_white & (S_LAYER << 16)) >> (b << 1) |
                           (board_white & (S_LAYER << 18)) >> (a << 1));

            board_white = ((board_white &  T_LAYER       ) << (a << 2) |
                           (board_white & (T_LAYER <<  4)) << (b << 2) |
                           (board_white & (T_LAYER << 32)) >> (b << 2) |
                           (board_white & (T_LAYER << 36)) >> (a << 2));

            board_black = ((board_black &  F_LAYER       ) << a |
                           (board_black & (F_LAYER <<  1)) << b |
                           (board_black & (F_LAYER <<  8)) >> b |
                           (board_black & (F_LAYER <<  9)) >> a);

            board_black = ((board_black &  S_LAYER       ) << (a << 1) |
                           (board_black & (S_LAYER <<  2)) << (b << 1) |
                           (board_black & (S_LAYER << 16)) >> (b << 1) |
                           (board_black & (S_LAYER << 18)) >> (a << 1));

            board_black = ((board_black &  T_LAYER       ) << (a << 2) |
                           (board_black & (T_LAYER <<  4)) << (b << 2) |
                           (board_black & (T_LAYER << 32)) >> (b << 2) |
                           (board_black & (T_LAYER << 36)) >> (a << 2));
        }
        else {
            board_white = ((board_white &  F_LAYER       ) <<  a |
                           (board_white & (F_LAYER <<  1)) >> -b |
                           (board_white & (F_LAYER <<  8)) << -b |
                           (board_white & (F_LAYER <<  9)) >> a);

            board_white = ((board_white &  S_LAYER       ) << ( a << 1) |
                           (board_white & (S_LAYER <<  2)) >> (-b << 1) |
                           (board_white & (S_LAYER << 16)) << (-b << 1) |
                           (board_white & (S_LAYER << 18)) >> ( a << 1));

            board_white = ((board_white &  T_LAYER       ) << ( a << 2) |
                           (board_white & (T_LAYER <<  4)) >> (-b << 2) |
                           (board_white & (T_LAYER << 32)) << (-b << 2) |
                           (board_white & (T_LAYER << 36)) >> ( a << 2));

            board_black = ((board_black &  F_LAYER       ) <<  a |
                           (board_black & (F_LAYER <<  1)) >> -b |
                           (board_black & (F_LAYER <<  8)) << -b |
                           (board_black & (F_LAYER <<  9)) >> a);

            board_black = ((board_black &  S_LAYER       ) << ( a << 1) |
                           (board_black & (S_LAYER <<  2)) >> (-b << 1) |
                           (board_black & (S_LAYER << 16)) << (-b << 1) |
                           (board_black & (S_LAYER << 18)) >> ( a << 1));

            board_black = ((board_black &  T_LAYER       ) << ( a << 2) |
                           (board_black & (T_LAYER <<  4)) >> (-b << 2) |
                           (board_black & (T_LAYER << 32)) << (-b << 2) |
                           (board_black & (T_LAYER << 36)) >> ( a << 2));
        }
    }

private:
    data board_white;
    data board_black;
};