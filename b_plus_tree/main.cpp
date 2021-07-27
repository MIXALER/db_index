#include <stdio.h>
#include <iostream>
#include "BTreeIndex.h"
#include "Record.h"

void printMenu();

//function declares
//可以忽略了，这些都是测试用的。请直接运行程序就好。
void t_empty();

void t_1_to_10();

void t_btree4();

void t_del();

int main()
{
    //t_empty();
    //t_add();
    //t_1_to_10();
    //t_btree4();
    //t_del();

    DataBase<Record> db("dbFile");

    //定义一个BTree索引数据表。
    BTreeIndex<int, Record> indexDB(3);
    //数据记录，用于存放输入的临时数据。
    Record record;
    //用于存放输入的临时数据。
    int key;
    //打印命令。
    printMenu();
    //命令号
    int cmd = -1;
    //接受命令。
    while (cin >> cmd)
    {
        switch (cmd)
        {
            //1:新增数据 格式（key value）
            case 1:
                cout << "please input key and value:";
                cin >> record.key >> record.content;
                indexDB.add(record.key, record);
                break;
                //2:删除数据 格式（key）
            case 2:
                cout << "please input key:";
                cin >> key;
                indexDB.del(key);
                break;
                //3:查询数据 格式（key）
            case 3:
                cout << "please input key:";
                cin >> key;
                record.key = -1;
                indexDB.find(key, record);
                if (record.key != -1)
                {
                    cout << "Record[key=" << record.key << ",content=" << record.content << "]" << endl;
                } else
                {
                    cout << "Record with key=" << key << " is not found！" << endl;
                }
                break;
                //4:打印B树索引" << endl;
            case 4:
                indexDB.getBTree()->print();
                break;
            case 5:
                indexDB.getBTree()->printAllLeaf();
                break;
            default:
                cout << "invalid cmmand!" << endl;
                break;
        }
        printMenu();
    }
    return 0;
}


//打印功能菜单
void printMenu()
{
    cout << endl;
    cout << "1:新增数据 格式（key value）;" << endl;
    cout << "2:删除数据 格式（key）;" << endl;
    cout << "3:查询数据 格式（key）;" << endl;
    cout << "4:打印B树索引" << endl;
    cout << "5:打印叶子结点" << endl;
}

void t_del()
{
    BTree<int, int> btree(3);
    for (int i = 1; i <= 3; i++)
    {
        btree.add(10 * i, 10 * i);
        btree.printAllLeaf();
    }
    btree.add(25, 25);
    btree.printAllLeaf();
    btree.add(45, 45);
    btree.printAllLeaf();
    btree.add(61, 61);
    btree.printAllLeaf();
    btree.add(63, 63);
    btree.printAllLeaf();
    btree.add(65, 65);
    btree.printAllLeaf();

    btree.add(64, 64);
    btree.printAllLeaf();

    btree.add(62, 62);
    btree.printAllLeaf();

    btree.add(82, 82);
    btree.printAllLeaf();
    btree.add(84, 84);
    btree.printAllLeaf();
    btree.add(86, 86);
    btree.printAllLeaf();
    btree.add(88, 88);
    btree.printAllLeaf();

    btree.add(85, 85);
    cout << "删除65之前-------------------------------------" << endl;
    btree.print();
    btree.printAllLeaf();
    cout << "删除65之后-------------------------------------" << endl;
    btree.del(65);
    btree.print();
    btree.printAllLeaf();

    //合并左结点
    //b_plus_tree.del(100);
    //b_plus_tree.print();
    //b_plus_tree.printAllLeaf();

    //合并右结点
    btree.del(85);
    btree.print();
    btree.printAllLeaf();
    btree.del(80);
    btree.print();
    btree.printAllLeaf();

    //构造导致父结点也合并的情况


    //内部结点合并并且通过借结点实现（向左借）
    btree.del(61);
    btree.print();
    btree.printAllLeaf();

    //内部结点合并并且通过借结点实现（向右借）
    btree.del(50);
    btree.print();
    btree.printAllLeaf();

    btree.del(60);
    btree.print();
    btree.printAllLeaf();

    //内部结点合并并且通过合并实现(与左边兄弟合并)
    //b_plus_tree.del(88);
    //b_plus_tree.print();
    //b_plus_tree.printAllLeaf();

    //内部结点合并并且通过合并实现(与右边兄弟合并)
    btree.del(20);
    btree.print();
    btree.printAllLeaf();
    btree.del(25);
    btree.print();
    btree.printAllLeaf();

    btree.del(30);
    btree.print();
    btree.printAllLeaf();

    //不断删除，知道多层递归删除
    btree.del(10);
    btree.print();

    btree.del(40);
    btree.print();

    btree.del(62);
    btree.print();

    btree.del(86);
    btree.print();
    btree.printAllLeaf();


    //b_plus_tree.del();
    //b_plus_tree.print();

    //b_plus_tree.printAllLeaf();

}

void t_btree4()
{
    BTree<int, int> btree(4);
    for (int i = 1; i <= 13; i++)
    {
        btree.add(i, i);
    }
    btree.print();
}

void t_1_to_10()
{
    BTree<int, int> btree(3);
    for (int i = 1; i <= 10; i++)
    {
        btree.add(i, i);
    }
    btree.print();
}

void t_add()
{
    BTree<int, int> btree(3);
    btree.print();

    btree.add(4, 4);
    btree.print();

    btree.add(5, 5);
    btree.print();

    btree.add(8, 8);
    btree.print();

    btree.add(7, 7);
    btree.print();

    btree.add(2, 2);
    btree.print();

    btree.add(12, 12);
    btree.print();

    btree.add(10, 10);
    btree.print();

    btree.add(6, 6);
    btree.print();

    btree.add(3, 3);
    btree.print();

    btree.add(9, 9);
    btree.print();

    btree.add(20, 20);
    btree.print();

    btree.add(30, 30);
    btree.print();

    btree.add(1, 1);
    btree.print();

    btree.add(0, 0);
    btree.print();

    btree.add(-1, -1);
    btree.print();

}

void t_empty()
{
    BTree<int, int> btree(3);
    btree.print();
}
