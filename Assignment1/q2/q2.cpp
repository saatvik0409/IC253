#include <bits/stdc++.h>
using namespace std;

struct Node {
    Node* right;
    Node* left;
    Node* up;
    Node* down;
    Node* next;

    pair<int,int> cell_position;

    Node(int i, int j) {
        right = left = up = down = next = nullptr;
        cell_position = {i, j};
    }
};

/* ================= READ PGM ================= */
vector<vector<int>> readPGM(const string& filename, int& width, int& height, int& maxval) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot open image file\n";
        exit(1);
    }

    string magic;
    file >> magic;
    if (magic != "P2" && magic != "P5") {
        cerr << "Unsupported PGM format\n";
        exit(1);
    }

    file >> width >> height >> maxval;
    file.ignore();

    vector<vector<int>> image(height, vector<int>(width, 0));

    if (magic == "P2") {
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++) {
                int pixel;
                file >> pixel;
                image[i][j] = (pixel == maxval ? 0 : 1);
            }
    } else { // P5
        for (int i = 0; i < height; i++)
            for (int j = 0; j < width; j++) {
                unsigned char pixel;
                file.read(reinterpret_cast<char*>(&pixel), 1);
                image[i][j] = (pixel == maxval ? 0 : 1);
            }
    }

    return image;
}

/* ================= WRITE PGM ================= */
void writePGM(const string& filename,
              const vector<vector<int>>& image,
              int width, int height, int maxval) {

    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Cannot write output image\n";
        exit(1);
    }

    file << "P2\n";
    file << width << " " << height << "\n";
    file << maxval << "\n";

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            file << (image[i][j] ? maxval : 0) << " ";
        }
        file << "\n";
    }
}

/* ================= SPARSE GRAPH ================= */
Node* convertToLinkedList(vector<vector<int>>& matrix,
                          vector<vector<Node*>>& node_matrix) {

    int m = matrix.size();
    int n = matrix[0].size();

    node_matrix.assign(m, vector<Node*>(n, nullptr));

    Node* head = nullptr;
    Node* tail = nullptr;

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (matrix[i][j] == 1) {
                Node* node = new Node(i, j);
                node_matrix[i][j] = node;

                if (!head) head = tail = node;
                else {
                    tail->next = node;
                    tail = node;
                }
            }
        }
    }

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            if (!node_matrix[i][j]) continue;

            for (int k = 0; k < 4; k++) {
                int x = i + dx[k];
                int y = j + dy[k];

                if (x >= 0 && y >= 0 && x < m && y < n &&
                    node_matrix[x][y]) {

                    if (dx[k] == 1)  node_matrix[i][j]->down  = node_matrix[x][y];
                    if (dx[k] == -1) node_matrix[i][j]->up    = node_matrix[x][y];
                    if (dy[k] == 1)  node_matrix[i][j]->right = node_matrix[x][y];
                    if (dy[k] == -1) node_matrix[i][j]->left  = node_matrix[x][y];
                }
            }
        }
    }

    return head;
}

/* ================= CONNECTED COMPONENTS ================= */
void dfs(Node* node,
         unordered_set<Node*>& visited,
         int& area,
         vector<Node*>& boundary) {

    visited.insert(node);
    area++;

    bool isBoundary = false;

    Node* neighbors[4] = {
        node->up, node->down, node->left, node->right
    };

    for (Node* nbr : neighbors) {
        if (!nbr) isBoundary = true;
        else if (!visited.count(nbr))
            dfs(nbr, visited, area, boundary);
    }

    if (isBoundary)
        boundary.push_back(node);
}

void findConnectedComponents(Node* head) {
    unordered_set<Node*> visited;
    int component = 0;

    for (Node* curr = head; curr; curr = curr->next) {
        if (!visited.count(curr)) {
            component++;
            int area = 0;
            vector<Node*> boundary;

            dfs(curr, visited, area, boundary);

            cout << "Object " << component << "\n";
            cout << "Area: " << area << "\n\n";

            cout << "Boundary pixels: ";
            for (auto* b : boundary) {
                cout << "(" << b->cell_position.first
                    << "," << b->cell_position.second << ") ";
            }
            cout << "\n\n";

        }
    }

    cout << "Total objects detected = " << component << "\n";
}

/* ================= FLIP IMAGE ================= */
vector<vector<int>> flipImage(const vector<vector<int>>& original) {
    int h = original.size();
    int w = original[0].size();

    vector<vector<int>> flipped(h, vector<int>(w, 0));
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            flipped[i][j] = original[i][j] ? 1 : 0;

    return flipped;
}

/* ================= MAIN ================= */
int main() {
    string inputFile = "input_image.pgm";
    string outputFile = "flipped_image.pgm";

    int width, height, maxval;
    vector<vector<int>> matrix = readPGM(inputFile, width, height, maxval);

    vector<vector<Node*>> node_matrix;
    Node* head = convertToLinkedList(matrix, node_matrix);

    findConnectedComponents(head);

    vector<vector<int>> flipped = flipImage(matrix);
    writePGM(outputFile, flipped, width, height, maxval);

    cout << "Flipped image saved as " << outputFile << "\n";

    return 0;
}
