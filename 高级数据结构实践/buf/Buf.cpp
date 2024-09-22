#include "Buf.h"
#include<algorithm>

Buf::Buf(int type, long long size)
{
    //判断buf类型
    switch (type)
    {
    case INPUT_BUF:
        this->size = DEFAULT_INPUT_BUF_SIZE;
        break;
    case OUTPUT_BUF:
        this->size = DEFAULT_OUTPUT_BUF_SIZE;
        break;
    default:
        //buf类型不合法
        cerr << "Unknow buffer type!" << endl;
        exit(1);
    }

	//size不合法
	if (size <= 0)
    {
        this->size = 0;
        cerr << "Illegal buffer size! set size default buffer size 100" << endl;
    }
    else
        this->size = size;

    if (this->size == 0) {
        this->size = (this->type == INPUT_BUF) ? DEFAULT_INPUT_BUF_SIZE : DEFAULT_OUTPUT_BUF_SIZE;
    }

	this->type = type;

    this->encoding = ENC_NOTKNOW;

    this->pos = 0;

    this->buffer = nullptr;

    this->actualSize = 0;
    //然后需要调用readfile
    //读取文件数据类型ENC


    //暂时不分配内存
    //buffer = malloc(sizeof(int) * size);
}
	
  

Buf::~Buf()
{
    if (buffer != nullptr)
	    free(buffer);
}


//设置编码，并分配内存空间
void Buf::setEncodingAndMalloc(int enc) {
    // 设置编码
    this->encoding = enc;

    //增加鲁棒性
    if (this->buffer != nullptr)
    {
        free(this->buffer);
        this->buffer = nullptr;
    }

    // 每种数据类型的大小
    size_t dataTypeSize = 0;

    // 根据不同的编码设置不同的数据类型大小
    switch (enc) {
    case ENC_STRING:
        // 假设字符串是以 char* 形式存储
        //opt,需要后续修改
        dataTypeSize = sizeof(char*);
        break;
    case ENC_INT16:
        dataTypeSize = sizeof(int16_t);
        break;
    case ENC_INT32:
        dataTypeSize = sizeof(int32_t);
        break;
    case ENC_INT64:
        dataTypeSize = sizeof(int64_t);
        break;
    case ENC_FLOAT:
        dataTypeSize = sizeof(float);
        break;
    case ENC_DOUBLE:
        dataTypeSize = sizeof(double);
        break;
    default:
        std::cerr << "Unsupported encoding type!" << std::endl;
        return; // 无效编码，直接返回
    }

    // 如果缓冲区大小为 0，设置为默认大小
    if (this->size == 0) {
        this->size = DEFAULT_INPUT_BUF_SIZE; // or OUTPUT_BUF_SIZE depending on type
    }

    // 当且仅当buffer为空时分配内存
    if(this->buffer == nullptr)
        this->buffer = malloc(this->size * dataTypeSize);

    // 如果内存分配失败，退出程序
    if (this->buffer == nullptr) {
        std::cerr << "Memory allocation failed!" << std::endl;
        exit(EXIT_FAILURE); 
    }

    // 初始化缓冲区（可选）
    memset(this->buffer, 0, this->size * dataTypeSize);
}


/// <summary>
/// 暂时只支持整型内部排序
/// </summary>
void Buf::bufInternalSort()
{
    if (buffer == nullptr) {
        std::cout << "Buffer is empty, no data to sort!" << std::endl;
        return;
    }

    switch (encoding) {
    case ENC_INT16: {
        // 将 void* buffer 转换为 int16_t* 数组
        int16_t* buf = static_cast<int16_t*>(buffer);
        std::sort(buf, buf + actualSize); // 对缓冲区进行排序
        break;
    }
    case ENC_INT32: {
        // 将 void* buffer 转换为 int32_t* 数组
        int32_t* buf = static_cast<int32_t*>(buffer);
        std::sort(buf, buf + actualSize); // 对缓冲区进行排序
        break;
    }
    case ENC_INT64: {
        // 将 void* buffer 转换为 int64_t* 数组
        int64_t* buf = static_cast<int64_t*>(buffer);
        std::sort(buf, buf + actualSize); // 对缓冲区进行排序
        break;
    }
    case ENC_FLOAT: {
        // 将 void* buffer 转换为 float* 数组
        float* buf = static_cast<float*>(buffer);
        std::sort(buf, buf + actualSize); // 对缓冲区进行排序
        break;
    }
    case ENC_DOUBLE: {
        // 将 void* buffer 转换为 double* 数组
        double* buf = static_cast<double*>(buffer);
        std::sort(buf, buf + actualSize); // 对缓冲区进行排序
        break;
    }
    case ENC_STRING: {
        // 将 void* buffer 转换为 char** 数组（假设字符串是以 char* 存储的）
        char** buf = static_cast<char**>(buffer);
        std::sort(buf, buf + actualSize, [](const char* a, const char* b) {
            return std::strcmp(a, b) < 0;
            });
        break;
    }
    default:
        std::cout << "Unsupported encoding type!" << std::endl;
        return;
    }

    std::cout << "Buffer sorted successfully!" << std::endl;
}

size_t Buf::getEncodingSize(int enc)
{
    switch (enc)
    {
    case ENC_INT16:
        return sizeof(int16_t);  // 返回 16 位整型的大小
    case ENC_INT32:
        return sizeof(int32_t);  // 返回 32 位整型的大小
    case ENC_INT64:
        return sizeof(int64_t);  // 返回 64 位整型的大小
    case ENC_FLOAT:
        return sizeof(float);    // 返回浮点型的大小
    case ENC_DOUBLE:
        return sizeof(double);   // 返回双精度浮点型的大小
    case ENC_STRING:
        //opt,如果是string那么读取方式将会不同
        //return sizeof(char);     // 字符串按照字符读取，返回 1 字节
    default:
        std::cerr << "Unknown encoding type: " << enc << std::endl;
        return 0;  // 默认返回 0，表示未知编码类型
    }
}

#define BUF_TEST
#ifndef BUF_TEST
int main(int argc, char** argv)
{
    //int32_t data[] = { 5, 3, 9, 1, 7 };
    //Buf bufferObj;
    //bufferObj.buffer = data;
    //bufferObj.size = 5;
    //bufferObj.encoding = ENC_INT32;

    //bufferObj.bufInternalSort();  // 应该对 int32_t 数组进行排序

    //for (int i = 0; i < 5; i++) {
    //    std::cout << data[i] << " ";  // 输出：1 3 5 7 9
    //}

    return 0;
}
#endif // !BUF_TEST
