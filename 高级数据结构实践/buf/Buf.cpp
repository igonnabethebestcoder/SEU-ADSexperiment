#include "Buf.h"
#include<algorithm>

Buf::Buf(int type, long long size)
{
	this->size = 0;
	//size不合法
	if (size <= 0)
	{
		cerr << "Illegal buffer size!" << endl;
		exit(1);
	}
	
	//buf类型不合法
	if (type != INPUT_BUF && type != OUTPUT_BUF)
	{
		cerr << "Unknow buffer type!" << endl;
		exit(1);
	}

	this->type = type;

	switch (type)
	{
	case INPUT_BUF:
		this->readfilefuc = nullptr;//opt
		break;
	case OUTPUT_BUF:
		this->writefilefunc = nullptr;//opt
		break;
	default:
		break;
	}

	//然后需要调用readfile
	//读取文件数据类型ENC


	//当前默认是int
	buffer = malloc(sizeof(int) * size);
}

Buf::~Buf()
{
    if (buffer != nullptr)
	    free(buffer);
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
        std::sort(buf, buf + size); // 对缓冲区进行排序
        break;
    }
    case ENC_INT32: {
        // 将 void* buffer 转换为 int32_t* 数组
        int32_t* buf = static_cast<int32_t*>(buffer);
        std::sort(buf, buf + size); // 对缓冲区进行排序
        break;
    }
    case ENC_INT64: {
        // 将 void* buffer 转换为 int64_t* 数组
        int64_t* buf = static_cast<int64_t*>(buffer);
        std::sort(buf, buf + size); // 对缓冲区进行排序
        break;
    }
    case ENC_FLOAT: {
        // 将 void* buffer 转换为 float* 数组
        float* buf = static_cast<float*>(buffer);
        std::sort(buf, buf + size); // 对缓冲区进行排序
        break;
    }
    case ENC_DOUBLE: {
        // 将 void* buffer 转换为 double* 数组
        double* buf = static_cast<double*>(buffer);
        std::sort(buf, buf + size); // 对缓冲区进行排序
        break;
    }
    case ENC_STRING: {
        // 将 void* buffer 转换为 char** 数组（假设字符串是以 char* 存储的）
        char** buf = static_cast<char**>(buffer);
        std::sort(buf, buf + size, [](const char* a, const char* b) {
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
