#include <iostream>
#include <string>
#include <fstream>
#include <chrono>
#include <random>
#include <cstring>
#include "bstruct.h"

using namespace std;

int main(){
    string str = "iabfiasefjksfbsefhjejebfb";

    ofstream indexfile;
    indexfile.open("index.dat", ios::binary);
    ofstream datfile;
    datfile.open("value.dat", ios::binary);

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine e(seed);
    uniform_int_distribution<int> d1(1, 8);

    long offset = 0;
    int size = sizeof(ref<int>);

    int num = 1000;
    for (int i = 0; i < num ; ++i){
        int length = d1(e);
        string current = str.substr(0, length);

        ref<int> r = {i, offset, length};
        offset += length;

        indexfile.write(reinterpret_cast<char *>(&r), size);
        datfile.write(current.c_str(), length);
    }

    indexfile.close();
    datfile.close();

    cout << "final" << endl;
}