#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define code_value_bits 16

bool comp(const pair<char, unsigned int>& l, const pair<char, unsigned int>& r) {
    if (l.second != r.second) return l.second >= r.second;
    return l.first < r.first;
}

int fget_bit(unsigned char* input_bit, unsigned int* bit_len, FILE* input, unsigned int* useless_bit) {
    if ((*bit_len) == 0) {
        (*input_bit) = fgetc(input);
        if (feof(input)) {
            (*useless_bit)++;
            if ((*useless_bit) > 14) {
                puts("impossible to decode");
                exit(1);
            }
        }
        (*bit_len) = 8;
    }
    int result = (*input_bit) & 1;
    (*input_bit) >>= 1;
    (*bit_len)--;
    return result;
}

void decode(string file1 = "coded.txt", string file2 = "output.txt") {
    unsigned int* alfabet = new unsigned int[256]{};

    FILE* in = fopen(file1.c_str(), "rb");
    if (!in) {
        puts("Input file doesn't exist");
        return;
    }

    unsigned char col = 0;
    unsigned int col_letters = 0;
    col = fgetc(in);
    if (!feof(in))
        col_letters = static_cast<unsigned int>(col);

    unsigned char character = 0;

    for (int i = 0; i < col_letters; i++) {
        character = fgetc(in);
        if (!feof(in))
            fread(reinterpret_cast<char*>(&alfabet[character]), sizeof(unsigned short), 1, in);
        else {
            puts("impossible to decode");
            return;
        }
    }

    vector<pair<char, unsigned int>> vec;
    for (int i = 0; i < 256; i++)
        if (alfabet[i] != 0)
            vec.push_back(make_pair(static_cast<char>(i), alfabet[i]));

    sort(vec.begin(), vec.end(), comp);

    unsigned short* ranges = new unsigned short[vec.size() + 2];
    ranges[0] = 0;
    ranges[1] = 1;
    for (int i = 0; i < vec.size(); i++) {
        unsigned int b = vec[i].second;
        for (int j = 0; j < i; j++) 
            b += vec[j].second;
        ranges[i + 2] = b;
    }

    if (ranges[vec.size()] > (1 << ((code_value_bits - 2)) - 1)) {
        puts("freq error"); 
        return;
    }

    unsigned int low_value = 0;
    unsigned int high_value = ((static_cast<unsigned int>(1) << code_value_bits) - 1);
    unsigned int divider = ranges[vec.size() + 1];
    unsigned int first_qtr = (high_value + 1) / 4;
    unsigned int half = first_qtr * 2;
    unsigned int third_qtr = first_qtr * 3;

    unsigned int bit_len = 0;
    unsigned char input_bit = 0;
    unsigned int useless_bit = 0;
    unsigned short code_value = 0;
    int tmp = 0;

    FILE* out = fopen(file2.c_str(), "wb +");

    for (int i = 1; i <= 16; i++) {
        tmp = fget_bit(&input_bit, &bit_len, in, &useless_bit);
        code_value = 2 * code_value + tmp;
    }
    unsigned int diff = high_value - low_value + 1;
    while (1) {
        unsigned int freq = static_cast<unsigned int>(((static_cast<unsigned int>(code_value) - low_value + 1) * divider - 1) / diff);//считаем общую частоту

        int j;

        for (j = 1; ranges[j] <= freq; j++);//находим нужный символ
        high_value = low_value + ranges[j] * diff / divider - 1;
        low_value = low_value + ranges[j - 1] * diff / divider;

        while (1) {
            if (high_value < half);//если верхняя граница лежит в первой половине
            else if (low_value >= half) {//если нижняя граница лежит во второй половине, смещаем значения на половину
                low_value -= half;
                high_value -= half;
                code_value -= half;
            }
            else if ((low_value >= first_qtr) && (high_value < third_qtr)) {//если обе границы лежат во второй четверти смещаем значения на четверть
                low_value -= first_qtr;
                high_value -= first_qtr;
                code_value -= first_qtr;
            }
            else break;

            low_value += low_value;
            high_value += high_value + 1;
            tmp = 0;
            tmp = fget_bit(&input_bit, &bit_len, in, &useless_bit);
            code_value += code_value + tmp;
        }

        if (j == 1) break;

        fputc(vec[j - 2].first, out);
        diff = high_value - low_value + 1;
    }

    fclose(in);
    fclose(out);
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