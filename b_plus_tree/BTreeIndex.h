#include "BTree.h"
#include "DataBase.h"

#include <iostream>

using namespace std;

template <class KeyType, class RecordType>
class BTreeIndex {
private:
	BTree<KeyType, int> * btree = NULL;
	DataBase<RecordType> * db = NULL;
public:
	BTreeIndex(int n);
	~BTreeIndex();
	//添加一个记录并且将key加入索引。
	void add(KeyType key, RecordType record);
	//删除指定key的记录。
	void del(KeyType key);
	//查询指定key的记录。
	void find(KeyType key ,RecordType & record);
	//返回成员变量
	DataBase<RecordType> * getDB();
	//返回成员变量
	BTree<KeyType, int> * getBTree();
};


template <class KeyType, class RecordType>
BTreeIndex<KeyType, RecordType>::BTreeIndex(int n) {
	btree = new BTree<KeyType, int>(n);
	db = new DataBase<RecordType>(string("dbFile")); //todo: 这里数据库默认"dbfile"暂时的设计
}

template <class KeyType, class RecordType>
BTreeIndex<KeyType, RecordType>::~BTreeIndex() {
	delete btree;
	btree = NULL;
	delete db;
	db = NULL;
}

//添加一个记录并且将key加入索引。
template <class KeyType, class RecordType>
void BTreeIndex<KeyType, RecordType>::add(KeyType key, RecordType record) {
	//增加数据，获得该数据的地址。
	int pos = db->save(record);
	//将(key->pos)加入索引。
	btree->add(key, pos);
}

//删除指定key的记录。
template <class KeyType, class RecordType>
void BTreeIndex<KeyType, RecordType>::del(KeyType key) {
	//根据索引找到要删除的记录的位置。
	int pos = this->btree->getKeyValue(key);
	//删除数据库文件中指定位置的记录。
	this->db->remove(pos);
	//删除索引。
	this->btree->del(key);
}

//查询指定key的记录。
template <class KeyType, class RecordType>
void BTreeIndex<KeyType, RecordType>::find(KeyType key, RecordType & record) {
	//根据索引找到要查询的记录的位置。
	int pos = this->btree->getKeyValue(key);
	if (pos != -1) {
		//返回pos位置的记录。
		record = this->db->getByOffset(pos);
	}
}

//返回成员变量
template <class KeyType, class RecordType>
DataBase<RecordType> * BTreeIndex<KeyType, RecordType>::getDB() {
	return this->db;
}

//返回成员变量
template <class KeyType, class RecordType>
BTree<KeyType, int> * BTreeIndex<KeyType, RecordType>::getBTree() {
	return this->btree;
}
