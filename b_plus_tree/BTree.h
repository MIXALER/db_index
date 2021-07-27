#include <iostream>
#include <string>
#include <fstream>
#include <queue>
#include <math.h>

using namespace std;

/*	@desc:BTreeNode
@1:B树的特点（指定每个结点最多n个key的情况）
@根结点：根结点中至少有两个指针被使用，所有结点指向B树下一层结点。
@叶子结点：键有序从左到右分布。最后一个指针用于指向同层下一个结点。剩下指针中至少((n+1)/2)下取整个被使用，自然也至少((n+1)/2)下取整个key。
@内层结点：键有序。至少((n+1)/2)上取整个指针使用。

举例1：n=3，则叶子结点至少2个指针，2个key。内层结点至少2个指针，1个key。
举例2：n=4，则叶子结点至少2个指针，2个key。内层结点至少3个指针，2个key。
举例3：n=5：则叶子结点至少3个指针，3个key。内层结点至少3个指针，2个key。
举例4：n=10：则叶子结点至少5个指针，5个key。内层结点至少6个指针，5个key。
*/

template<class KeyType, class ValueType>
class BTreeNode
{
public:
    int keyNumber = 0;//用于记录当前本结点中key的数量。
    bool isLeaf = true;//标识本结点是否为叶子结点。
    KeyType *key;//key用于记录所有的key。
    //n个(创建时创建n+1个，不使用第0个)
    BTreeNode<KeyType, ValueType> *parent;//parent指向父节点
    void **ptr; //ptr是孩子指针
    //n+1个(创建时创建n+1个，全部使用)
    ValueType *value;//叶子结点的值。

    BTreeNode(int n, bool isLeaf);//n表示结点允许的最大key数量
    ~BTreeNode();
};

template<class KeyType, class ValueType>
BTreeNode<KeyType, ValueType>::BTreeNode(int n, bool isLeaf)
{
    this->isLeaf = isLeaf;
    this->keyNumber = 0;
    this->parent = NULL;
    this->key = new KeyType[n + 1];
    this->ptr = new void *[n + 1];
    for (int i = 0; i < n + 1; i++)
    {
        this->ptr[i] = NULL;
    }
    //叶子结点
    if (isLeaf)
    {
        value = new ValueType[n + 1];
    }
}

template<class KeyType, class ValueType>
BTreeNode<KeyType, ValueType>::~BTreeNode()
{
    this->keyNumber = 0;
    this->parent = NULL;
    delete[] this->key;
    this->key = NULL;
    delete[] this->ptr;
    this->ptr = NULL;

    if (isLeaf)
    {
        delete[] value;
    }
}

/* BTree */
template<class KeyType, class ValueType>
class BTree
{
private:
    int n = 0;//n表示每个结点最少的key数量。
    int innerNodeMinPtrs = 0;
    int innerNodeMinKey = 0;
    int leafNodeMinPtrs = 0;
    int leafNodeMinKey = 0;

    BTreeNode<KeyType, ValueType> *root;//根结点。

public:

    BTree(int n);

    ~BTree();

    //释放p所指向结点以及其子结点。
    void freeNode(BTreeNode<KeyType, ValueType> *p);

    //在索引树种查找关键字key。
    //	找到时p指向key所在结点, 返回key所在位置i;
    //	找不到时, p指向key应该插入的叶子结点, 返回0。
    int find(BTreeNode<KeyType, ValueType> *&p, KeyType key);

    //在索引树种查找关键字key，返回对应的值。
    //-1表示找不到
    ValueType getKeyValue(KeyType key);

    //在指定结点中种查找关键字key的位置。
    //	找到--->返回key所在位置i;
    //	找不到->返回-1。
    int indexOf(BTreeNode<KeyType, ValueType> *p, KeyType key);

    //添加一个key，key最终会被添加到一个叶子结点。
    int add(KeyType key, ValueType value);

    //向一个指定的叶子结点中插入key。
    void insert2node(BTreeNode<KeyType, ValueType> *p, KeyType key, ValueType value);

    //向一个指定的未满叶子结点中插入key。
    void insert2notFullNode(BTreeNode<KeyType, ValueType> *p, KeyType key, ValueType value);

    //向一个指定的内部结点中插入key。
    void insertToInnerNode(BTreeNode<KeyType, ValueType> *p, KeyType key, BTreeNode<KeyType, ValueType> *ptr);

    //向一个指定的未满内部结点中插入key。
    void insertToNotFullInnerNode(BTreeNode<KeyType, ValueType> *p, KeyType key, BTreeNode<KeyType, ValueType> *ptr);

    //分裂叶子结点
    BTreeNode<KeyType, ValueType> *splitNode(BTreeNode<KeyType, ValueType> *p,
                                             KeyType key,
                                             ValueType value);

    //分裂内部结点
    BTreeNode<KeyType, ValueType> *splitInnerNode(BTreeNode<KeyType, ValueType> *p,
                                                  KeyType key,
                                                  BTreeNode<KeyType, ValueType> *ptr);

    //删除key
    bool del(int key);

    //删除指定结点中指定位置的key，并且如果需要更新祖先结点值则更新。
    bool del_direct_index(BTreeNode<KeyType, ValueType> *p, int index);

    //删除指定内部结点中key。
    bool del_inner_direct_index(BTreeNode<KeyType, ValueType> *p, int index);

    //删除内部结点的指定key。
    bool del_inner_node(BTreeNode<KeyType, ValueType> *p, int key);

    //在指定结点中种查找关键字key的位置。
    //	返回p在其父亲中位置i;
    //	若p无父亲，返回-1.
    int indexInParent(BTreeNode<KeyType, ValueType> *p);

    //递归替换oldKey为newKey(直接找到并且替换则结束，或者到根结点都没找到也结束)。
    void replace(BTreeNode<KeyType, ValueType> *p, KeyType oldKey, KeyType newKey);

    //返回树的第一个叶子结点。
    BTreeNode<KeyType, ValueType> *firstLeafNode();

    //打印B树(方便调试)
    void print();

    //打印叶子结点。
    void printAllLeaf();

    //打印一个结点的key
    void printNode(BTreeNode<KeyType, ValueType> *pNode);

};

/**
@desc:构造函数，n表示每个结点最多的key的数量。
*/
template<class KeyType, class ValueType>
BTree<KeyType, ValueType>::BTree(int n)
{
    this->n = n;
    this->innerNodeMinPtrs = ceil((1.0 + n) / 2.0);
    this->innerNodeMinKey = innerNodeMinPtrs - 1;
    this->leafNodeMinPtrs = floor((1.0 + n) / 2.0);
    this->leafNodeMinKey = leafNodeMinPtrs;
    root = new BTreeNode<KeyType, ValueType>(n, true);


    //索引载入。
    fstream fs;
    fs.open("index", ios::in | ios::binary);
    KeyType key;
    int pos;
    if (fs)
    {
        while (!fs.eof())
        {
            key = -1;
            pos = -1;
            fs.read((char *) (&key), sizeof(KeyType));
            if (fs.eof())
                break;
            fs.read((char *) (&pos), sizeof(int));
            if (key != -1 && pos != -1)
            {
                this->add(key, pos);
            }
        }
    }

}

/**
@desc:析构函数，释放整个树的结点空间。
*/
template<class KeyType, class ValueType>
BTree<KeyType, ValueType>::~BTree()
{

    //索引写入文件。
    fstream fs("index", ios::out | ios::binary);
    KeyType key;
    BTreeNode<KeyType, ValueType> *p = this->firstLeafNode();
    while (p)
    {
        //遍历p
        for (int i = 1; i <= p->keyNumber; i++)
        {
            fs.write((char *) (&(p->key[i])), sizeof(KeyType));
            fs.write((char *) (&(p->value[i])), sizeof(ValueType));
        }
        p = (BTreeNode<KeyType, ValueType> *) (p->ptr[0]);
    }
    fs.flush();
    fs.close();

    n = 0;
    freeNode(root);
    root = NULL;
}

/**
@desc:释放p所指向结点以及其子结点。
*/
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::freeNode(BTreeNode<KeyType, ValueType> *p)
{
    //叶子结点直接释放并且返回。
    if (p->isLeaf)
    {
        delete p;
        return;
    }
    //非叶子结点，首先循环释放p所指向结点的所有子树
    //注意：key是从下标1->keyNumnber
    //ptr是从下标0->keyNumnber
    //ptr会比key多一个
    for (int i = 0; i <= p->keyNumber; i++)
    {
        freeNode((BTreeNode<KeyType, ValueType> *) (p->ptr[i]));
    }
    //释放p结点
    delete p;
}

/*
@desc:在索引树种查找关键字key。
@return:
找到时p指向key所在结点,返回key所在位置i;
找不到时,p指向key应该插入的叶子结点,返回0。
*/
template<class KeyType, class ValueType>
int BTree<KeyType, ValueType>::find(BTreeNode<KeyType, ValueType> *&p, KeyType key)
{
    p = this->root;
    while (p)
    {
        int i = p->keyNumber;
        while (i > 0 && key < p->key[i])
        {
            i--;
        }
        /*
        叶子结点:
        //i==0 ==> key<p->key[1],返回i(0) ==> 没找到
        //i!=0 ==> key>=p->key[i],返回i(非0,代表了位置)
        //     ==> key==p->key[i]则找到,否则没找到。
        */
        if (p->isLeaf)
        {
            if (i > 0 && key == p->key[i])
            {
                return i;//找到
            }
            return 0;//找不到
        } else
        {
            p = (BTreeNode<KeyType, ValueType> *) (p->ptr[i]);//如果不是叶子结点,则继续搜寻子树。
        }
    }//p==NULL理论上是不存在这种情况的,只有异常情况。
    return -1;//异常情况
}

//在索引树种查找关键字key，返回对应的值。
//-1表示找不到
template<class KeyType, class ValueType>
ValueType BTree<KeyType, ValueType>::getKeyValue(KeyType key)
{
    BTreeNode<KeyType, ValueType> *pNode;
    int ret = this->find(pNode, key);
    if (ret > 0)
        return pNode->value[ret];
    else
        return -1;
}

/*
@desc:添加key到索引树中。
@return:
-1,插入key过程发生异常。
0,插入key已经存在了。
1,插入成功。
*/

//在指定结点中种查找关键字key的位置。
//	找到--->返回key所在位置i;
//	找不到->返回-1。
template<class KeyType, class ValueType>
int BTree<KeyType, ValueType>::indexOf(BTreeNode<KeyType, ValueType> *p, KeyType key)
{
    int i = p->keyNumber;
    while (i > 0 && key != p->key[i])
    {
        i--;
    }
    if (i == 0)
    {
        //not found
        return -1;
    } else
    {
        return i;
    }
}

template<class KeyType, class ValueType>
int BTree<KeyType, ValueType>::add(KeyType key, ValueType value)
{
    BTreeNode<KeyType, ValueType> *p = NULL;//创建一个结点指针用于查询key的时候存储返回结点指针。
    int ret = this->find(p, key);
    if (ret == -1 || p == NULL)
    {
        return -1;//异常情况
    }
    if (ret != 0)
    {
        return 0;//在索引中找到了key,不进行操作,返回0。
    }
    insert2node(p, key, value);
    return 1;
}

/*
@desc:分裂叶子结点p，返回分裂后右边那个结点。
*/
template<class KeyType, class ValueType>
BTreeNode<KeyType, ValueType> *
BTree<KeyType, ValueType>::splitNode(BTreeNode<KeyType, ValueType> *p, KeyType key, ValueType value)
{
    BTreeNode<KeyType, ValueType> *r = new BTreeNode<KeyType, ValueType>(n, true);
    r->isLeaf = p->isLeaf;
    r->parent = p->parent;

    //1,2,3,...,leafNodeMinKey(leafMinKey个key在左结点)
    //leafNodeMinKey+1,leafNodeMinKey+2m....(剩下的在右结点)
    //key放哪边呢？如果放左边，则需要把leafNodeMinKey那一项放右边

    //如果key小于leafMinKey那个位置的key则会在左结点中
    if (key < p->key[this->leafNodeMinKey])
    {
        //从leafMinKey位置开始往r结点中复制
        int j = 1;
        for (int i = this->leafNodeMinKey; i <= p->keyNumber; i++, j++)
        {
            r->key[j] = p->key[i];
            r->value[j] = p->value[i];
            //r->ptr[j] = p->ptr[i];
            //p->ptr[i] = NULL;
        }
        //在左边结点中插入key,将要插入位置之后的key和ptr进行后移。
        int i = -1;
        for (i = this->leafNodeMinKey - 1; i > 0; i--)
        {
            if (key < p->key[i])
            {
                p->key[i + 1] = p->key[i];
                p->value[i + 1] = p->value[i];
                //p->ptr[i + 1] = p->ptr[i];
            } else
            {
                //此时key要插入的位置就是i+1处
                break;
            }
        }
        //插入key
        p->key[i + 1] = key;
        p->value[i + 1] = value;
        p->keyNumber = this->leafNodeMinKey;
        //确定右边结点的key数量
        r->keyNumber = this->n + 1 - p->keyNumber;
    } else
    {
        //从leafMinKey+1位置开始往r结点中复制(复制过程中顺便插入key)
        int j = 1;
        int flag = 0;
        for (int i = this->leafNodeMinKey + 1; i <= p->keyNumber; i++)
        {
            if (p->key[i] > key)
            {
                //插入key
                r->key[j] = key;
                r->value[j] = value;
                j++;
                flag = 1;
            }
            r->key[j] = p->key[i];
            r->value[j] = p->value[i];
            //r->ptr[j] = p->ptr[i];
            //p->ptr[i] = NULL;
            j++;
        }
        if (!flag)
        {
            //没有插入key，说明key是最大的，所以继续插入key即可
            r->key[j] = key;
            r->value[j] = value;
        }
        //确定左边结点的key数量
        p->keyNumber = this->leafNodeMinKey;
        //确定右边结点的key数量
        r->keyNumber = this->n + 1 - p->keyNumber;
    }
    //通过第0个指针指向下一个叶子结点
    if (p->isLeaf)
    {
        r->ptr[0] = p->ptr[0];
        p->ptr[0] = r;
    }

    return r;
}

/*
@desc:分裂内部结点p，返回分裂后右边那个结点。
@return：返回分裂出来的右边结点，并且，右边结点的0号位置key是需要插入到父结点中的。
*/
template<class KeyType, class ValueType>
BTreeNode<KeyType, ValueType> *BTree<KeyType, ValueType>::splitInnerNode(BTreeNode<KeyType, ValueType> *p, KeyType key,
                                                                         BTreeNode<KeyType, ValueType> *ptr)
{
    BTreeNode<KeyType, ValueType> *r = new BTreeNode<KeyType, ValueType>(n, true);
    r->isLeaf = p->isLeaf;
    r->parent = p->parent;

    //1,2,3,...,leafNodeMinKey(leafMinKey个key在左结点)
    //leafNodeMinKey+1,leafNodeMinKey+2m....(剩下的在右结点)
    //key放哪边呢？如果放左边，则需要把leafNodeMinKey那一项放右边

    //--->并且不同于叶子结点分裂的是，右边结点的第一个key不存储，而是要作为p的parent的key插入进去。

    //如果key小于leafMinKey那个位置的key则会在左结点中
    if (key < p->key[this->leafNodeMinKey])
    {
        //从leafMinKey位置开始往r结点中复制
        /*	注意此处j设置为0
        @1：首先，leafNodeMinKey位置的key是不需要放到右结点的，所以放入右结点0位置也无妨（0位置无用）
        @2：其次，右结点的ptr[0]位置需要放置的是准备插入到父结点中的key对应的那个ptr。而这种情况下也正好就是leafNodeMinKey位置。
        */
        int j = 0;
        for (int i = this->leafNodeMinKey; i <= p->keyNumber; i++, j++)
        {
            r->key[j] = p->key[i];
            r->ptr[j] = p->ptr[i];
            ((BTreeNode<KeyType, ValueType> *) (r->ptr[j]))->parent = r;//更新父结点
            p->ptr[i] = NULL;
        }
        //在左边结点中插入key,将要插入位置之后的key和ptr进行后移。
        int i = -1;
        for (i = this->leafNodeMinKey - 1; i > 0; i--)
        {
            if (key < p->key[i])
            {
                p->key[i + 1] = p->key[i];
                p->ptr[i + 1] = p->ptr[i];
            } else
            {
                //此时key要插入的位置就是i+1处
                break;
            }
        }
        //插入key
        p->key[i + 1] = key;
        p->ptr[i + 1] = ptr;
        ptr->parent = p;//更新父结点
        p->keyNumber = this->leafNodeMinKey;
        //确定右边结点的key数量(相对叶子结点分裂多减去1)
        r->keyNumber = this->n + 1 - p->keyNumber - 1;
    } else
    {
        //从leafMinKey+1位置开始往r结点中复制(复制过程中顺便插入key)
        int j = 0;
        int flag = 0;
        for (int i = this->leafNodeMinKey + 1; i <= p->keyNumber; i++)
        {
            if (p->key[i] > key)
            {
                //插入key
                r->key[j] = key;
                r->ptr[j] = ptr;
                ptr->parent = r;
                j++;
                flag = 1;
            }
            r->key[j] = p->key[i];
            r->ptr[j] = p->ptr[i];
            ((BTreeNode<KeyType, ValueType> *) (r->ptr[j]))->parent = r;//更新父结点
            p->ptr[i] = NULL;
            j++;
        }
        if (!flag)
        {
            //没有插入key，说明key是最大的，所以继续插入key即可
            r->key[j] = key;
            r->ptr[j] = ptr;
            ptr->parent = r;
        }
        //确定左边结点的key数量
        p->keyNumber = this->leafNodeMinKey;
        //确定右边结点的key数量(相比叶子结点分裂多减去1)
        r->keyNumber = this->n + 1 - p->keyNumber - 1;
    }
    return r;
}

//向一个指定的结点中插入key。
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::insert2node(BTreeNode<KeyType, ValueType> *p, KeyType key, ValueType value)
{
    //如果结点未满
    if (p->keyNumber < n)
    {
        this->insert2notFullNode(p, key, value);
    } else
    {
        //结点满了，分裂p结点。
        BTreeNode<KeyType, ValueType> *r = this->splitNode(p, key, value);
        //向父结点插入key
        if (p->parent)
        {
            //分裂的不是根结点
            this->insertToInnerNode(p->parent, r->key[1], r);
        } else
        {
            //分裂的是根结点
            BTreeNode<KeyType, ValueType> *newRoot = new BTreeNode<KeyType, ValueType>(this->n, false);
            newRoot->keyNumber = 1;
            newRoot->key[1] = r->key[1];
            newRoot->ptr[0] = p;
            newRoot->ptr[1] = r;
            p->parent = newRoot;//更新父亲
            r->parent = newRoot;//更新父亲
            this->root = newRoot;
        }
    }
}

//向一个指定的未满结点中插入key
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::insert2notFullNode(BTreeNode<KeyType, ValueType> *p, KeyType key, ValueType value)
{
    //首先将要插入位置之后的key和ptr进行后移。
    int i = -1;
    for (i = p->keyNumber; i > 0; i--)
    {
        if (key < p->key[i])
        {
            p->key[i + 1] = p->key[i];
            p->value[i + 1] = p->value[i];
            //p->ptr[i + 1] = p->ptr[i];
        } else
        {
            //此时key>p->key[i](等于的情况不存在,等于即已经存在key,不会执行到这。)
            //此时key要插入的位置就是i+1处
            break;
        }
    }
    //插入key
    p->key[i + 1] = key;
    p->value[i + 1] = value;
    p->keyNumber++;
}

/**
@desc:向一个指定的内部结点中插入key。
*/
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::insertToInnerNode(BTreeNode<KeyType, ValueType> *p, KeyType key,
                                                  BTreeNode<KeyType, ValueType> *ptr)
{
    //如果该内部结点未满
    if (p->keyNumber < n)
    {
        this->insertToNotFullInnerNode(p, key, ptr);
    } else
    {
        //结点满了，分裂p结点。
        BTreeNode<KeyType, ValueType> *r = this->splitInnerNode(p, key, ptr);
        //向父结点插入key
        if (p->parent)
        {
            //分裂的不是根结点
            this->insertToInnerNode(p->parent, r->key[0], r);
        } else
        {
            //分裂的是根结点
            BTreeNode<KeyType, ValueType> *newRoot = new BTreeNode<KeyType, ValueType>(this->n, false);
            newRoot->keyNumber = 1;
            newRoot->key[1] = r->key[0];
            newRoot->ptr[0] = p;
            newRoot->ptr[1] = r;
            p->parent = newRoot;//更新父亲
            r->parent = newRoot;//更新父亲
            this->root = newRoot;
        }

    }
}

/**
@desc:向一个指定的未满内部结点中插入key。
*/
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::insertToNotFullInnerNode(BTreeNode<KeyType, ValueType> *p, KeyType key,
                                                         BTreeNode<KeyType, ValueType> *ptr)
{
    //首先将要插入位置之后的key和ptr进行后移。
    int i = -1;
    for (i = p->keyNumber; i > 0; i--)
    {
        if (key < p->key[i])
        {
            p->key[i + 1] = p->key[i];
            p->ptr[i + 1] = p->ptr[i];
        } else
        {
            //此时key>p->key[i](等于的情况不存在,等于即已经存在key,不会执行到这。)
            //此时key要插入的位置就是i+1处
            break;
        }
    }
    //插入key
    p->key[i + 1] = key;
    p->ptr[i + 1] = ptr;
    p->keyNumber++;
}

/*
@desc:删除索引树中的key。
@return:如果找不到要删除的key，返回false。
否则删除key，并且返回true。
*/
template<class KeyType, class ValueType>
bool BTree<KeyType, ValueType>::del(int key)
{
    BTreeNode<KeyType, ValueType> *p = NULL;//创建一个结点指针用于查询key的时候存储返回结点指针。
    int ret = this->find(p, key);
    if (ret == -1 || p == NULL)
    {
        return false;//异常情况
    }
    if (!ret)
    {
        //can not find
        return false;
    }
    //如果删除之后结点还够最少数量。(或者特例：只有一个根结点叶子的情况也可以直接删除。)
    if (p->keyNumber >= this->leafNodeMinKey + 1 || p->parent == NULL)
    {
        return this->del_direct_index(p, ret);
    } else if (p->keyNumber == this->leafNodeMinKey)
    {
        //如果删除之后结点不够最少数量。
        BTreeNode<KeyType, ValueType> *pp = p->parent;//父结点
        BTreeNode<KeyType, ValueType> *left_partner = NULL;//指向求助对象结点
        BTreeNode<KeyType, ValueType> *right_partner = NULL;//
        //试图从右边借(从右边借方便找，因为可以根据p->ptr[0]找到右边结点)
        right_partner = (BTreeNode<KeyType, ValueType> *) (p->ptr[0]);
        //判断是否存在右兄弟，并且right_partner是否是右亲兄弟。
        if (right_partner && pp == right_partner->parent)
        {
            //是亲兄弟，借。(注意，如果是亲兄弟，那么right_partner->key[0] == pp->key[对应的那个key]，因为right_partner是右边，必然不会是某个子树的第一个结点，只有第一个结点才有可能出现[第一个key在父亲中不存在的情况]。)
            //如果有兄弟key多余最少key数量则借。
            if (right_partner->keyNumber >= this->leafNodeMinKey + 1)
            {
                //找到父结点中对应于right_partner->key[1]的位置。
                int pos = this->indexOf(pp, right_partner->key[1]);
                //删除本结点中的key
                this->del_direct_index(p, ret);
                //向本结点中添加从right_partner中借过来的key。
                p->keyNumber++;
                p->key[p->keyNumber] = right_partner->key[1];
                p->value[p->keyNumber] = right_partner->value[1];
                //p->ptr[p->keyNumber] = right_partner->ptr[1];
                //删除right_partner中被借走的key(这个过程不可能分裂，既然跟它借，它就是数量够，不会再去借也不合再合并)
                del_direct_index(right_partner, 1);
                //至此，整个删除结束，所以直接退出。
                return true;
            }
            //else {//此处表示有右亲兄弟但是没有足够数量key导致不能借，预留，如果需要后边可能需要被合并。}
        } else
        {
            //如果right_partner!=NULL，而没进去上一个if，说明((right_partner && pp == right_partner->parent))不成立，即有右兄弟但不是亲兄弟。
            //如果right_partner==NULL，没有右兄弟。
            //总之，进入else代表没有右亲兄弟 --> 必然有左亲兄弟。

            //没有右亲兄弟的右兄弟，没用，所以设置为NULL。
            right_partner = NULL;
        }

        //所以只能是试图去找左兄弟(通过判断父结点中是否含有p->key[1]可以知道是否有左兄弟)。
        //因为，一个子树中只有第一个孩子是无左兄弟，也只有第一个孩子的key[1]不在父结点中。(注意这句话只在叶子结点中成立)

        //呀呀呀呀呀呀呀个呸的--->>>重大错误，找了好久，本身此处的查找仅仅是为了确定是否是第一个孩子，而调用find则是递归p子树中是否存在指定key，但实际情况是pp本身就是p的父亲，当然能找到。此处永远能找到，其他逻辑上错误自然就一堆堆来了。
        //int splitKey = this->find(pp, p->key[1]);
        int splitKey = this->indexOf(pp, p->key[1]);
        if (splitKey != -1)
        {
            //如果不是0，说明能找到，此处不需要判断是否是亲兄弟，肯定是亲的。
            //splitKey位置指针为p，splitKey-1位置指针即为左兄弟。
            //但是需要判断该兄弟是否有足够的key能够借。
            left_partner = (BTreeNode<KeyType, ValueType> *) (pp->ptr[splitKey - 1]);
            if (left_partner->keyNumber >= this->leafNodeMinKey + 1)
            {
                //可以借。
                //先删除p结点的ret位置key(直接删除，不借也不合并)。
                this->del_direct_index(p, ret);
                //更新父结点中 p->key[1] --> left_partner->key[left_partner->keyNumber]
                this->replace(pp, p->key[1], left_partner->key[left_partner->keyNumber]);
                //然后从左兄弟中借一个过来。
                this->insert2notFullNode(p, left_partner->key[left_partner->keyNumber],
                                         left_partner->value[left_partner->keyNumber]);
                left_partner->keyNumber--;
                //至此，整个删除又结束了。
                return true;
            }//else {//此处表示有左亲兄弟但是没有足够数量key导致不能借，预留，如果需要后边可能需要被合并。}
        } else
        {
            //p是父亲的第一个孩子，也就是没有左亲兄弟。
            left_partner = NULL;
        }

        //如果到达这里，说明左右兄弟都没能成功借到。
        //但是注意，不可能两个兄弟都不存在，最多一个不存在，另一个不够借。
        //所以合并即可。
        if (!left_partner && !right_partner)
        {
            //异常情况，即左右亲兄弟都没有。
            cout << "异常，我竟然一个亲兄弟都没有！" << endl;
            return false;
        }

        //有一个亲兄弟
        //开始删除以及合并
        if (left_partner)
        {
            //cout<< "合并左兄弟结点" <<endl;
            //合并左兄弟
            //将本结点删除指定key后剩余的key全部复制到左结点中。
            //本结点中key比左结点中所有key都大，所以可以顺序复制过去。
            //删除（直接删除）
            this->del_direct_index(p, ret);
            //复制本结点中key到左边结点中。
            int j = left_partner->keyNumber + 1;
            for (int i = 1; i <= p->keyNumber; i++, j++)
            {
                left_partner->key[j] = p->key[i];
                left_partner->value[j] = p->value[i];
                //left_partner->ptr[j] = p->ptr[i];
            }
            //更新左结点的key的数量。
            left_partner->keyNumber += p->keyNumber;
            //更新叶子结点直接的链接。
            left_partner->ptr[0] = p->ptr[0];
            //删除父结点中的key=p->key[1]的那个key。
            this->del_inner_node(pp, p->key[1]);
            //释放p结点空间
            delete p;
            return true;
        } else
        {
            //合并右兄弟
            //cout << "合并右兄弟结点" << endl;
            //将右兄弟结点全部key复制到当前结点中。
            //右兄弟结点中key比当前结点中所有key都大，所以可以顺序复制。

            //删除（直接删除）
            this->del_direct_index(p, ret);

            //复制右兄弟结点中key到当前边结点中。
            int j = p->keyNumber + 1;
            for (int i = 1; i <= right_partner->keyNumber; i++, j++)
            {
                p->key[j] = right_partner->key[i];
                p->value[j] = right_partner->value[i];
                //p->ptr[j] = right_partner->ptr[i];
            }
            //更新当前结点的key的数量。
            p->keyNumber += right_partner->keyNumber;
            //更新叶子结点直接的链接。
            p->ptr[0] = right_partner->ptr[0];

            //删除父结点中的key=right_partner->key[1]的那个key。
            this->del_inner_node(pp, right_partner->key[1]);

            //释放p结点空间
            delete right_partner;

            return true;
        }
    } else
    {
        cout << "invalid keyNumber: " << p->keyNumber << endl;;
        return false;
    }
}

//删除内部结点的指定key
template<class KeyType, class ValueType>
bool BTree<KeyType, ValueType>::del_inner_node(BTreeNode<KeyType, ValueType> *p, int key)
{
    int ret = this->indexOf(p, key);
    if (ret == -1)
    {
        //没找到
        return false;
    } else
    {
        //找到
        if (p->keyNumber >= this->innerNodeMinKey + 1)
        {
            //可以直接删除
            this->del_inner_direct_index(p, ret);
            return true;
        }

        //当前结点是根结点的情况。
        if (p->parent == NULL)
        {
            if (p->keyNumber > 1)
            {
                //还有一个以上结点
                //直接删除即可。
                this->del_inner_direct_index(p, ret);
                return true;
            } else
            {
                //只剩下一个结点了
                //这种情况需要删除根结点即可，将唯一的一个孩子作为新的根结点。
                BTreeNode<KeyType, ValueType> *newRoot = (BTreeNode<KeyType, ValueType> *) (p->ptr[0]);
                newRoot->parent = NULL;//清除新的根结点的parent
                delete this->root;
                this->root = newRoot;
                return true;
            }
        }
        //删除之后结点数量会不够(需要借或者合并)
        //非根结点
        BTreeNode<KeyType, ValueType> *pp = p->parent;
        BTreeNode<KeyType, ValueType> *left_partner = NULL;
        BTreeNode<KeyType, ValueType> *right_partner = NULL;
        //首先获得当前结点在父结点中的位置
        int pos = this->indexInParent(p);
        //cout << "当前结点在父结点中位置为 pos = "<<pos << endl;
        //注意pos不等于-1，因为上边已经将p为根结点的情况给处理掉了。
        if (pos > 0)
        {
            //pos!=0 存在左边兄弟
            //拿到左兄弟指针
            left_partner = (BTreeNode<KeyType, ValueType> *) (pp->ptr[pos - 1]);
            if (left_partner->keyNumber >= this->innerNodeMinKey + 1)
            {
                //左边兄弟够借
                //先直接删除
                this->del_inner_direct_index(p, ret);
                /*	借
                @1:将p的父结点中对应的key作为p结点的一个key加入
                @2:用左兄弟的最大key替换父结点中p对应的那个key。
                @3:将左兄弟的最右ptr作为p结点的最左边的ptr加入。
                */
                //即添加父结点p对应的key和 左兄弟的最大key对应的ptr到p中
                //父结点p对应的 key = pp->key[pos] ; 左兄弟的最大key = left_partner->key[left_partner->keyNumber] ;最右 ptr = left_partner->ptr[left_partner->keyNumber]。
                //@1将本结点所有内容向右移动
                int i = p->keyNumber;
                while (i >= 0)
                {
                    p->key[i + 1] = p->key[i];
                    p->ptr[i + 1] = p->ptr[i];
                    i--;
                }
                //添加
                p->key[1] = pp->key[pos];
                //更新p结点的key数量
                p->keyNumber++;

                //@2:用左兄弟的最大key替换父结点中p对应的那个key。
                pp->key[pos] = left_partner->key[left_partner->keyNumber];

                //@3:将左兄弟的最右ptr作为p结点的最左边的ptr加入。
                p->ptr[0] = left_partner->ptr[left_partner->keyNumber];
                ((BTreeNode<KeyType, ValueType> *) (left_partner->ptr[left_partner->keyNumber]))->parent = p;//维护新的父亲关系

                //更新左兄弟结点的key数量
                left_partner->keyNumber--;

                //经过观察，父亲关系以及更新，叶子结点的顺序则不需要管，因为没有影响到（画图很直观能看出来）。

                //结束
                return true;
            }
            //else {//左兄弟不够借}
        } else
        {
            //pos = 0，即p是第一个孩子，没有左兄弟。
            left_partner = NULL;
        }
        //判断是否有右兄弟
        if (pos < pp->keyNumber)
        {
            //pos < pp->keyNumber 存在右边兄弟
            //拿到右兄弟指针
            right_partner = (BTreeNode<KeyType, ValueType> *) (pp->ptr[pos + 1]);
            if (right_partner->keyNumber >= this->innerNodeMinKey + 1)
            {
                //右兄弟key的数量够借
                //先直接删除
                this->del_inner_direct_index(p, ret);
                /*	借
                @1:将p的父结点中对应的key右边一个key作为p结点的一个key加入
                @2:用右兄弟的最小key替换父结点中p对应的那个key的右边的key。
                @3:将右兄弟的最左ptr作为p结点的最右边的ptr加入。
                */
                //即添加父结点p对应的key右边的key 和 右兄弟中最左边的ptr到p的最右边。
                //父结点p对应的 key的右边key = pp->key[pos+1] ; 右兄弟的最小 key = right_partner->key[1] ;最左 ptr = right_partner->ptr[0]。
                //@1添加
                //更新p结点的key数量
                p->keyNumber++;
                //keyNumber已经加了1，所以keyNumber位置已经是最右边位置。
                p->key[p->keyNumber] = pp->key[pos + 1];

                //@2:用右兄弟的最小key替换父结点中p对应的那个key。
                pp->key[pos + 1] = right_partner->key[1];

                //@3:将右兄弟的最左ptr作为p结点的最右边的ptr加入。
                p->ptr[p->keyNumber] = right_partner->ptr[0];

                ((BTreeNode<KeyType, ValueType> *) (right_partner->ptr[0]))->parent = p;//维护新的父亲关系

                //将右兄弟结点的剩余所有key和ptr左移（因为最小的一个已经没了）
                //注意虽然第一个结点key不需要左移，但是还是从i=1开始，因为这样可以顺便把ptr[0]左移
                for (int i = 1; i <= right_partner->keyNumber; i++)
                {
                    right_partner->key[i - 1] = right_partner->key[i];
                    right_partner->ptr[i - 1] = right_partner->ptr[i];
                }
                //更新右兄弟结点的key数量
                right_partner->keyNumber--;

                //经过观察，父亲关系以及更新，叶子结点的顺序则不需要管，因为没有影响到（画图很直观能看出来）。

                //结束
                return true;
            }
            //else {//右兄弟不够借}
        } else
        {
            //pos = pp->keyNumber，即p是最后一个孩子，没有右兄弟。
            right_partner = NULL;
        }

        //有一个亲兄弟
        //开始删除以及合并
        if (left_partner)
        {
            //cout<< "合并左兄弟结点" <<endl;
            //合并左兄弟
            //将本结点删除指定key后剩余的key全部复制到左结点中。
            //本结点中key比左结点中所有key都大，所以可以顺序复制过去。

            //删除（直接删除）
            this->del_inner_direct_index(p, ret);

            //复制本结点中key/ptr到左边结点中。
            //先将本结点对应到父结点中那个可以放到本结点的第0个位置，这样复制的时候可以统一点（本身父结点中那个key就是要首先放到左兄弟中最右边，接下来才是放本结点的剩余key的）。
            p->key[0] = pp->key[pos];
            int j = left_partner->keyNumber + 1;
            for (int i = 0; i <= p->keyNumber; i++, j++)
            {
                left_partner->key[j] = p->key[i];
                left_partner->ptr[j] = p->ptr[i];
                ((BTreeNode<KeyType, ValueType> *) (left_partner->ptr[j]))->parent = left_partner;//更新父亲关系
            }
            //更新左结点的key的数量(注意还有一个0号位置，所以需要加1，0号位置实质是从父结点中拿来的)。
            left_partner->keyNumber += (p->keyNumber + 1);

            //删除父结点中的key=p->key[0]的那个key，也可以说是 pp->key[pos]。
            this->del_inner_node(pp, pp->key[pos]);

            //释放p结点空间
            delete p;

            return true;
        } else
        {
            //合并右兄弟
            //cout << "合并右兄弟结点" << endl;
            //将右兄弟结点全部key复制到当前结点中。
            //右兄弟结点中key比当前结点中所有key都大，所以可以顺序复制。

            //删除（直接删除）
            this->del_inner_direct_index(p, ret);

            //复制右兄弟结点中key到当前边结点中(包括父结点中一个)。
            right_partner->key[0] = pp->key[pos + 1];
            int j = p->keyNumber + 1;
            for (int i = 0; i <= right_partner->keyNumber; i++, j++)
            {
                p->key[j] = right_partner->key[i];
                p->ptr[j] = right_partner->ptr[i];
                ((BTreeNode<KeyType, ValueType> *) (p->ptr[j]))->parent = p;
            }
            //更新当前结点的key的数量。
            p->keyNumber += (right_partner->keyNumber + 1);

            //删除父结点中的key=right_partner->key[0]的那个key，也可以说是 pp->key[pos+1]。
            this->del_inner_node(pp, pp->key[pos + 1]);

            //释放p结点空间
            delete right_partner;

            return true;
        }

    }
}

//在指定结点中种查找关键字key的位置。
//	返回p在其父亲中位置i;
//	若p无父亲，返回-1.
template<class KeyType, class ValueType>
int BTree<KeyType, ValueType>::indexInParent(BTreeNode<KeyType, ValueType> *p)
{
    if (p->parent == NULL)
        return -1;
    BTreeNode<KeyType, ValueType> *pp = p->parent;

    int i = pp->keyNumber;
    while (i > 0 && p->key[1] < pp->key[i])
    {
        i--;
    }
    //i==0也对，i==0是最左边孩子的情况。
    return i;
}


/*
@desc:	删除指定结点中的指定index的key。
@return:	删除成功返回true。
否则返回false。
*/
template<class KeyType, class ValueType>
bool BTree<KeyType, ValueType>::del_direct_index(BTreeNode<KeyType, ValueType> *p, int index)
{
    int delKey = p->key[index];
    //如果找到了，则p所指向的即为key所在结点，ret则是key所在位置。
    //循环将[ret+1 --> keyNumber]位置的key和value前移。
    for (int i = index + 1; i <= p->keyNumber; i++)
    {
        p->key[i - 1] = p->key[i];
        p->value[i - 1] = p->value[i];
        //p->ptr[i - 1] = p->ptr[i];
    }
    p->keyNumber--;
    //如果删除的是第一个位置，则有可能需要替换父结点(key-->p->ptr[ret])。
    //非叶子结点不会出现需要更新父结点的情况(注意此处判断是为了保险，实质程序中内部结点不会调用这个方法)。
    if (p->isLeaf && index == 1)
    {
        this->replace(p->parent, delKey, p->key[1]);
    }
    return true;
}

/*
	@desc:	删除指定内部结点中的指定index的key。
	@return:	删除成功返回true。
	否则返回false。
*/
template<class KeyType, class ValueType>
bool BTree<KeyType, ValueType>::del_inner_direct_index(BTreeNode<KeyType, ValueType> *p, int index)
{
    int delKey = p->key[index];
    //如果找到了，则p所指向的即为key所在结点，ret则是key所在位置。
    //循环将[ret+1 --> keyNumber]位置的key和ptr前移。
    for (int i = index + 1; i <= p->keyNumber; i++)
    {
        p->key[i - 1] = p->key[i];
        p->ptr[i - 1] = p->ptr[i];
    }
    p->keyNumber--;
    return true;
}

/**
@desc:从p结点以前一直向上找到根结点，直到找到oldKey并且替换为newKey，或直到根结点找不到也结束。
*/
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::replace(BTreeNode<KeyType, ValueType> *p, KeyType oldKey, KeyType newKey)
{
    int hasFind = 0;
    //如果还没找到 并且 没到根结点。
    while (!hasFind && p)
    {
        //查找和替换
        for (int i = 1; i <= p->keyNumber; i++)
        {
            if (p->key[i] == oldKey)
            {
                p->key[i] = newKey;//替换
                hasFind = 1;//标记为找到
                break;
            }
        }
        p = p->parent;
    }

}

/**
@desc:打印B树(方便调试)
为了看起来方便，需要层次遍历。
*/
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::print()
{
    if (this->root == NULL)
    {
        cout << "BTree is empty！" << endl;
        return;
    }

    cout << "BTree输出：" << endl;
    //定义一个队列
    queue < BTreeNode<KeyType, ValueType> * > pnodeQueue;
    queue<int> keyNumberQueue;

    //根结点进队
    pnodeQueue.push(this->root);

    //node临时数量(用于打印BTree的结构，这样打印会比较直观)
    int nodeNumber = 0;//记录当前本层遍历数
    int nodeNumbers = 1;//记录本层结点总数
    int newNodeNumbers = 0;//记录下一层结点总数

    //开始层次遍历
    while (pnodeQueue.size())
    {
        //获取队首结点，并且出队。
        BTreeNode<KeyType, ValueType> *pNode = pnodeQueue.front();
        pnodeQueue.pop();
        if (pNode)
        {
            //打印该结点的keys
            this->printNode(pNode);
            if (!pNode->isLeaf)
            {
                //将该结点的所有孩子结点指针入队。
                for (int i = 0; i <= pNode->keyNumber; i++)
                {
                    if (pNode->ptr[i])
                        pnodeQueue.push((BTreeNode<KeyType, ValueType> *) (pNode->ptr[i]));
                }
                newNodeNumbers += pNode->keyNumber + 1;
            }
            nodeNumber++;
            if (nodeNumber == nodeNumbers)
            {
                cout << endl;
                keyNumberQueue.push(newNodeNumbers);
                newNodeNumbers = 0;
                if (!keyNumberQueue.empty())
                {
                    //重新计数
                    nodeNumber = 0;
                    nodeNumbers = keyNumberQueue.front();
                    keyNumberQueue.pop();
                }
            }
        }
    }
    cout << "BTree输出完毕！" << endl;
}

//打印一个结点的key
template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::printNode(BTreeNode<KeyType, ValueType> *pNode)
{
    int isFirst = 1;
    cout << "(";
    //遍历该结点
    for (int i = 1; i <= pNode->keyNumber; i++)
    {
        if (isFirst)
        {
            cout << pNode->key[i];
            isFirst = 0;
        } else
        {
            cout << ", " << pNode->key[i];
        }

    }
    cout << ") ";
}

//返回树的第一个叶子结点。
template<class KeyType, class ValueType>
BTreeNode<KeyType, ValueType> *BTree<KeyType, ValueType>::firstLeafNode()
{
    BTreeNode<KeyType, ValueType> *p = this->root;
    while (p && !p->isLeaf)
    {
        p = (BTreeNode<KeyType, ValueType> *) (p->ptr[0]);
    }
    return p;
}

template<class KeyType, class ValueType>
void BTree<KeyType, ValueType>::printAllLeaf()
{
    cout << "打印叶子结点：" << endl;
    BTreeNode<KeyType, ValueType> *p = this->firstLeafNode();
    while (p)
    {
        cout << "(";
        //遍历p
        for (int i = 1; i <= p->keyNumber; i++)
        {
            cout << p->key[i] << "=" << p->value[i] << " ";
        }
        cout << ") ";
        p = (BTreeNode<KeyType, ValueType> *) (p->ptr[0]);
    }
    cout << endl << "打印叶子结点结束！" << endl;
}

