#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <queue>
#include "bstruct.h"

using namespace std;

int main(){
    ifstream infile;
    infile.open("index.dat", ios::binary);

    ifstream file;
    file.open("value.dat", ios::binary);

    
    for (int i = 0; i < 10; ++i){
        ref<int> r;
        infile.read(reinterpret_cast<char *>(&r), sizeof(r));
        char ch[r.length + 1];
        file.read(ch, r.length);
        cout << r.key << '\t' << r.offset << '\t' << r.length << endl;
        cout << ch << endl;
    }
    infile.close();
    file.close();
}