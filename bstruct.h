#ifndef bstruct_h
#define bstruct_h

#include <vector>
#include <string>

using namespace std;

//声明
template <typename T>
class BTree;

//声明
template <typename T>
class Bdirectory;

//b树key的数据类型 key-offset-lenth
template<typename T>
struct ref{
    T key;
    long offset;
    int length;
};

//索引文件节点信息的数据类型 _keys_size,_children_size,_parent
struct info{
    int _keys_size = 0;
    int _children_size = 1;
    char _parent[30];
};

//b树节点的数据类型 包含parent,keys,children,selfname
template <typename T>
class BTNode{
      friend class BTree<T>;
      friend class Bdirectory<T>;
    
    private:
      string parent = "";
      vector<ref<T>> keys;
      vector<string> children;
      string selfname = "";

    public:
    BTNode(){
        parent = "";
        children.push_back("");  
    }
    void clear(){
        parent = "";
        keys.clear();
        children.clear();
        selfname = "";
    }   
};

//b树关键码的数据类型 包含关键码的值和关键码所在的节点的文件名
template<typename T>
struct Bkey{
    ref<T> r;
    string filename = "";
};


//表示文件操作出错的类
template<typename T>
class file_error{
    friend class Bdirectory<T>;
};

//Date的数据类型 包含key,value
template <typename T>
struct Data{
    ref<T> r;
    string value;
};

#endif