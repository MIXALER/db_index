//为了实验方便,全部使用public数据。
//每个记录100字节大小。
class Record
{
public:
    //作为关键字
    int key = -1;
    //value
    char content[96] = "";

    Record();

    Record(int key);
};

Record::Record()
{
}

Record::Record(int key)
{
    this->key = key;
}

