#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <random>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <utility>
#include <thread>
#include <mutex>

/************* utils.h *************/

inline int Bitcount(uint64_t b) {
    b = (b & 0x5555555555555555) + ((b >> 1) & 0x5555555555555555);
    b = (b & 0x3333333333333333) + ((b >> 2) & 0x3333333333333333);
    return (((b + (b >> 4)) & 0x0f0f0f0f0f0f0f0f) * 0x0101010101010101) >> 56;
}

/************* board.h *************/

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

protected:
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

/************* weight.h *************/

class Weight {
public:
    Weight() {}
    Weight(size_t len) : value(len) {}
    Weight(Weight&& f) : value(std::move(f.value)) {}
    Weight(const Weight& f) = default;

    Weight& operator =(const Weight& f) = default;
    float& operator[] (size_t i) { return value[i]; }
    const float& operator[] (size_t i) const { return value[i]; }
    size_t size() const { return value.size(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const Weight& w) {
        auto& value = w.value;
        uint64_t size = value.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(uint64_t));
        out.write(reinterpret_cast<const char*>(value.data()), sizeof(float) * size);
        return out;
    }
    friend std::istream& operator >>(std::istream& in, Weight& w) {
        auto &value = w.value;
        uint64_t size = 0;
        in.read(reinterpret_cast<char*>(&size), sizeof(uint64_t));
        value.resize(size);
        in.read(reinterpret_cast<char*>(value.data()), sizeof(float) * size);
        return in; 
    }

protected:
    std::vector<float> value;
};

/************* tuple.h *************/

class Tuple {
public:
    Tuple(const std::string& args = "") {
        std::stringstream ss(args);
        for (std::string pair; ss >> pair; ) {
            std::string key = pair.substr(0, pair.find('='));
            std::string value = pair.substr(pair.find('=') + 1);
            meta[key] = { value };
        }
        if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
            load_weights(meta["load"]);
        else
            init_weight();
    }
    ~Tuple() {}

private:
    typedef std::string key;
    struct value {
        std::string value;
        operator std::string() const { return value; }
        template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
        operator numeric() const { return numeric(std::stod(value)); }
    };
    std::map<key, value> meta;

private:
    void init_weight() {
        tuple_normal.emplace_back(65536);
        tuple_border.emplace_back(65536);
    }

    void load_weights(const std::string& path) {
        std::ifstream in(path, std::ios::in | std::ios::binary);
        if (!in.is_open()) std::exit(-1);
        uint32_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size));

        tuple_normal.resize(size / 2); 
        for (Weight& w : tuple_normal) in >> w;
        tuple_border.resize(size / 2);
        for (Weight& w : tuple_border) in >> w;
        in.close();
    }

public:
    float get_board_value(const Board &board, const int color) {  // 0 black 1 white
        Board b(board.get_board(0 ^ color), board.get_board(1 ^ color));
        float value = 0.0f;

        for (int i = 4; i > 0; i--) {
            // for (int i = 0; i < 13; i++) {
            //     uint32_t index = board_to_tuple_index(b, oblique[i]);
            //     value += tuple_normal[0][index];
            // }
            for (int i = 0; i < 6; i++) {
                uint32_t index = board_to_tuple_index(b, straight[i]);
                value += tuple_normal[0][index];
            }
            for (int i = 0; i < 2; i++) {
                uint32_t index = board_to_tuple_index(b, border[i]);
                value += tuple_border[0][index];
            }
            b.rotate(1);
        }
        return value / 84.0f;
    }

private:
    uint32_t board_to_tuple_index(const Board &b, const int position[]) {
        const Board::data white = b.get_board(1);
        const Board::data black = b.get_board(0);
        
        uint32_t index = 0;
        for(int i = 0; i < 8; i++){
            index <<= 2;
            if (position[i] == -1)  index |= 0b11;
            else {
                index |= (white >> (position[i] - 1)) & 2;
                index |= (black >> position[i]) & 1;
            }
        }
        return index;
    }

private:
    const int oblique[13][8] = {{  1,  8, -1, -1, -1, -1, -1, -1 },
                                {  2,  9, 16, -1, -1, -1, -1, -1 },
                                {  3, 10, 17, 24, -1, -1, -1, -1 },
                                {  4, 11, 18, 25, 32, -1, -1, -1 },
                                {  5, 12, 19, 26, 33, 40, -1, -1 },
                                {  6, 13, 20, 27, 34, 41, 48, -1 },
                                { 14, 21, 28, 35, 42, 49, -1, -1 },
                                { 15, 22, 29, 36, 43, 50, 57, -1 },
                                { 23, 30, 37, 44, 51, 58, -1, -1 },
                                { 31, 38, 45, 52, 59, -1, -1, -1 },
                                { 39, 46, 53, 60, -1, -1, -1, -1 },
                                { 47, 54, 61, -1, -1, -1, -1, -1 },
                                { 55, 62, -1, -1, -1, -1, -1, -1 }};

    const int straight[6][8] = {{ 1,  9, 17, 25, 33, 41, 49, 57 },
                                { 2, 10, 18, 26, 34, 42, 50, 58 },
                                { 3, 11, 19, 27, 35, 43, 51, 59 },
                                { 4, 12, 20, 28, 36, 44, 52, 60 },
                                { 5, 13, 21, 29, 37, 45, 53, 61 },
                                { 6, 14, 22, 30, 38, 46, 54, 62 }};
    
    const int border[6][8] = {{  8, 16, 24, 32, 40, 48, -1, -1 },
                              { 15, 23, 31, 39, 47, 55, -1, -1 }};

    std::vector<Weight> tuple_normal, tuple_border;
};

/************* tree.h *************/

class TreeNode {
public:
    TreeNode() {}
    TreeNode(const Board &b) : 
        parent(NULL),
        board(b),
        win_count(1),
        visit_count(2),
        state_value(0.0f),
        softmax_value(1.0f),
        child_softmax_total(0.0),
        player(0),
        explore(false) { child.clear(); }

    TreeNode(const Board &b,
             float state_value,
             float softmax_value,
             int player,
             TreeNode* parent) : 
        parent(parent),
        board(b),
        win_count(1),
        visit_count(2),
        state_value(state_value),
        softmax_value(softmax_value),
        child_softmax_total(0.0),
        player(player),
        explore(false) { child.clear(); }

    TreeNode(const TreeNode& node) = default;
    TreeNode& operator =(const TreeNode& node) = default;

public:
    TreeNode* get_parent() { return parent; }
    void set_parent(TreeNode *parent) { this->parent = parent; }

    Board& get_board() { return board; }
    const Board& get_board() const { return board; }

    int get_win_count() { return win_count; }
    void add_win_count() { win_count++; }

    int get_visit_count() { return visit_count; }
    void add_visit_count() { visit_count++; }

    float get_state_value() { return state_value; }
    void set_state_value(float state_value) { this->state_value = state_value; }

    float get_softmax_value() { return softmax_value; }
    void set_softmax_value(float softmax_value) { this->softmax_value = softmax_value; }

    float get_child_softmax_total() { return child_softmax_total; }
    void set_child_softmax_total(float child_softmax_total) { this->child_softmax_total = child_softmax_total; }

    int get_player() { return player; }
    void set_player(int player) { this->player = player; }

    std::vector<TreeNode>& get_all_child() { return child; }
    TreeNode& get_child(int index) { return child.at(index); }

    bool is_explore() { return explore; }
    void set_explore() { explore = true; }

    TreeNode get_best_child_node() {
        return *std::max_element(child.begin(), child.end(),
                                 [](const TreeNode A, const TreeNode B) { return A.visit_count < B.visit_count; });
    }

private:
    TreeNode *parent;
    Board board;
    int win_count;
    int visit_count;
    float state_value; // tuple value
    float softmax_value;
    float child_softmax_total;
    int player; // current player
    bool explore;
    std::vector<TreeNode> child;
};

class Tree {
public:
    Tree(const Board& b) { root = TreeNode(b); }
    TreeNode& get_root() { return root; }

private:
    TreeNode root;
};

/************* mcts.h *************/

class MCTS {
public:
    MCTS(Tuple *tuple, bool with_tuple = false, int simulation_count = 5000, uint32_t seed = 10) :
        tuple(tuple),
        with_tuple(with_tuple),
        simulation_count(simulation_count) { engine.seed(seed); }

    void playing(Board &board, int player, int sim) {
        TreeNode node = find_next_move(board, player, sim);
        if (node.get_board() != board) {
            board = node.get_board();
        }
    }

    // return board after best action
    TreeNode find_next_move(Board board, int player, int sim) {
        Tree tree(board);
        TreeNode root = tree.get_root();
        root.set_explore();
        root.set_player(player);

        for (int i = 0; i < simulation_count; i++) {
            // std::cout << i << " " << simulation_count << "\n";
            // Phase 1 - Selection 
            TreeNode* leaf = selection(&root);
            // Phase 2 - Expansion
            if (leaf->is_explore()) leaf = expansion(leaf);
            leaf->set_explore();
            // Phase 3 - Simulation
            int value = simulation(leaf, sim);
            // Phase 4 - Backpropagation
            backpropagation(leaf, value);
        }

        // cannot find move
        if (root.get_all_child().size() == 0) return root;

        return root.get_best_child_node();
    }

private:
    TreeNode* selection(TreeNode* root) {
        // std::cout << "selection\n";
        TreeNode* current_node = root;
        TreeNode* best_node = nullptr; // best child node in one layer

        while (current_node->get_all_child().size() != 0) {
            float best_value = -1e9;
            const float t = float(current_node->get_visit_count());
            const float child_softmax_sum = current_node->get_child_softmax_total();
            std::vector<TreeNode> &child = current_node->get_all_child();

            // find the child with maximum PUCB value
            for (size_t i = 0; i < child.size(); i++) {
                float w = -float(child[i].get_win_count());
                float n = float(child[i].get_visit_count());
                float q = w / n;
                float value;

                // check whether MCTS with tuple value
                if (with_tuple) {
                    float poly = child[i].get_softmax_value() / child_softmax_sum;
                    float ucb = sqrt(t) / n;
                    value = q + poly * ucb * 3;
                }
                else {
                    value = q + sqrt(2 * log2(t) / n);
                }

                if (best_value < value) {
                    best_value = value;
                    best_node = &child[i];
                }
            }
            current_node = best_node;
        }
        return current_node;
    }

    TreeNode* expansion(TreeNode* leaf) {
        // std::cout << "expansion\n";
        const Board& board = leaf->get_board();
        // no need to expand if game is over
        if (board.game_over())  return leaf;

        int player = leaf->get_player();
        float child_softmax_total = 0;
        const float softmax_coefficient = 4;

        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, player);

        // expand all the possible child node, calculate tuple value, record previous action
        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, player);
            float state_value = tuple->get_board_value(tmp, player);
            float softmax_value = exp(state_value * softmax_coefficient);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf
            ));
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, player);
            float state_value = tuple->get_board_value(tmp, player);
            float softmax_value = exp(state_value * softmax_coefficient);
            child_softmax_total += softmax_value;
            leaf->get_all_child().push_back(TreeNode(
                tmp,
                state_value,
                softmax_value,
                player ^ 1,
                leaf
            ));
        }
        leaf->set_child_softmax_total(child_softmax_total);

        // there are no actions can be made
        if (leaf->get_all_child().size() == 0) return leaf;

        // randomly pick one child
        std::uniform_int_distribution<int> dis(0, leaf->get_all_child().size() - 1);
        return &(leaf->get_child(dis(engine)));
    }

    int simulation(TreeNode *leaf, int sim) {
        // std::cout << "simulation\n";
        TreeNode leaf_copy(*leaf);
        Board& board = leaf_copy.get_board();
        int player = leaf_copy.get_player();
        const int origin_player = player;
        
        // check if game is over before simulation
        if (board.game_over()) {
            int black_bitcount = Bitcount(board.get_board(0));
            int white_bitcount = Bitcount(board.get_board(1));
            if (origin_player == 0) return black_bitcount - white_bitcount;
            else                    return white_bitcount - black_bitcount;
        }

        std::uniform_real_distribution<> dis(0, 1);
        std::vector<unsigned> eats, moves;
        int skip_count = 0;

        // playout for at most 100 steps
        for (int i = 0; i < 100 && !board.game_over(); i++) {
            board.get_possible_action(eats, moves, player);
            std::shuffle(eats.begin(), eats.end(), engine);
            std::shuffle(moves.begin(), moves.end(), engine);

            Board before(board);

            // random
            if (sim == 0) {
                int size1 = eats.size(), size2 = moves.size();
                if (dis(engine) * (size1 + size2) < size1) {
                    if (eats.size() > 0) {
                        board.eat(eats[0] & 0b111111, player);
                    }
                }
                else {
                    if (moves.size() > 0) {
                        board.move(moves[0] & 0b111111, player);
                    }
                }
            }
            // eat first
            else if (sim == 1) {
                if (eats.size() > 0) {
                    board.eat(eats[0] & 0b111111, player);
                }
                else if (moves.size() > 0) {
                    board.move(moves[0] & 0b111111, player);
                }
            }

            if (before != board) skip_count = 0;
            else                 skip_count++;
            if (skip_count == 2)    break;

            player ^= 1; // toggle player
        }

        // the one has more piece wins
        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        // std::cout << black_bitcount << " " << white_bitcount << std::endl;
        if (origin_player == 0) return black_bitcount - white_bitcount;
        else                    return white_bitcount - black_bitcount;
    }

    void backpropagation(TreeNode *node, int value) {
        // std::cout << "backpropagation\n";
        while (node != NULL) {
            node->add_visit_count();
            if (value > 0) node->add_win_count();
            node = node->get_parent();
            value *= -1;
        }
    }

private:
    Tuple *tuple;
    const bool with_tuple;
    const int simulation_count;
    std::default_random_engine engine;
};

/************* main.cpp *************/

const std::string PLAYER[] = {"MCTS_with_tuple", "MCTS"};
const std::string SIMULATION[] = {"(random)", "(eat-first)"};
std::mutex mtx;
int fight_black_win, fight_white_win;

void fight_thread(int player1, int player2, int sim1, int sim2, Tuple *tuple, int game_count, uint32_t seed) {
    MCTS mcts_tuple(tuple, true, 60000, seed);
    MCTS mcts(tuple, false, 60000, seed);
    
    int black_win = 0, white_win = 0;
    for (int i = 0; i < game_count; i++) {
        // std::cout << i << "\n";
        Board board;
        int color = 0, current, sim;
        int skip_count = 0;
        while (true) {
            // std::cout << "hi\n";
            Board before(board);
            current = color ? player2 : player1;
            sim = color ? sim2 : sim1;
            switch (current) {
                case 0:
                    mcts_tuple.playing(board, color, sim);
                    break;
                case 1:
                    mcts.playing(board, color, sim);
                    break;
                default:
                    break;
            }
            if (before != board) skip_count = 0;
            else                 skip_count++;
            if (skip_count == 2)    break;
            if (board.game_over())  break;

            color ^= 1;
        }

        int black_bitcount = Bitcount(board.get_board(0));
        int white_bitcount = Bitcount(board.get_board(1));
        // if (black_bitcount + white_bitcount < 60) board.print_board();
        if (black_bitcount > white_bitcount)    black_win++;
        if (black_bitcount < white_bitcount)    white_win++;
    }

    mtx.lock();
    fight_black_win += black_win;
    fight_white_win += white_win;
    mtx.unlock();
}

void fight(int player1, int player2, int sim1, int sim2, Tuple *tuple, int game_count) {
    std::cout << PLAYER[player1];
    if (player1 <= 1)   std::cout << SIMULATION[sim1];
    std::cout << " VS " << PLAYER[player2];
    if (player2 <= 1)   std::cout << SIMULATION[sim2];
    std::cout << std::endl;

    fight_black_win = 0, fight_white_win = 0;
    std::vector<std::thread> threads;
    std::random_device rd;
    for(int i = 0; i < 10; i++) {
        threads.push_back(std::thread(fight_thread, player1, player2, sim1, sim2, tuple, game_count / 10, rd()));
    }
    for (auto& th : threads) {
        th.join();
    }

    std::streamsize ss = std::cout.precision();
    std::cout << "Playing " << game_count << " episodes: \n";
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "Black: " << fight_black_win * 100.0 / (fight_black_win + fight_white_win)
              << " %" << std::endl;
    std::cout << "White: " << fight_white_win * 100.0 / (fight_black_win + fight_white_win)
              << " %\n" << std::endl;
    std::cout.precision(ss);
}

int main(int argc, const char* argv[]) {
    std::cout << "Othello: ";
    std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
    std::cout << std::endl << std::endl;
    int game_count = 2000;
    std::string tuple_args;

    for (int i = 1; i < argc; i++) {
        std::string para(argv[i]);
        if (para.find("--game=") == 0) {
            game_count = std::stoi(para.substr(para.find("=") + 1));
        } else if (para.find("--tuple=") == 0) {
            tuple_args = para.substr(para.find("=") + 1);
        }
    }

    Tuple tuple(tuple_args);
    fight(0, 1, 0, 0, &tuple, game_count);
    fight(1, 0, 0, 0, &tuple, game_count);

    return 0;
}