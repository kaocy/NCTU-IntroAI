#include <bits/stdc++.h>
using namespace std;

const int MAX_SIZE = 100;
int board[MAX_SIZE][MAX_SIZE];
char c_board[MAX_SIZE][MAX_SIZE]; // for output solution
int constraints[MAX_SIZE][MAX_SIZE]; // for degree heuristic

const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

int board_size_x, board_size_y, mine_total, solution_num = 0;
bool forward_check;
int heuristic_type;

struct Node {
    vector<vector<int>> assignments;
    vector<vector<int>> domains;
    vector<pair<int, int>> unassigned;
    int current_mine_num;

    Node () {
        assignments.resize(board_size_x, vector<int>(board_size_y, 0));
        domains.resize(board_size_x, vector<int>(board_size_y, -1));
        unassigned.clear();
        current_mine_num = 0;
    }
    Node (const vector<vector<int>> &assignments, const vector<vector<int>> &domains, const vector<pair<int, int>> &unassigned, int current_mine_num)
        : assignments(assignments), domains(domains), unassigned(unassigned), current_mine_num(current_mine_num) {}
};

struct Degree_cmp {
    bool operator() (pair<int, int> a, pair<int, int> b) {
        return constraints[a.first][a.second] > constraints[b.first][b.second];
    }
};

bool is_outside(int x, int y) {
    return x < 0 || x >= board_size_x || y < 0 || y >= board_size_y;
}

void output(const vector<vector<int>> &assignments) {
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

void init_root(Node &node) {
    for (int x = 0; x < board_size_x; x++) {
        for (int y = 0; y < board_size_y; y++) {
            if (board[x][y] == -1) {
                node.assignments[x][y] = -1;
                node.unassigned.push_back(make_pair(x, y));
            }
        }
    }

    memset(constraints, 0, sizeof(constraints));
    for (int x = 0; x < board_size_x; x++) {
        for (int y = 0; y < board_size_y; y++) {
            if (board[x][y] != -1) continue;

            int current_domain = 0b11;
            for (int i = 0; i < 8; i++) {
                int center_x = x + dx[i];
                int center_y = y + dy[i];
                if (is_outside(center_x, center_y))   continue;
                if (board[center_x][center_y] == -1)  continue;

                constraints[x][y]++;

                int mine_num = 0;
                int space_num = 0;
                for (int j = 0; j < 8; j++) {
                    int outer_x = center_x + dx[j];
                    int outer_y = center_y + dy[j];
                    if (is_outside(outer_x, outer_y))   continue;

                    if (board[outer_x][outer_y] == -1) {
                        if (node.assignments[outer_x][outer_y] == 1)     mine_num++;
                        if (node.assignments[outer_x][outer_y] == -1)    space_num++;
                    }
                }

                int mine_need = board[center_x][center_y] - mine_num;
                if (mine_need == 0) current_domain &= 0b01;
                else if (mine_need == space_num) current_domain &= 0b10;
                else if (mine_need < space_num) current_domain &= 0b11;
                else if (mine_need > space_num) current_domain &= 0b00;

                if (current_domain == 0b00) break;
            }
            node.domains[x][y] = current_domain;
        }
    }

    if (heuristic_type == 2)    sort(node.unassigned.begin(), node.unassigned.end(), Degree_cmp());
}

void backtrack_search(const Node &root) {
    int num_expand = 0;
    stack<Node> frontier;
    frontier.push(root);

    while (!frontier.empty()) {
        Node node = frontier.top(); frontier.pop();
        if (node.unassigned.empty()) {
            output(node.assignments);
            continue;
        }

        num_expand++;
        pair<int, int> variable = node.unassigned.front();

        int x = variable.first, y = variable.second;
        int current_domain = node.domains[x][y];
        if (current_domain == 0)  continue;

        vector<int> value_bit{0, 1};

        if (heuristic_type == 3 && current_domain == 0b11) {
            int domain_affect[2] = {0, 0};
            for (int k : value_bit) {
                vector<vector<int>> assignments = node.assignments;
                vector<vector<int>> domains = node.domains;
                vector<pair<int, int>> unassigned;
                unassigned.assign(node.unassigned.begin() + 1, node.unassigned.end());
                int current_mine_num = node.current_mine_num + k;
                assignments[x][y] = k;

                bool not_satisfied = false; // forward checking

                for (int i = 0; i < 8; i++) {
                    int center_x = x + dx[i];
                    int center_y = y + dy[i];
                    if (is_outside(center_x, center_y))   continue;
                    if (board[center_x][center_y] == -1)  continue;

                    int mine_num = 0, space_num = 0;
                    for (int j = 0; j < 8; j++) {
                        int outer_x = center_x + dx[j];
                        int outer_y = center_y + dy[j];
                        if (is_outside(outer_x, outer_y))   continue;

                        if (board[outer_x][outer_y] == -1) {
                            if (assignments[outer_x][outer_y] == 1)    mine_num++;
                            if (assignments[outer_x][outer_y] == -1)   space_num++;
                        }
                    }
                    int mine_need = board[center_x][center_y] - mine_num;
                    int update_domain; 
                    if (mine_need == 0)              update_domain = 0b01;
                    else if (mine_need == space_num) update_domain = 0b10;
                    else if (mine_need < space_num)  update_domain = 0b11;
                    else if (mine_need > space_num)  update_domain = 0b00;

                    for (int j = 0; j < 8; j++) {
                        int outer_x = center_x + dx[j];
                        int outer_y = center_y + dy[j];
                        if (is_outside(outer_x, outer_y))   continue;

                        if (board[outer_x][outer_y] == -1 && assignments[outer_x][outer_y] == -1) {
                            domain_affect[k] += __builtin_popcount(domains[outer_x][outer_y]);
                            domains[outer_x][outer_y] &= update_domain;
                            domain_affect[k] -= __builtin_popcount(domains[outer_x][outer_y]);
                            if (domains[outer_x][outer_y] == 0) {
                                not_satisfied = true;
                            }
                        }
                    }
                }
                if (not_satisfied) domain_affect[k] = INT_MAX;
            }
            if (domain_affect[0] > domain_affect[1]) {
                swap(value_bit[0], value_bit[1]);
            }
        }

        for (int k : value_bit) {
            if ((current_domain & (1 << k)) == 0)   continue;

            vector<vector<int>> assignments = node.assignments;
            vector<vector<int>> domains = node.domains;
            vector<pair<int, int>> unassigned;
            unassigned.assign(node.unassigned.begin() + 1, node.unassigned.end());
            int current_mine_num = node.current_mine_num + k;
            assignments[x][y] = k;

            bool not_satisfied = false; // forward checking

            for (int i = 0; i < 8; i++) {
                int center_x = x + dx[i];
                int center_y = y + dy[i];
                if (is_outside(center_x, center_y))   continue;
                if (board[center_x][center_y] == -1)  continue;

                int mine_num = 0, space_num = 0;
                for (int j = 0; j < 8; j++) {
                    int outer_x = center_x + dx[j];
                    int outer_y = center_y + dy[j];
                    if (is_outside(outer_x, outer_y))   continue;

                    if (board[outer_x][outer_y] == -1) {
                        if (assignments[outer_x][outer_y] == 1)    mine_num++;
                        if (assignments[outer_x][outer_y] == -1)   space_num++;
                    }
                }
                int mine_need = board[center_x][center_y] - mine_num;
                int update_domain; 
                if (mine_need == 0)              update_domain = 0b01;
                else if (mine_need == space_num) update_domain = 0b10;
                else if (mine_need < space_num)  update_domain = 0b11;
                else if (mine_need > space_num)  update_domain = 0b00;

                for (int j = 0; j < 8; j++) {
                    int outer_x = center_x + dx[j];
                    int outer_y = center_y + dy[j];
                    if (is_outside(outer_x, outer_y))   continue;

                    if (board[outer_x][outer_y] == -1 && assignments[outer_x][outer_y] == -1) {
                        domains[outer_x][outer_y] &= update_domain;
                        if (forward_check && domains[outer_x][outer_y] == 0) {
                            not_satisfied = true;
                        }
                    }
                }
            }

            // return earlier if constraint will not be satisfied
            if (forward_check) {
                if (not_satisfied)  continue;
                int low_bound = 0, upp_bound = 0;
                for (const auto &variable : unassigned) {
                    int vx = variable.first, vy = variable.second;
                    if (domains[vx][vy] & 0b10)  upp_bound++;
                    if (domains[vx][vy] == 0b10) low_bound++;
                }
                // check constraint of total mine number 
                if ((low_bound + current_mine_num > mine_total) ||
                    (upp_bound + current_mine_num < mine_total)) continue;
            }

            if (heuristic_type == 1) {
                int min_legal_value = 3, index = 0, len = unassigned.size();
                for (int i = 0; i < len; i++) {
                    int vx = unassigned[i].first, vy = unassigned[i].second;
                    int bits = __builtin_popcount(domains[vx][vy]);
                    if (bits < min_legal_value) {
                        min_legal_value = bits;
                        index = i;
                    }
                }
                if (index != 0) swap(unassigned[0], unassigned[index]);
            }

            frontier.push(Node{assignments, domains, unassigned, current_mine_num});
        }
    }

    cout << "Number of solutions: " << solution_num << "\n";
    cout << "Number of expanded nodes: " << num_expand << "\n";
}

int main(int argc, char **argv) { // 8 2 9 2
    if (argc > 1) forward_check = (atoi(argv[1]) == 1);
    if (argc > 2) heuristic_type = atoi(argv[2]);
    else          heuristic_type = 0;
    cout << "forward_check: " << forward_check << " heuristic: " << heuristic_type << "\n";
    cout << __builtin_popcount(4) << __builtin_popcount(31) << "\n";

    cin >> board_size_x >> board_size_y >> mine_total;
    Node root;
    for (int i = 0; i < board_size_x; i++) {
        for (int j = 0; j < board_size_y; j++) {
            cin >> board[i][j];
        }
    }
    init_root(root);

    auto t1 = std::chrono::high_resolution_clock::now();
    backtrack_search(root);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << duration << " us\n";
    return 0;
}