#include <iostream>
#include <fstream>
#include "Record.h"

using namespace std;


/**
@title:用于模拟数据库文件，封装了向文件中增加一个记录、删除记录的方法。
*/
template<class RecordType>
class DataBase
{
private:
    string dbFilePath;
    int recordSize = 0;
public:
    //构造函数
    DataBase<RecordType>(string dbFilePath);

    //增加一个记录到文件，然后返回该记录的位置。
    int save(RecordType record);

    //删除指定位置的记录。
    void remove(int offset);

    //返回指定位置的记录。
    RecordType getByOffset(int offset);

    //更改指定位置的记录
    void set(int offset, RecordType record);
};

//构造函数
template<class RecordType>
DataBase<RecordType>::DataBase(string dbFilePath)
{
    this->dbFilePath = dbFilePath;
    //记录的大小。
    this->recordSize = sizeof(RecordType);

    //测试数据文件是否存在。
    fstream fs;
    fs.open(this->dbFilePath.c_str(), ios::in | ios::binary);
    if (fs)
    {
        fs.close();
    } else
    {
        //创建数据库文件。
        fstream fs(this->dbFilePath.c_str(), ios::out | ios::binary);
        fs.close();
    }
}

template<class RecordType>
//增加一个记录到文件，然后返回该记录的位置。
int DataBase<RecordType>::save(RecordType record)
{
    int pos = -1;
    //打开文件。
    fstream fs;
    fs.open(this->dbFilePath.c_str(), ios::in | ios::out | ios::binary);
    //定位到文件的末尾。
    fs.seekp(0, ios::end);
    //记录存储该记录的开始位置。
    pos = fs.tellp();
    //写入记录。
    fs.write((char *) (&record), this->recordSize);
    //关闭文件流。
    fs.flush();
    fs.close();
    //返回该记录在文件中的开始位置。
    return pos;
}

/**
	@desc:删除指定位置的记录。
	注意，删除和普通意义删除不是很一样。
	不是说abc删除b之后为ac，而是变为a空白b。
	因为abc三个记录的位置都是被索引了的。如果直接删除b，并且将c向前靠拢的话，需要更新索引。
	所以我们不更新索引，而是清空b记录即可。
*/
template<class RecordType>
void DataBase<RecordType>::remove(int offset)
{
    //...暂时不实现删除，因为用处不大，如果仅仅是清除数据实际是没有意义的。
    //只要那块数据不被使用，清除不清除都一样。
    //重点在于这块空间的重新利用。
}

//返回指定位置的记录。
template<class RecordType>
RecordType DataBase<RecordType>::getByOffset(int offset)
{
    //用于存储临时返回值。
    Record ret;
    //打开文件。
    fstream fs;
    fs.open(this->dbFilePath.c_str(), ios::in | ios::binary);
    //定位到指定位置。
    fs.seekp(offset, ios::beg);
    //读入记录。
    fs.read((char *) (&ret), this->recordSize);
    //关闭文件流。
    fs.flush();
    fs.close();
    //返回该记录。
    return ret;
}

template<class RecordType>
void DataBase<RecordType>::set(int offset, RecordType record)
{
    //打开文件
    fstream fs;
    fs.open(this->dbFilePath.c_str(), ios::in | ios::binary);
    //定位到指定位置
    fs.seekp(offset, ios::beg);
    //更改记录
//    fs.
}