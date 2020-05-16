#include <bits/stdc++.h>
using namespace std;

const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

typedef pair<int, int> Coordinate;

class Control {
public:
    Control(string level) {
        if (level == "Easy") {
            board_size_x = 9;
            board_size_y = 9;
            num_mines = 10;
        }
        if (level == "Medium") {
            board_size_x = 16;
            board_size_y = 16;
            num_mines = 25;
        }
        if (level == "Hard") {
            board_size_x = 30;
            board_size_y = 16;
            num_mines = 99;
        }
    }

    void initialize_board() {
        std::random_device rd;
        std::default_random_engine gen = std::default_random_engine(rd());
        std::uniform_int_distribution<int> dis1(0, board_size_x - 1);
        std::uniform_int_distribution<int> dis2(0, board_size_y - 1);

        memset(mines, false, sizeof(mines));

        int num = 0;
        while (num < num_mines) {
            int px = dis1(gen), py = dis2(gen);
            if (mines[px][py]) continue;
            mines[px][py] = true;
            num++;
        }

        for (int x = 0; x < board_size_x; x++) {
            for (int y = 0; y < board_size_y; y++) {
                if (mines[x][y]) {
                    hints[x][y] = -1;
                    continue;
                }

                int hint = 0;
                for (int i = 0; i < 8; i++) {
                    int outer_x = x + dx[i];
                    int outer_y = y + dy[i];
                    if (is_outside(outer_x, outer_y))   continue;
                    if (mines[outer_x][outer_y])  hint++;
                }
                hints[x][y] = hint;
            }
        }

        // for (int x = 0; x < board_size_x; x++) {
        //     for (int y = 0; y < board_size_y; y++) {
        //         cout << mines[x][y] << " ";
        //     }
        //     cout << "\n";
        // }
        // cout << "\n";
        // for (int x = 0; x < board_size_x; x++) {
        //     for (int y = 0; y < board_size_y; y++) {
        //         cout << hints[x][y] << " ";
        //     }
        //     cout << "\n";
        // }
    }

    int get_hint(int x, int y) {
        return hints[x][y];
    }

    vector<Coordinate> get_initial_safe_cells() {
        std::random_device rd;
        std::default_random_engine gen = std::default_random_engine(rd());
        std::uniform_int_distribution<int> dis1(0, board_size_x - 1);
        std::uniform_int_distribution<int> dis2(0, board_size_y - 1);

        memset(safe, false, sizeof(safe));
        vector<Coordinate> safe_cells;
        int num_safe_cells = int(round(sqrt(board_size_x * board_size_y)));

        int num = 0;
        while (num < num_safe_cells) {
            int px = dis1(gen), py = dis2(gen);
            if (mines[px][py] || safe[px][py]) continue;
            safe_cells.push_back(make_pair(px, py));
            safe[px][py] = true;
            num++;
        }

        // for (auto &cells : safe_cells) {
        //     cout << cells.first << " " << cells.second << "\n";
        // }

        return safe_cells;
    }

private:
    bool is_outside(int x, int y) {
        return x < 0 || x >= board_size_x || y < 0 || y >= board_size_y;
    }

private:
    int board_size_x, board_size_y;
    int num_mines;
    bool mines[100][100];
    int hints[100][100];
    bool safe[100][100];
};

class Game {
public:
    Game(string level) : control(level) {}

    void start() {
        control.initialize_board();
        auto cells = control.get_initial_safe_cells();
    }

private:
    Control control;
};

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Usage: ./minesweeper [level]\n";
        exit(EXIT_FAILURE);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    Game game(argv[1]);
    game.start();
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << float(duration) / 1000.0 << " ms\n";
    return 0;
}