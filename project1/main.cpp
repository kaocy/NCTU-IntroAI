#include <bits/stdc++.h>
using namespace std;

struct Node {
    int x, y, layer;
    Node() : x(-1), y(-1) {}
    Node(int x, int y) : x(x), y(y) {}
};

const int BOARD_SIZE = 8;
const int MAX_LAYER = BOARD_SIZE * BOARD_SIZE;
bool explored[BOARD_SIZE][BOARD_SIZE];
Node parent[BOARD_SIZE][BOARD_SIZE];
int layer[BOARD_SIZE][BOARD_SIZE];

const int dx[8] = {-2, -1, 1, 2, -2, -1, 1, 2};
const int dy[8] = {1, 2, 2, 1, -1, -2, -2, -1};

bool is_outside(int x, int y) {
    return x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE;
}

void output(const Node& leaf, int num_expand) {
    vector<string> points;
    Node node = leaf;

    points.clear();
    while (node.x != -1 && node.y != -1) {
        string str = "(" + to_string(node.x) + "," + to_string(node.y) + ")";
        points.push_back(str);
        node = parent[node.x][node.y];
    }
    reverse(points.begin(), points.end());

    cout << "path: ";
    for (const string& point : points) cout << point << " ";
    cout << "\n\n";
    cout << "Total " << points.size() - 1 << " steps\n";
    cout << "Total " << num_expand << " expanded nodes\n";
}

int BFS(const Node& start, const Node& goal) {
    printf("Using BFS, from (%d,%d) to (%d,%d)\n\n", start.x, start.y, goal.x, goal.y);
    
    int num_expand = 0;
    queue<Node> frontier;
    frontier.push(start);
    memset(explored, false, sizeof(explored));
    parent[start.x][start.y] = Node{-1, -1};

    while (!frontier.empty()) {
        Node node = frontier.front(); frontier.pop();
        if (explored[node.x][node.y]) continue;
        if (node.x == goal.x && node.y == goal.y) {
            output(node, num_expand);
            break;
        }

        explored[node.x][node.y] = true;
        num_expand++;
        for (int i = 0; i < 8; i++) {
            int x = node.x + dx[i];
            int y = node.y + dy[i];
            if (is_outside(x, y))   continue;
            if (explored[x][y])     continue;
            frontier.push(Node{x, y});
            parent[x][y] = node;
        }
    }
    return num_expand;
}

int DFS(const Node& start, const Node& goal) {
    printf("Using DFS, from (%d,%d) to (%d,%d)\n\n", start.x, start.y, goal.x, goal.y);

    int num_expand = 0;
    stack<Node> frontier;
    frontier.push(start);
    memset(explored, false, sizeof(explored));
    parent[start.x][start.y] = Node{-1, -1};

    while (!frontier.empty()) {
        Node node = frontier.top(); frontier.pop();
        if (explored[node.x][node.y]) continue;
        if (node.x == goal.x && node.y == goal.y) {
            output(node, num_expand);
            break;
        }

        explored[node.x][node.y] = true;
        num_expand++;
        for (int i = 0; i < 8; i++) {
            int x = node.x + dx[i];
            int y = node.y + dy[i];
            if (is_outside(x, y))   continue;
            if (explored[x][y])     continue;
            frontier.push(Node{x, y});
            parent[x][y] = node;
        }
    }
    return num_expand;
}

int IDS(const Node& start, const Node& goal) {
    printf("Using IDS, from (%d,%d) to (%d,%d)\n\n", start.x, start.y, goal.x, goal.y);

    int num_expand = 0;
    for (int l = 0; l < MAX_LAYER; l++) {
        stack<Node> frontier;
        frontier.push(start);
        memset(explored, false, sizeof(explored));
        parent[start.x][start.y] = Node{-1, -1};

        for (int i = 0; i < BOARD_SIZE; i++) for (int j = 0; j < BOARD_SIZE; j++)
            layer[i][j] = INT_MAX;
        layer[start.x][start.y] = 0;

        while (!frontier.empty()) {
            Node node = frontier.top(); frontier.pop();
            if (explored[node.x][node.y]) continue;

            if (node.x == goal.x && node.y == goal.y) {
                output(node, num_expand);
                return num_expand;
            }
            if (layer[node.x][node.y] == l)    continue;

            explored[node.x][node.y] = true;
            num_expand++;
            for (int i = 0; i < 8; i++) {
                int x = node.x + dx[i];
                int y = node.y + dy[i];
                if (is_outside(x, y))   continue;
                if (explored[x][y])     continue;
                frontier.push(Node{x, y});
                if (layer[node.x][node.y] + 1 < layer[x][y]) {
                    parent[x][y] = node;
                    layer[x][y] = layer[node.x][node.y] + 1;
                }
            }
        }
    }
    return num_expand;
}

// int A_star(const Node& start, const Node& goal) {
//     printf("Using A*, from (%d,%d) to (%d,%d)\n\n", start.x, start.y, goal.x, goal.y);
// }

// int IDA_star(const Node& start, const Node& goal) {
//     printf("Using IDA*, from (%d,%d) to (%d,%d)\n\n", start.x, start.y, goal.x, goal.y);
// }

int search(int type, int sx, int sy, int gx, int gy) {
    Node start(sx, sy);
    Node goal(gx, gy);

    if (type == 1) return BFS(start, goal);
    if (type == 2) return DFS(start, goal);
    if (type == 3) return IDS(start, goal);
    // if (type == 4) return A_star(start, goal);
    // if (type == 5) return IDA_star(start, goal);
    return 0;
}

int main(int argc, char **argv) {
    search(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
    return 0;
}