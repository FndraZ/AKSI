#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <sys/stat.h>

using namespace std;

class Node { //узел дерева
public:
    char key;
    int amount;
    Node* right;
    Node* left;

    bool operator()(const Node& x, const Node& y) {
        return x.amount > y.amount;
    }

    Node(char k = '\0', int cast = 0, Node* l = NULL, Node* r = NULL) {//конструктор
        key = k;
        amount = cast;
        left = l;
        right = r;
    }
};

// создание дерева
Node* maketree(priority_queue<Node, vector<Node>, Node> leafs) {
    while (leafs.size() > 1) {
        Node left = leafs.top(); leafs.pop();
        Node right = leafs.top(); leafs.pop();
        Node* node = new Node('\0', left.amount + right.amount, new Node(left), new Node(right));
        leafs.push(*node);
    }
    return new Node(leafs.top());
}

// создание кодов по дереву
void huffman(Node*& root, string code, map<char, string>& table) {
    if (!root) return;
    if (!root->left && !root->right) table[root->key] = code;
    huffman(root->left, code + "0", table);
    huffman(root->right, code + "1", table);
}

// кодирование
void encode(string file1 = "input.txt", string file2 = "coded.txt") {
    ifstream in(file1, ios::binary);
    if (!in) {
        puts("Input file doesn't exist");
        return;
    }

    // частота байтов
    char x; map<char, int> freq;
    while (in.get(x)) freq[x]++;
    in.clear();
    in.seekg(0);

    // очередь узлов
    priority_queue<Node, vector<Node>, Node> pq;
    for (auto pair : freq) pq.push(Node(pair.first, pair.second));
    // создание дерева
    Node* root = maketree(pq);

    // таблца кодов
    map<char, string> table;
    huffman(root, "", table);

    ofstream out(file2, ios::binary);

    // запись частоты байтов
    out << (char)(freq.size() - 1);
    for (auto pair : freq) {
        out << pair.first;
        out.write((char*)&pair.second, sizeof(pair.second));
    }

    // кодирование
    char temp = 0, count = 0;
    while (in.get(x)) {
        for (auto iter = table[x].begin(); iter != table[x].end(); iter++) {
            temp |= ((*iter - '0') << (7 - count));
            count++;
            if (count == 8) {
                out << temp;
                temp = 0;
                count = 0;
            }
        }
    }

    in.close();
    out.close();
}

void compress(string file1 = "input.txt", string file2 = "coded.txt") {
    struct stat sb; int bsize = 0;
    struct stat se; int esize = 0;

    stat(file1.c_str(), &sb); bsize = sb.st_size;
    stat(file2.c_str(), &se); esize = se.st_size;

    cout << "compression: " << (bsize + 0.0) / esize << endl;
}

int main() {
    encode();
    compress();
}