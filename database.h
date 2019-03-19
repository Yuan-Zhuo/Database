#ifndef Database__h
#define Database__h

#include <iostream>
#include "btree.h"
#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <set>
#include <queue>

using namespace std;

template <typename T>
class Database{
    public:
      Database(string p,string index,string dat,int m):tree(p,m),_index(index),_dat(dat),_ksize(sizeof(T)){
          readIndex();
      }

      ~Database(){}

      void put(const T key, const string &value);
      string get(const T key);
      string remove(const T key);
      vector<string> get_range(const T key1, const T key2);
      
      void flush();
    
    private:
      BTree<T> tree;
      vector<Data<T>> buffer;

      const string _index;
      const string _dat;

      const int _ksize;
      static const int _fetch = 50;

      ref<T> dataref(const T key);

      void readIndex();
      void changeDat(const Data<T> &d);
      long addDat(const Data<T> &d);
      string getData(const ref<T> &r);
};

#endif

template <typename T>
ref<T> Database<T>::dataref(const T key){
    ref<T> r;

    ref<T> refdown = tree.pointDownref(key);
    ref<T> refup = tree.pointUpref(key);

    if (refdown.key > key){
        r = refdown;
    } else if (refup.key < key){
        r = refup;
    } else {
        while (refdown.key != key){
            string file = tree.search(refdown.key);
            Bkey<T> bk = {refdown, file};
            Bkey<T> bk1 = tree.successor(bk);
            refdown = bk1.r;
        }
        r = refdown;
    }

    return r;
}

//pass
template <typename T>
string Database<T>::get(const T key){
    int num = buffer.size();
    for (int i = 0; i < num; ++i){
        Data<T> d = buffer[i];
        if (d.r.key == key)
            return d.value;
    }

    ref<T> r = tree.search_(key);
    if (r.offset == -1)
        return "";

    return getData(r);
}

template <typename T>
vector<string> Database<T>::get_range(const T key1, const T key2){
    flush();
    vector<string> path;
    ref<T> r1 = dataref(key1);
    ref<T> r2 = dataref(key2);

    while (r1.key != r2.key){
        string file = tree.search(r1.key);
        string str = getData(r1);
        path.push_back(str);
        Bkey<T> bk = {r1, file};
        Bkey<T> bk1 = tree.successor(bk);
        r1 = bk1.r;
    }
    string str = getData(r2);
    path.push_back(str);

    if (r1.key < key1){
        path.erase(path.begin());
    }
    if (r2.key > key2){
        path.erase(path.begin() + path.size() - 1);
    }
    return path;
}

template <typename T>
string Database<T>::remove(const T key){
    int pos = -1;
    int num = buffer.size();
    for (int i = 0; i < num; ++i){
        Data<T> d = buffer[i];
        if (d.r.key == key)
            pos = i;
    }

    string str = "";
    if (pos >= 0){
        str = (buffer[pos]).value;
        buffer.erase(buffer.begin() + pos);
    } else {
        ref<T> r = tree.search_(key);
        if (r.offset != -1){
            str = getData(r);
            tree.remove(key);
        }
    }
    return str;
}

//pass
template <typename T>
void Database<T>::flush(){
    queue<Data<T>> q;

    int num = buffer.size();
    for (int i = 0; i < num; ++i){
        q.push(buffer[i]);
    }
    buffer.clear();

    while(!q.empty()){
        Data<T> d = q.front();
        q.pop();
        if (d.r.offset == -1){
            d.r.offset = addDat(d);
        } else {
            changeDat(d);
        }
        tree.insert(d.r);
    }

    return;
}

//pass
template <typename T>
void Database<T>::put(const T key, const string &value){
    int num = buffer.size();
    for (int i = 0; i < num; ++i){
        Data<T> d = buffer[i];
        if (d.r.key == key){
            (buffer[i]).value = value;
            return;
        }
    }

    ref<T> r = tree.search_(key);
    if (r.offset != -1 && r.length < value.size()){
        tree.remove(key);
        r.offset = -1;
    }
    r.length = value.size();
    Data<T> d = {r, value};
    buffer.push_back(d);

    if (buffer.size() >= _fetch)
        flush();
}

//pass
template<typename T>
void Database<T>::readIndex(){
    ifstream infile;
    infile.open(_index.c_str(), ios::binary);
    if(infile.fail())
        throw file_error<T>();

    const int size = sizeof(ref<T>);
    ref<T> r;
    infile.read(reinterpret_cast<char *>(&r), size);
    while(!infile.eof()){
        tree.insert(r);
        infile.read(reinterpret_cast<char *>(&r), size);
    }
}

//pass
template<typename T>
void Database<T>::changeDat(const Data<T> &d){
    ofstream outfile;
    outfile.open(_dat.c_str(), ios::ate | ios::in | ios::binary);
    if(outfile.fail())
        throw file_error<T>();

    outfile.seekp(d.r.offset, ios::beg);
    char ch[d.r.length];
    strcpy(ch, d.value.c_str());
    outfile.write(ch, d.r.length);

    outfile.close();
}

//pass
template<typename T>
long Database<T>::addDat(const Data<T> &d){
    ofstream outfile;
    outfile.open(_dat.c_str(), ios::ate | ios::in | ios::binary);
    if(outfile.fail())
        throw file_error<T>();

    outfile.seekp(0, ios::end);
    long off = outfile.tellp();
    char ch[d.r.length + 1];
    strcpy(ch, d.value.c_str());
    outfile.write(ch, d.r.length);

    outfile.close();

    return off;
}

//pass
template<typename T>
string Database<T>::getData(const ref<T> &r){
    ifstream infile;
    infile.open(_dat.c_str(), ios::binary);
    if(infile.fail()){
        throw file_error<T>();
    }

    infile.seekg(r.offset, ios::beg);
    char ch[r.length + 1];
    infile.read(ch, r.length);

    infile.close();
    string str = ch;
    return str;
}