#include <iostream>
#include <fstream>
#include <map>
#include <queue>

using namespace std;

class Node { 
public:
    char key;
    int amount;
    Node* right;
    Node* left;

    bool operator()(const Node& x, const Node& y) {
        return x.amount > y.amount;
    }

    Node(char k = '\0', int cast = 0, Node* l = NULL, Node* r = NULL) {
        key = k;
        amount = cast;
        left = l;
        right = r;
    }
};

Node* maketree(priority_queue<Node, vector<Node>, Node> leafs) {
    while (leafs.size() > 1) {
        Node left = leafs.top(); leafs.pop();
        Node right = leafs.top(); leafs.pop();
        Node* node = new Node('\0', left.amount + right.amount, new Node(left), new Node(right));
        leafs.push(*node);
    }
    return new Node(leafs.top());
}

void huffman(Node*& root, string code, map<char, string>& table) {
    if (!root) return;
    if (!root->left && !root->right) table[root->key] = code;
    huffman(root->left, code + "0", table);
    huffman(root->right, code + "1", table);
}

void decode(string file1 = "coded.txt", string file2 = "output.txt") {
    ifstream in(file1, ios::binary);
    if (!in) { 
        puts("Input file doesn't exist"); 
        return; 
    }

    char x;
    in.get(x);
    map<char, int> freq;

    for (int i = 0; i <= x; i++) {
        char y;
        int z;
        in.get(y);
        in.read((char*)&z, sizeof(z));
        freq[y] = z;
    }

    priority_queue<Node, vector<Node>, Node> pq;
    for (auto pair : freq) pq.push(Node(pair.first, pair.second));

    Node* root = maketree(pq);

    ofstream out(file2, ios::binary);

    Node* temp = root;
    char count = 0;
    while (in.get(x)) {
        for (int i = 0; i < 8; i++) {
            bool b = x & (1 << (7 - i));
            if (b) temp = temp->right;
            else if (!b) temp = temp->left;
            if (!temp->left && !temp->right) {
                out << temp->key;
                temp = root;
            }
        }
    }

    in.close();
    out.close();
}
void check(string file1 = "input.txt", string file2 = "output.txt") {
    ifstream uncoded(file1, ios::binary);
    ifstream decoded(file2, ios::binary);

    char a, b; unsigned int equal = 0;
    while (uncoded.get(a) && decoded.get(b))
        if (a != b) equal++;

    while (uncoded.get(a)) equal++;
    while (decoded.get(b)) equal++;
    if (equal) cout << "uncoded != decoded" << endl;
    else cout << "uncoded == decoded" << endl;

    uncoded.close();
    decoded.close();
}

int main() {
    decode();
    check();
}