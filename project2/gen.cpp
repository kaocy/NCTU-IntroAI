#include <bits/stdc++.h>
using namespace std;

int board[100][100];
const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

int main() {
    int x, y, mine_total, hint_total;
    cin >> x >> y >> mine_total >> hint_total;
    memset(board, -1, sizeof(board));

    std::random_device rd;
    std::default_random_engine gen = std::default_random_engine(rd());
    std::uniform_int_distribution<int> dis1(0, x - 1);
    std::uniform_int_distribution<int> dis2(0, y - 1);

    int mine_num = 0;
    while (mine_num < mine_total) {
        int px = dis1(gen), py = dis2(gen);
        if (board[px][py] == -2) continue;
        board[px][py] = -2;
        mine_num++;
    }

    int hint_num = 0;
    while (hint_num < hint_total) {
        int px = dis1(gen), py = dis2(gen);
        if (board[px][py] >= 0 || board[px][py] == -2) continue;

        int hint = 0;
        for (int i = 0; i < 8; i++) {
            int outer_x = px + dx[i];
            int outer_y = py + dy[i];
            if (outer_x < 0 || outer_x >= x || outer_y < 0 || outer_y >=y)   continue;
            if (board[outer_x][outer_y] == -2)  hint++;
        }
        board[px][py] = hint;
        hint_num++;
    }

    cout << x << " " << y << " " << mine_total << "\n";
    for (int i = 0; i < x; i++) {
        for (int j = 0; j < y; j++) {
            if (board[i][j] < 0)    cout << "-1 ";
            else                    cout << board[i][j] << " ";
            // cout << board[i][j] << " ";
        }
        cout << "\n";
    }
}