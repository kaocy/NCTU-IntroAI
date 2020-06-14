#pragma once
#include <algorithm>
#include <vector>
#include <random>
#include <unordered_map>
#include <iterator>
#include <unistd.h>
#include "board.h"
#include "action.h"
#include "utils.h"
#include "tuple.h"
// #include "mcts.h"

class Agent {
public:
    Agent() {}
    virtual ~Agent() {}
    virtual void open_episode() {}
    virtual void close_episode(const std::string& flag = "") {}
    virtual Action take_action(const Board& b) { return Action(); }
    virtual bool check_for_win(const Board& b) { return b.game_over(); }
};

class RandomAgent : public Agent {
public:
    RandomAgent() : Agent(), dis(0.0, 1.0) { engine.seed(rd()); }
    virtual ~RandomAgent() {}

protected:
    std::random_device rd;
    std::default_random_engine engine;
    std::uniform_real_distribution<> dis;
};

// class MCTSTrainingPlayer : public RandomAgent {
// public:
//     MCTSTrainingPlayer(unsigned color, Tuple *tuple, float epsilon = 0.9) :
//         RandomAgent(),
//         color(color),
//         tuple(tuple),
//         epsilon(epsilon) {}

//     std::string role() { return color ? "White" : "Black"; }

//     virtual void open_episode() {
//         record.clear();
//     }

//     virtual void close_episode(const std::string& flag = "") {
//         float result = std::stof(flag);
//         // the first record is done by black, then alter
//         for (Board i : record) {
//             tuple->train_weight(i, result, 0);
//             result *= -1;
//         }
//     }

// public:
//     // use MCTS in training
//     virtual Action take_action(const Board& before) {
//         Board tmp = Board(before);
//         MCTS mcts(tuple, true, 1600, rd(), epsilon);
//         std::pair<std::string, unsigned> prev_action = mcts.training(tmp, color, 1);
//         record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));

//         std::string type = prev_action.first;
//         unsigned code = prev_action.second;
//         if (type == "eat")  return Action::Eat(code, color);
//         if (type == "move") return Action::Move(code, color);

//         // cannot find valid action
//         return Action();
//     }

// private:
//     int color; // 0 for black or 1 for white
//     std::vector<Board> record;
//     Tuple *tuple;
//     float epsilon;
// };

class TupleTrainingPlayer : public RandomAgent {
public:
    TupleTrainingPlayer(unsigned color, Tuple *tuple, float epsilon = 0.9, uint32_t seed = 10) :
        RandomAgent(),
        color(color),
        tuple(tuple),
        epsilon(epsilon) { engine.seed(seed); }

    std::string role() { return color ? "White" : "Black"; }

    virtual void open_episode() {
        record.clear();
    }

    virtual void close_episode(const std::string& flag = "") {
        float result = std::stof(flag);
        result *= (color ? -1.0 : 1.0);
        for (Board b : record) {
            tuple->train_weight(b, result, 0);
        }
    }

    float get_epsilon() const {
        return epsilon;
    }

    void increase_epsilon() {
        if (epsilon < 0.94) epsilon += 0.05;
    }

public:
    // tuple with ϵ-greedy
    virtual Action take_action(const Board& board) {
        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, color);

        if (dis(engine) < epsilon) {
            float best_value = -1e9;
            unsigned best_code = 0;
            int best_action_type;

            // std::cout << "eat\n";
            for (unsigned code : eats) {
                Board tmp = Board(board);
                tmp.eat(code & 0b111111, color);
                float value = tuple->get_board_value(tmp, color);
                // std::cout << value << "\n";
                if (value > best_value) {
                    best_value = value;
                    best_code = code;
                    best_action_type = 0;
                }
            }
            // std::cout << "move\n";
            for (unsigned code : moves) {
                Board tmp = Board(board);
                tmp.move(code & 0b111111, color);
                float value = tuple->get_board_value(tmp, color);
                // std::cout << value << "\n";
                if (value > best_value) {
                    best_value = value;
                    best_code = code;
                    best_action_type = 1;
                }
            }

            if (best_code != 0) {
                if (!best_action_type) {
                    Board tmp = Board(board);
                    tmp.eat(best_code & 0b111111, color);
                    record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));
                    return Action::Eat(best_code & 0b111111, color);
                }
                else {
                    Board tmp = Board(board);
                    tmp.move(best_code & 0b111111, color);
                    record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));
                    return Action::Move(best_code & 0b111111, color);
                }
            }
        }
        else {
            std::shuffle(eats.begin(), eats.end(), engine);
            std::shuffle(moves.begin(), moves.end(), engine);
            int size1 = eats.size(), size2 = moves.size();

            if (dis(engine) * (size1 + size2) < size1) {  // eat seems to be TOO important
                if (eats.size() > 0) {
                    Board tmp = Board(board);
                    tmp.eat(eats[0] & 0b111111, color);
                    record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));
                    return Action::Eat(eats[0] & 0b111111, color);
                }
            }
            else {
                if (moves.size() > 0) {
                    Board tmp = Board(board);
                    tmp.move(moves[0] & 0b111111, color);
                    record.emplace_back(tmp.get_board(0 ^ color), tmp.get_board(1 ^ color));
                    return Action::Move(moves[0] & 0b111111, color);
                }
            }
        }

        // cannot find valid action
        return Action();
    }

private:
    int color; // 0 for black or 1 for white
    std::vector<Board> record;
    Tuple *tuple;
    float epsilon;
};

class TupleRolloutPlayer : public RandomAgent {
public:
    TupleRolloutPlayer(Tuple *tuple) : RandomAgent(), tuple(tuple) {}

public:
    // choose best action with tuple value
    void playing(Board &board, int color) {
        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, color);

        float best_value = -1e9;
        unsigned best_code = 0;
        int best_action_type;

        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, color);
            float value = tuple->get_board_value(tmp, color);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 0;
            }
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, color);
            float value = tuple->get_board_value(tmp, color);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 1;
            }
        }

        if (best_code != 0) {
            if (!best_action_type) 
                board.eat(best_code & 0b111111, color);
            else
                board.move(best_code & 0b111111, color);
        }
    }

private:
    Tuple *tuple;
};

class HeuristicRolloutPlayer : public RandomAgent {
public:
    HeuristicRolloutPlayer() : RandomAgent() {}

public:
    // choose best action with heuristic value
    void playing(Board &board, int color) {
        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, color);

        float best_value = -1e9;
        unsigned best_code = 0;
        int best_action_type;

        for (unsigned code : eats) {
            Board tmp = Board(board);
            tmp.eat(code & 0b111111, color);
            float value = get_board_heuristic(tmp, color);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 0;
            }
        }
        for (unsigned code : moves) {
            Board tmp = Board(board);
            tmp.move(code & 0b111111, color);
            float value = get_board_heuristic(tmp, color);
            if (value > best_value) {
                best_value = value;
                best_code = code;
                best_action_type = 1;
            }
        }

        if (best_code != 0) {
            if (!best_action_type) 
                board.eat(best_code & 0b111111, color);
            else
                board.move(best_code & 0b111111, color);
        }
    }

    float get_board_heuristic(const Board &board, int color) const {
        Board::data black = board.get_board(0 ^ color);
        Board::data white = board.get_board(1 ^ color);

        float value = 0.0;
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                Board::data x = (1ULL << (i * 8 + j));
                if (black & x)       value += heuristic[i][j];
                else if (white & x)  value -= heuristic[i][j];
            }
        }
        return value;
    }

private:
    // const float heuristic[8][8] = {{  1.00, -0.25, 0.10, 0.05, 0.05, 0.10, -0.25,  1.00 },
    //                                { -0.25, -0.25, 0.01, 0.01, 0.01, 0.01, -0.25, -0.25 },
    //                                {  0.10,  0.01, 0.05, 0.02, 0.02, 0.05,  0.01,  0.10 },
    //                                {  0.05,  0.01, 0.02, 0.01, 0.01, 0.02,  0.01,  0.05 },
    //                                {  0.05,  0.01, 0.02, 0.01, 0.01, 0.02,  0.01,  0.05 },
    //                                {  0.10,  0.01, 0.05, 0.02, 0.02, 0.05,  0.01,  0.10 },
    //                                { -0.25, -0.25, 0.01, 0.01, 0.01, 0.01, -0.25, -0.25 },
    //                                {  1.00, -0.25, 0.10, 0.05, 0.05, 0.10, -0.25,  1.00 }};
    const float heuristic[8][8] = {{  4.0, -3.0,  2.0,  2.0,  2.0,  2.0, -3.0,  4.0 },
                                   { -3.0, -4.0, -1.0, -1.0, -1.0, -1.0, -4.0, -3.0 },
                                   {  2.0, -1.0,  1.0,  0.0,  0.0,  1.0, -1.0,  2.0 },
                                   {  2.0, -1.0,  0.0,  1.0,  1.0,  0.0, -1.0,  2.0 },
                                   {  2.0, -1.0,  0.0,  1.0,  1.0,  0.0, -1.0,  2.0 },
                                   {  2.0, -1.0,  1.0,  0.0,  0.0,  1.0, -1.0,  2.0 },
                                   { -3.0, -4.0, -1.0, -1.0, -1.0, -1.0, -4.0, -3.0 },
                                   {  4.0, -3.0,  2.0,  2.0,  2.0,  2.0, -3.0,  4.0 },};
};

class GreedyRolloutPlayer : public RandomAgent {
public:
    GreedyRolloutPlayer() : RandomAgent() {}
    GreedyRolloutPlayer(int seed) : RandomAgent() { engine.seed(seed); }

public:
    void playing(Board &board, int color) {
        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, color);
        std::shuffle(eats.begin(), eats.end(), engine);
        std::shuffle(moves.begin(), moves.end(), engine);

        if (eats.size() > 0) {
            board.eat(eats[0] & 0b111111, color);
        }
        else if (moves.size() > 0) {
            board.move(moves[0] & 0b111111, color);
        }
    }
};

class RandomRolloutPlayer : public RandomAgent {
public:
    RandomRolloutPlayer() : RandomAgent() {}
    RandomRolloutPlayer(int seed) : RandomAgent() { engine.seed(seed); }

public:
    void playing(Board &board, int color) {
        std::vector<unsigned> eats, moves;
        board.get_possible_action(eats, moves, color);
        
        int size1 = eats.size(), size2 = moves.size();
        if (dis(engine) * (size1 + size2) < size1) {
            if (eats.size() > 0) {
                board.eat(eats[0] & 0b111111, color);
            }
        }
        else {
            if (moves.size() > 0) {
                board.move(moves[0] & 0b111111, color);
            }
        }
    }
};
