#include <bits/stdc++.h>
using namespace std;

int board_size_x, board_size_y, mine_total, solution_num = 0;
int board[100][100];
char c_board[100][100];
const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

struct Domain {
    int x, y, value;
    Domain(int x, int y, int value = 0b11) : x(x), y(y), value(value) {}
};

struct Node {
    vector<vector<int>> assignments;
    list<Domain> domains;

    Node () {
        assignments.resize(board_size_x, vector<int>(board_size_y, 0));
        domains.clear();
    }
    Node (const vector<vector<int>>& assignments, const list<Domain>& domains)
        : assignments(assignments), domains(domains) {}
};

bool is_outside(int x, int y) {
    return x < 0 || x >= board_size_x || y < 0 || y >= board_size_y;
}

void output(const vector<vector<int>>& assignments) {
    int mine_num = 0;
    for (int i = 0; i < board_size_x; i++) {
        for (int j = 0; j < board_size_y; j++) {
            if (board[i][j] == -1) {
                if (assignments[i][j] == 0)   c_board[i][j] = '-';
                if (assignments[i][j] == 1)   c_board[i][j] = '*', mine_num++;
            }
            else {
                c_board[i][j] = char('0' + board[i][j]);
            }
        }
    }
    if (mine_num != mine_total) return ;

    cout << "Solution " << ++solution_num << ":\n";
    for (int i = 0; i < board_size_x; i++) {
        for (int j = 0; j < board_size_y; j++) {
            cout << c_board[i][j] << " ";
        }
        cout << "\n";
    }
    cout << "======================\n";
}

void backtrack_search(const Node& root) {
    int num_expand = 0;
    stack<Node> frontier;
    frontier.push(root);

    while (!frontier.empty()) {
        Node node = frontier.top(); frontier.pop();
        if (node.domains.empty()) {
            output(node.assignments);
            continue;
        }

        num_expand++;
        Domain domain = node.domains.front();
        node.domains.pop_front();
        int current_domain = 0b11;
        for (int i = 0; i < 8; i++) {
            int center_x = domain.x + dx[i];
            int center_y = domain.y + dy[i];
            if (is_outside(center_x, center_y))   continue;
            if (board[center_x][center_y] == -1)  continue;

            // cout << center_x << " " << center_y << "\n";

            int mine_num = 0;
            int space_num = 0;
            for (int j = 0; j < 8; j++) {
                int outer_x = center_x + dx[j];
                int outer_y = center_y + dy[j];
                if (is_outside(outer_x, outer_y))   continue;

                if (board[outer_x][outer_y] == -1) {
                    if (node.assignments[outer_x][outer_y] == 1)    mine_num++;
                    if (node.assignments[outer_x][outer_y] == -1)   space_num++;
                }
            }
            int mine_need = board[center_x][center_y] - mine_num;
            // cout << board[center_x][center_y] << " " << mine_num << " " << space_num << "\n"; 
            if (mine_need == 0) current_domain &= 0b01;
            else if (mine_need == space_num) current_domain &= 0b10;
            else if (mine_need < space_num) current_domain &= 0b11;
            else if (mine_need > space_num) current_domain &= 0b00;

            if (current_domain == 0b00) break;
        }

        vector<vector<int>> assignments = node.assignments;
        list<Domain> domains = node.domains;
        if (current_domain & 0b10) {
            assignments[domain.x][domain.y] = 1;
            frontier.push(Node{assignments, domains});
        }
        if (current_domain & 0b01) {
            assignments[domain.x][domain.y] = 0;
            frontier.push(Node{assignments, domains});
        }
    }
    cout << "Number of solutions: " << solution_num << "\n";
    cout << "Number of expanded nodes: " << num_expand << "\n";
}

int main(int argc, char **argv) {
    while (cin >> board_size_x >> board_size_y >> mine_total) {
        Node root;
        for (int i = 0; i < board_size_x; i++) {
            for (int j = 0; j < board_size_y; j++) {
                cin >> board[i][j];
                if (board[i][j] == -1) {
                    root.assignments[i][j] = -1;
                    root.domains.emplace_back(i, j, 0b11);
                }
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        backtrack_search(root);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        cout << duration << " us\n";
    }
    return 0;
}