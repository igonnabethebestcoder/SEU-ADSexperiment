#include "Buf.h"
#include<algorithm>

Buf::Buf(int type, long long size)
{
	this->size = 0;
	//size���Ϸ�
	if (size <= 0)
	{
		cerr << "Illegal buffer size!" << endl;
		exit(1);
	}
	
	//buf���Ͳ��Ϸ�
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

	//Ȼ����Ҫ����readfile
	//��ȡ�ļ���������ENC


	//��ǰĬ����int
	buffer = malloc(sizeof(int) * size);
}

Buf::~Buf()
{
    if (buffer != nullptr)
	    free(buffer);
}

/// <summary>
/// ��ʱֻ֧�������ڲ�����
/// </summary>
void Buf::bufInternalSort()
{
    if (buffer == nullptr) {
        std::cout << "Buffer is empty, no data to sort!" << std::endl;
        return;
    }

    switch (encoding) {
    case ENC_INT16: {
        // �� void* buffer ת��Ϊ int16_t* ����
        int16_t* buf = static_cast<int16_t*>(buffer);
        std::sort(buf, buf + size); // �Ի�������������
        break;
    }
    case ENC_INT32: {
        // �� void* buffer ת��Ϊ int32_t* ����
        int32_t* buf = static_cast<int32_t*>(buffer);
        std::sort(buf, buf + size); // �Ի�������������
        break;
    }
    case ENC_INT64: {
        // �� void* buffer ת��Ϊ int64_t* ����
        int64_t* buf = static_cast<int64_t*>(buffer);
        std::sort(buf, buf + size); // �Ի�������������
        break;
    }
    case ENC_FLOAT: {
        // �� void* buffer ת��Ϊ float* ����
        float* buf = static_cast<float*>(buffer);
        std::sort(buf, buf + size); // �Ի�������������
        break;
    }
    case ENC_DOUBLE: {
        // �� void* buffer ת��Ϊ double* ����
        double* buf = static_cast<double*>(buffer);
        std::sort(buf, buf + size); // �Ի�������������
        break;
    }
    case ENC_STRING: {
        // �� void* buffer ת��Ϊ char** ���飨�����ַ������� char* �洢�ģ�
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

    //bufferObj.bufInternalSort();  // Ӧ�ö� int32_t �����������

    //for (int i = 0; i < 5; i++) {
    //    std::cout << data[i] << " ";  // �����1 3 5 7 9
    //}

    return 0;
}
#endif // !BUF_TEST
