#ifndef bdirectory_h
#define bdirectory_h

#include <iostream>
#include <fstream>
#include <windows.h>
#include <cstdio>
#include <sstream>
#include <cstring>
#include "bstruct.h"

using namespace std;

template <typename T>
class BTree;

template<typename T>
class Bdirectory{
    friend class BTree<T>;

  private:
    const string _path;
    const int _m;
    const int _ksize;
    static const int _csize = sizeof(char) * 30;
    const int _isize;
    int _count;
    int _sum;

  public:
        Bdirectory(string p, int m) : _path(p), _m(m), _ksize(sizeof(ref<T>)), _isize(sizeof(info)), _count(0), _sum(0){
            bool flag = CreateDirectory(_path.c_str(), NULL);
            if (!flag)
                throw file_error<T>();
        }
        ~Bdirectory();

        int filecount() { return _count; }
        void display(string filename);

        string createFile();
        bool readFile(string filename, BTNode<T> &v);
        bool writeFile(string filename, BTNode<T> &v);
        bool removeFile(string filename);
        void adjustparent(string childname, string parentname);
        string leftestchild(string filename);
};

#endif

template<typename T>
string Bdirectory<T>::leftestchild(string filename){
    ifstream infile;
    infile.open(filename.c_str(), ios::binary);
    if(infile.fail())
        throw file_error<T>();

    infile.seekg(_ksize * _m, ios::beg);
    char ch[30];
    infile.read(reinterpret_cast<char *>(&ch), _csize);
    string str = ch;

    if(str.empty()){
        return filename;
    }

    return leftestchild(str);
}

template<typename T>
void Bdirectory<T>::adjustparent(string childname,string parent){
    ofstream outfile;
    outfile.open(childname.c_str(), ios::ate | ios::in | ios::binary);
    if(outfile.fail())
        throw file_error<T>();

    outfile.seekp(-_isize, ios::end);
    outfile.seekp(2 * sizeof(int), ios::cur);
    char ch[30];
    strcpy(ch, parent.c_str());
    outfile.write(reinterpret_cast<char *>(&ch), sizeof(ch));

    outfile.close();
}

template <typename T>
void Bdirectory<T>::display(string filename){
    ifstream infile;
    infile.open(filename.c_str(), ios::binary);
    if(infile.fail())
        throw file_error<T>();

    infile.seekg(-_isize, ios::end);
    info i;
    infile.read(reinterpret_cast<char *>(&i), _isize);

    if (i._keys_size == 0)
        return;
    
    infile.seekg(0, ios::beg);
    for (int j = 0; j < i._keys_size; ++j){
        ref<T> r;
        infile.read(reinterpret_cast<char *>(&r), _ksize);
        cout << r.key << '\t' << r.offset << endl;
    }
    cout << endl;

    char ch[30];
    infile.seekg(_ksize * _m, ios::beg);
    for (int j = 0; j < i._children_size; ++j){
        infile.read(reinterpret_cast<char *>(&ch), _csize);
        cout << ch << endl;
    }
    cout << endl;

    cout << i._parent << endl;

    infile.close();
    return;
}

template <typename T>
string Bdirectory<T>::createFile(){
    stringstream stream;
    stream << _sum;
    string filename = _path;
    string addname;
    stream >> addname;
    addname += ".dat";
    stream.clear();
    filename = filename + '/' + addname;

    ofstream outfile;
    outfile.open(filename.c_str(), ios::binary);
    if(outfile.fail())
        throw file_error<T>();

    ref<T> r;
    for (int i = 0; i < _m; ++i){
        outfile.write(reinterpret_cast<char *>(&r), _ksize);
    }
    char ch[30] = "";
    for (int i = 0; i <= _m; ++i){
        outfile.write(reinterpret_cast<char *>(&ch), _csize);
    }
    info i;
    outfile.write(reinterpret_cast<char *>(&i), _isize);

    outfile.close();
    _sum++;
    _count++;

    return filename;
}

template<typename T>
bool Bdirectory<T>::readFile(string filename, BTNode<T> &v){
    v.clear();

    if(filename==""){
        v.children.push_back("");
        v.selfname="";
        return false;
    }
    
    ifstream infile;
    infile.open(filename.c_str(), ios::binary);
    if(infile.fail())
        //return false;
        throw file_error<T>();

    infile.seekg(-_isize, ios::end);
    info i;
    infile.read(reinterpret_cast<char *>(&i), _isize);

    infile.seekg(0, ios::beg);
    for (int j = 0; j < i._keys_size; ++j){
        ref<T> r;
        infile.read(reinterpret_cast<char *>(&r), _ksize);
        v.keys.push_back(r);
    }

    infile.seekg(_ksize * _m, ios::beg);
    for (int j = 0; j < i._children_size; ++j){
        char ch[30];
        infile.read(reinterpret_cast<char *>(&ch), _csize);
        v.children.push_back(ch);
    }

    v.selfname = filename;
    v.parent = i._parent;

    infile.close();
    return true;
}

template<typename T>
bool Bdirectory<T>::writeFile(string filename, BTNode<T> &v){
    ofstream outfile;
    outfile.open(filename.c_str(), ios::ate | ios::in | ios::binary);
    if(outfile.fail())
        throw file_error<T>();

    outfile.seekp(0, ios::beg);
    int keys_size = v.keys.size();
    ref<T> r;
    for (int j = 0; j < keys_size; ++j){
        r = v.keys[j];
        outfile.write(reinterpret_cast<char *>(&r), _ksize);
    }

    outfile.seekp(_ksize * _m, ios::beg);
    int children_size = v.children.size();
    char ch[30];
    for (int j = 0; j < children_size; ++j){
        string str = v.children[j];
        strcpy(ch, str.c_str());
        outfile.write(reinterpret_cast<char *>(&ch), _csize);
    }

    outfile.seekp(-_isize, ios::end);
    info i;
    i._keys_size = keys_size;
    i._children_size = children_size;
    strcpy(i._parent, v.parent.c_str());
    outfile.write(reinterpret_cast<char *>(&i), _isize);

    outfile.close();
    return true;  
}

template <typename T>
bool Bdirectory<T>::removeFile(string filename){
    int x = remove(filename.c_str());
    if(x!=0)
        throw file_error<T>();

    _count--;
    return true;
}

template<typename T>
Bdirectory<T>::~Bdirectory(){
    string cmd_delete = "rd /s /q ";

    int pos = _path.find('/');
    string cmd_path = _path;
    cmd_path.replace(pos, 1, "\\\\");

    cmd_delete += cmd_path;
    system(cmd_delete.c_str());
}