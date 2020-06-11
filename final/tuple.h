#pragma once
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <vector>
#include "board.h"
#include "weight.h"

class Tuple {
public:
    Tuple(const std::string& args = "") : learning_rate(0.0001f) {
        std::stringstream ss(args);
        for (std::string pair; ss >> pair; ) {
            std::string key = pair.substr(0, pair.find('='));
            std::string value = pair.substr(pair.find('=') + 1);
            meta[key] = { value };
        }
        if (meta.find("alpha") != meta.end())
            learning_rate = float(meta["alpha"]);
        if (meta.find("load") != meta.end()) // pass load=... to load from a specific file
            load_weights(meta["load"]);
        else
            init_weight();
    }
    ~Tuple() {
        if (meta.find("save") != meta.end()) // pass save=... to save to a specific file
            save_weights(meta["save"]);
    }

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

    void save_weights(const std::string& path) {
        std::ofstream out(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!out.is_open()) std::exit(-1);
        uint32_t size = tuple_normal.size() + tuple_border.size();
        out.write(reinterpret_cast<char*>(&size), sizeof(size));

        for (Weight& w : tuple_normal) out << w;
        for (Weight& w : tuple_border) out << w;
        out.close();
    }

public:
    /**
     * 0: states in one game
     * 1: states from MCTS nodes
     */
    void train_weight(const Board &b, float result, int source = 0) {
        if (source == 0)      set_board_value(b, result, learning_rate);
        else if (source == 1) set_board_value(b, result, learning_rate * 0.05f);
    }

public:
    float get_board_value(const Board &board, const int color) {  // 0 black 1 white
        Board b(board.get_board(0 ^ color), board.get_board(1 ^ color));
        float value = 0.0f;

        for (int i = 4; i > 0; i--) {
            for (int i = 0; i < 13; i++) {
                uint32_t index = board_to_tuple_index(b, oblique[i]);
                value += tuple_normal[0][index];
                // std::cout << std::hex << index << "   " << std::dec << tuple_normal[0][index] << "\n";
            }
            // std::cout << "--------------\n";
            for (int i = 0; i < 6; i++) {
                uint32_t index = board_to_tuple_index(b, straight[i]);
                value += tuple_normal[0][index];
                // std::cout << std::hex << index << "   " << std::dec << tuple_normal[0][index] << "\n";
            }
            // std::cout << "--------------\n";
            for (int i = 0; i < 2; i++) {
                uint32_t index = board_to_tuple_index(b, border[i]);
                value += tuple_border[0][index];
                // std::cout << std::hex << index << "   " << std::dec << tuple_border[0][index] << "\n";
            }
            b.rotate(1);
        }
        // std::cout << "========  " << value << "   " << value / 84.0f << "  ========\n";
        return value / 84.0f;
    }

    void set_board_value(const Board &board, float value, float alpha) {
        Board b(board);

        for (int k = 4; k > 0; k--) {
            for (int i = 0; i < 13; i++) {
                uint32_t index = board_to_tuple_index(b, oblique[i]);
                tuple_normal[0][index] += alpha * (value - tuple_normal[0][index]);
            }
            for (int i = 0; i < 6; i++) {
                uint32_t index = board_to_tuple_index(b, straight[i]);
                tuple_normal[0][index] += alpha * (value - tuple_normal[0][index]);
            }
            for (int i = 0; i < 2; i++) {
                uint32_t index = board_to_tuple_index(b, border[i]);
                tuple_border[0][index] += alpha * (value - tuple_border[0][index]);
            }
            b.rotate(1);
        }
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
                                {  7, 14, 21, 28, 35, 42, 49, 56 },
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
    float learning_rate;
};