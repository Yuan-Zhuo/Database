#ifndef btree___h
#define btree___h

#include <iostream>
#include <vector>
#include <queue>
#include "bdirectory.h"
#include "bstruct.h"
using namespace std;

template <typename T>
class Database;

template <typename T>
class BTree{
        friend class Database<T>;
        friend class Bdirectory<T>;

    public:
      BTree(string p, int m) : _size(0), _path(p), _m(m), _dir(p, m){
          _rootfile = _dir.createFile();
      }

      int order() const { return _m; }
      int size() const { return _size; }
      bool empty() const { return _size == 0; }

      void insert(const ref<T> &r);
      void remove(const T &key);
      Bkey<T> successor(const Bkey<T> &bk);
      void display();
      ref<T> search_(const T &key);
      string search(const T &key);

    private:
      int _size;
      const string _path;
      const int _m;
      string _rootfile;
      string _hotfile;
      Bdirectory<T> _dir;

      ref<T> pointDownref(const T key);
      ref<T> pointUpref(const T key);
      int _rank(vector<ref<T>> &v, T key) const;
      void solveOverflow(string str);
      void solveUnderflow(string str);
      void print(string str);
      
};

#endif

template <typename T>
ref<T> BTree<T>::pointUpref(const T key){
    string file = search(key);
    if(file.empty())
        file = _hotfile;
    
    BTNode<T> v;
    _dir.readFile(file, v);

    return v.keys[v.keys.size() - 1];
}

template <typename T>
ref<T> BTree<T>::pointDownref(const T key){
    string file = search(key);
    if(file.empty())
        file = _hotfile;
    
    BTNode<T> v;
    _dir.readFile(file, v);

    return v.keys[0];
}

//查找一个关键码，查找成功得到该关键码在数据库中的offset，否则返回-1
template<typename T>
ref<T> BTree<T>::search_(const T &key){
    ref<T> r = {key, -1, 0};

    BTNode<T> v;
    _dir.readFile(_rootfile, v);
    _hotfile = "";

    while(!v.selfname.empty()){
        int pos = _rank(v.keys, key);
        if (pos >= 0 && key == (v.keys[pos]).key){
            r.offset = (v.keys[pos]).offset;
            r.length = (v.keys[pos]).length;
            break;
        }

        _hotfile = v.selfname;
        _dir.readFile(v.children[pos + 1], v);
    }

    return r;
}

//插入一个关键码
template <typename T>
void BTree<T>::insert(const ref<T> &r){
    string str = search(r.key);
    if(!str.empty()){
        BTNode<T> v;
        _dir.readFile(str, v);
        int pos = 0;
        while (v.keys[pos].key != r.key)
            pos++;

        (v.keys[pos]).offset = r.offset;
        (v.keys[pos]).length = r.length;
        _dir.writeFile(str, v);
        return;
    }
        
    BTNode<T> _hot;
    _dir.readFile(_hotfile, _hot);

    int pos = _rank(_hot.keys, r.key);
    _hot.keys.insert(_hot.keys.begin() + pos + 1, r);
    _hot.children.insert(_hot.children.begin() + pos + 2, "");
    _size++;

    _dir.writeFile(_hotfile, _hot);

    solveOverflow(_hotfile);
    return;
}

//删除一个关键码，删除成功返回关键码在数据库中的offset,否则返回-1
template<typename T>
void BTree<T>::remove(const T &key){
    string str = search(key);
    if(str.empty())
        return;

    BTNode<T> v;
    _dir.readFile(str, v);

    int pos = _rank(v.keys, key);
    if (v.children[0] != ""){
        BTNode<T> u;
        _dir.readFile(v.children[pos + 1], u);

        string str = _dir.leftestchild(u.selfname);
        _dir.readFile(str, u);

        v.keys[pos] = u.keys[0];
        v = u;
        pos = 0;
    }

    v.keys.erase(v.keys.begin() + pos);
    v.children.erase(v.children.begin() + pos + 1);
    _size--;

    _dir.writeFile(v.selfname, v);

    solveUnderflow(v.selfname);
    return;
}

template<typename T>
void BTree<T>::display(){
    print(_rootfile);
}

template<typename T>
void BTree<T>::print(string str){
    queue<string> q;
    BTNode<T> ptr;
    _dir.readFile(str, ptr);

    if (ptr.keys.size() != 0){
        q.push(str);
    }

    string last = str;
    string nlast = "";
    if (ptr.children[0] != ""){
        nlast = ptr.children[ptr.children.size() - 1];
    }

    while(!q.empty()){
        BTNode<T> current;
        _dir.readFile(q.front(), current);
        q.pop();

        for (ref<T> r : (current.keys)){
            cout << r.key << ' ';
        }
        cout << "||";
        if (current.selfname == last){
            cout << endl;
            last = nlast;
            BTNode<T> nodelast;
            _dir.readFile(last, nodelast);
            if (nodelast.children[0] != ""){
                nlast = nodelast.children[nodelast.children.size() - 1];
            }
        }
        if (current.children[0] != ""){
            for (string child : (current.children)){
                q.push(child);
            }
        }
    }
    cout << endl;
}

//由Bkey的对象找直接后继节点 返回Bkey
template<typename T>
Bkey<T> BTree<T>::successor(const Bkey<T> &bk){
    BTNode<T> v;
    Bkey<T> cur;
    Bkey<T> emptybkey;

    if (bk.filename == "")
        return emptybkey;

    
    _dir.readFile(bk.filename, v);
    
    const int pos = _rank(v.keys, bk.r.key);
    if (v.children[0]!=""){
        BTNode<T> u;
        _dir.readFile(v.children[pos + 1], u);
        
        string str = _dir.leftestchild(u.selfname);
        _dir.readFile(str, u);

        cur.r = u.keys[0];
        cur.filename = u.selfname;
    } else if (v.keys.size() > pos + 1){
        cur.r = v.keys[pos + 1];
        cur.filename = v.selfname;
    } else if (v.parent != ""){
        BTNode<T> p;
        _dir.readFile(v.parent, p);
        int parent_pos = _rank(p.keys, bk.r.key);
        while(p.keys.size() == parent_pos + 1){
            if (p.selfname == _rootfile)
                return emptybkey;
            _dir.readFile(p.parent, p);
            parent_pos = _rank(p.keys, bk.r.key);
        }

        cur.r = p.keys[parent_pos + 1];
        cur.filename = p.selfname;
    }

    return cur;
}

template<typename T>
void BTree<T>::solveUnderflow(string str){
    BTNode<T> v;
    _dir.readFile(str, v);

    if ((_m + 1) / 2 <= v.children.size())
        return;

    BTNode<T> p;
    _dir.readFile(v.parent, p);
    if (v.parent == ""){
        if (!v.keys.size() && v.children[0] != ""){
            _rootfile = v.children[0];
            BTNode<T> _root;
            _dir.readFile(_rootfile, _root);
            _root.parent = "";
            _dir.removeFile(v.selfname);
            _dir.writeFile(_rootfile, _root);
        }
        return;
    }

    int pos = 0;
    while (p.children[pos] != v.selfname)
        pos++;

    if (pos > 0){
        BTNode<T> left;
        _dir.readFile(p.children[pos - 1], left);
        if ((_m + 1) / 2 < left.children.size()){
            v.keys.insert(v.keys.begin(), p.keys[pos - 1]);
            p.keys[pos - 1] = left.keys[left.keys.size() - 1];
            left.keys.pop_back();
            v.children.insert(v.children.begin(), left.children[left.children.size() - 1]);
            left.children.pop_back();
            if (v.children[0] != "")
                _dir.adjustparent(v.children[0], v.selfname);

            _dir.writeFile(v.selfname, v);
            _dir.writeFile(left.selfname, left);
            _dir.writeFile(p.selfname, p);

            return;
        }
    }

    if (p.children.size() - 1 > pos){
        BTNode<T> right;
        _dir.readFile(p.children[pos + 1], right);
        if ((_m + 1) / 2 < right.children.size()){
            v.keys.push_back(p.keys[pos]);
            p.keys[pos] = right.keys[0];
            right.keys.erase(right.keys.begin());
            v.children.push_back(right.children[0]);
            right.children.erase(right.children.begin());
            if (v.children[v.children.size() - 1] != "")
                _dir.adjustparent(v.children[v.children.size() - 1], v.selfname);

            _dir.writeFile(v.selfname, v);
            _dir.writeFile(right.selfname, right);
            _dir.writeFile(p.selfname, p);

            return;
        }
    }

    if (pos > 0){
        BTNode<T> left;
        _dir.readFile(p.children[pos - 1], left);
        
        left.keys.insert(left.keys.begin() + left.keys.size(), p.keys[pos - 1]);
        p.keys.erase(p.keys.begin() + pos - 1);
        p.children.erase(p.children.begin() + pos);
        left.children.push_back(v.children[0]);
        v.children.erase(v.children.begin());
        if (left.children[left.children.size() - 1] != "")
            _dir.adjustparent(left.children[left.children.size() - 1], left.selfname);

        while (!v.keys.empty()){
            left.keys.push_back(v.keys[0]);
            v.keys.erase(v.keys.begin());
            left.children.push_back(v.children[0]);
            v.children.erase(v.children.begin());

            if (left.children[left.children.size() - 1] != "")
                _dir.adjustparent(left.children[left.children.size() - 1], left.selfname);
        }

        _dir.writeFile(left.selfname, left);
        _dir.writeFile(p.selfname, p);
        _dir.removeFile(v.selfname);
    } else {
        BTNode<T> right;
        _dir.readFile(p.children[pos + 1], right);

        right.keys.insert(right.keys.begin(), p.keys[pos]);
        p.keys.erase(p.keys.begin() + pos);
        p.children.erase(p.children.begin() + pos);
        right.children.insert(right.children.begin(), v.children[v.children.size() - 1]);
        v.children.pop_back();
        if (right.children[0] != "")
            _dir.adjustparent(right.children[0], right.selfname);
        
        while (!v.keys.empty()){
            right.keys.insert(right.keys.begin(), v.keys[v.keys.size() - 1]);
            v.keys.pop_back();
            right.children.insert(right.children.begin(), v.children[v.children.size() - 1]);
            v.children.pop_back();

            if (right.children[0] != "")
                _dir.adjustparent(right.children[0], right.selfname);
            
        }
        _dir.writeFile(right.selfname, right);
        _dir.writeFile(p.selfname, p);
        _dir.removeFile(v.selfname);
    }

    solveUnderflow(p.selfname);
}

//查找一个关键码，查找成功返回该关键码所在节点的文件名，否则返回""
template<typename T>
string BTree<T>::search(const T &key){
    BTNode<T> v;
    _dir.readFile(_rootfile, v);
    _hotfile = "";

    while(!v.selfname.empty()){
        int pos = _rank(v.keys, key);
        if (pos >= 0 && key == (v.keys[pos]).key)
            return v.selfname;
        
        _hotfile = v.selfname;

        _dir.readFile(v.children[pos + 1], v);
    }

    return "";
}

template<typename T>
int BTree<T>::_rank(vector<ref<T>> &v,T key) const{
    int len = v.size();
    for (int i = 0; i < len; ++i){
        ref<T> r = v[i];
        if (r.key > key)
            return i - 1;
    }
    return len - 1;
}

template<typename T>
void BTree<T>::solveOverflow(string str){
    BTNode<T> v;
    _dir.readFile(str, v);

    if (v.children.size() <= _m)
        return;

    int pos = _m / 2;
    BTNode<T> u;
    u.selfname = _dir.createFile();
    for (int i = 0; i < _m - pos - 1; ++i){
        u.children.insert(u.children.begin() + i, v.children[pos + 1]);
        v.children.erase(v.children.begin() + pos + 1);
        u.keys.insert(u.keys.begin() + i, v.keys[pos + 1]);
        v.keys.erase(v.keys.begin() + pos + 1);
    }
    u.children[_m - pos - 1] = v.children[pos + 1];
    v.children.erase(v.children.begin() + pos + 1);

    if(!u.children[0].empty()){
        for (int i = 0; i < _m - pos; ++i){
            _dir.adjustparent(u.children[i], u.selfname);
        }
    }

    BTNode<T> p;
    bool flag = _dir.readFile(v.parent, p);
    if(!flag){
        _rootfile = p.selfname = _dir.createFile();
        p.children[0] = v.selfname;
        v.parent = p.selfname;
    }

    int parent_pos = _rank(p.keys, v.keys[0].key) + 1;
    p.keys.insert(p.keys.begin() + parent_pos, v.keys[pos]);
    v.keys.erase(v.keys.begin() + pos);
    p.children.insert(p.children.begin() + parent_pos + 1, u.selfname);
    u.parent = p.selfname;

    _dir.writeFile(v.selfname, v);
    _dir.writeFile(u.selfname, u);
    _dir.writeFile(p.selfname, p);

    solveOverflow(p.selfname);
}