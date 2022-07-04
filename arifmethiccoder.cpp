#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/stat.h>

using namespace std;

#define code_value_bits 16

bool comp(const pair<char, unsigned int>& l, const pair<char, unsigned int>& r) {//сравнени для сортировки
    if (l.second != r.second) return l.second >= r.second;
    return l.first < r.first;
}

int indexOFsymbol(char symbol, vector<pair<char, unsigned int>> vec) {//возвращает номер буквы в таблице
    for (int i = 0; i < vec.size(); i++) 
        if (symbol == vec[i].first)
            return i + 2;

    return -1;
}

void fput_bit(unsigned int bit, unsigned int* bit_len, unsigned char* file_bit, FILE* output) { //записывает биты в файл
    (*file_bit) = (*file_bit) >> 1;
    if (bit) (*file_bit) |= (1 << 7);

    (*bit_len)--;

    if ((*bit_len) == 0) {
        fputc((*file_bit), output);
        (*bit_len) = 8;
    }
}

void bitsANDfollow(unsigned int bit, unsigned int* follow_bits, unsigned int* bit_len, unsigned char* write_bit, FILE* output_file) {//переносит найденные биты в файл
    fput_bit(bit, bit_len, write_bit, output_file);

    for (; *follow_bits > 0; (*follow_bits)--)
        fput_bit(!bit, bit_len, write_bit, output_file);
}

void encode(string file1 = "input.txt", string file2 = "coded.txt") {
    unsigned int* alfabet = new unsigned int[256]{};
    FILE* in = fopen(file1.c_str(), "rb");
    if (!in) { 
        puts("input doesn't exist"); 
        return; 
    }

    unsigned char character = 0;
    //формируем начальный алфавит
    while (!feof(in)) {
        character = fgetc(in);
        if (!feof(in)) alfabet[character]++;
    }

    fclose(in);
    //формируем таблицу пар
    vector<pair<char, unsigned int>> vec;

    for (int i = 0; i < 256; i++)
        if (alfabet[i] != 0)
            vec.push_back(make_pair(static_cast<char>(i), alfabet[i]));

    sort(vec.begin(), vec.end(), comp);//Сортировка таблицы
    //формируем целочисленные интервалы
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
    unsigned int diff = high_value - low_value + 1;
    unsigned int first_qtr = (high_value + 1) / 4;
    unsigned int half = first_qtr * 2;
    unsigned int third_qtr = first_qtr * 3;

    unsigned int follow_bits = 0;//сколько битов записано условно
    unsigned int bit_len = 8;//сколько битов незаполнено
    unsigned char write_bit = 0;//результат работы алгоритма

    int j = 0;

    in = fopen(file1.c_str(), "rb");
    FILE* out = fopen(file2.c_str(), "wb +");
    //записываем в шапку кол-во символов и кол-во каждого символа
    fputc(vec.size(), out);
    for (int i = 0; i < 256; i++)
        if (alfabet[i] != 0) {
            fputc(static_cast<char>(i), out);
            fwrite(reinterpret_cast<const char*>(&alfabet[i]), sizeof(unsigned short), 1, out);
        }


    while (!feof(in)) {
        character = fgetc(in);

        if (!feof(in)) {
            j = indexOFsymbol(character, vec);//находим индекс символа в таблице инервалов
            //рассчитываем новые границы
            high_value = low_value + ranges[j] * diff / divider - 1;
            low_value = low_value + ranges[j - 1] * diff / divider;

            while (1) {
                if (high_value < half)//если верхняя граница лежит в первой половине, записываем 0
                    bitsANDfollow(0, &follow_bits, &bit_len, &write_bit, out);
                else if (low_value >= half) {//если нижняя граница лежит во второй половине, записываем 1 и меняем границы
                    bitsANDfollow(1, &follow_bits, &bit_len, &write_bit, out);
                    low_value -= half;
                    high_value -= half;
                }
                else if ((low_value >= first_qtr) && (high_value < third_qtr)) {//если обе границы лежат во второй четверти, записываем бит условно и меняем границы
                    follow_bits++;
                    low_value -= first_qtr;
                    high_value -= first_qtr;
                }
                else break;

                low_value += low_value;
                high_value += high_value + 1;
            }
        }
        else {//кодируем последний символ
            high_value = low_value + ranges[1] * diff / divider - 1;
            low_value = low_value + ranges[0] * diff / divider;

            while (1) {
                if (high_value < half)
                    bitsANDfollow(0, &follow_bits, &bit_len, &write_bit, out);
                else if (low_value >= half) {
                    bitsANDfollow(1, &follow_bits,
                        &bit_len, &write_bit, out);
                    low_value -= half;
                    high_value -= half;
                }
                else if ((low_value >= first_qtr) && (high_value < third_qtr)) {
                    follow_bits++;
                    low_value -= first_qtr;
                    high_value -= first_qtr;
                }
                else break;

                low_value += low_value;
                high_value += high_value + 1;
            }

            follow_bits++;

            if (low_value < first_qtr) //если нижняя граница лежит в первой четверти, записываем 0, иначе 1
                bitsANDfollow(0, &follow_bits, &bit_len, &write_bit, out);
            else
                bitsANDfollow(1, &follow_bits, &bit_len, &write_bit, out);
            //записываем остаток
            write_bit >>= bit_len;
            fputc(write_bit, out);
        }
        diff = high_value - low_value + 1;
    }

    fclose(in);
    fclose(out);
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