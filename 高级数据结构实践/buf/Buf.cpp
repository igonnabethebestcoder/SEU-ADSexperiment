#include "Buf.h"
#include<algorithm>

Buf::Buf(int type, long long size)
{
    //�ж�buf����
    switch (type)
    {
    case INPUT_BUF:
        this->size = DEFAULT_INPUT_BUF_SIZE;
        break;
    case OUTPUT_BUF:
        this->size = DEFAULT_OUTPUT_BUF_SIZE;
        break;
    default:
        //buf���Ͳ��Ϸ�
        cerr << "Unknow buffer type!" << endl;
        exit(1);
    }

	//size���Ϸ�
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
    //Ȼ����Ҫ����readfile
    //��ȡ�ļ���������ENC


    //��ʱ�������ڴ�
    //buffer = malloc(sizeof(int) * size);
}
	
  

Buf::~Buf()
{
    if (buffer != nullptr)
	    free(buffer);
}


//���ñ��룬�������ڴ�ռ�
void Buf::setEncodingAndMalloc(int enc) {
    // ���ñ���
    this->encoding = enc;

    // ÿ���������͵Ĵ�С
    size_t dataTypeSize = 0;

    // ���ݲ�ͬ�ı������ò�ͬ���������ʹ�С
    switch (enc) {
    case ENC_STRING:
        // �����ַ������� char* ��ʽ�洢
        //opt,��Ҫ�����޸�
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
        return; // ��Ч���룬ֱ�ӷ���
    }

    // �����������СΪ 0������ΪĬ�ϴ�С
    if (this->size == 0) {
        this->size = DEFAULT_INPUT_BUF_SIZE; // or OUTPUT_BUF_SIZE depending on type
    }

    // ���ҽ���bufferΪ��ʱ�����ڴ�
    if(this->buffer == nullptr)
        this->buffer = malloc(this->size * dataTypeSize);

    // ����ڴ����ʧ�ܣ��˳�����
    if (this->buffer == nullptr) {
        std::cerr << "Memory allocation failed!" << std::endl;
        exit(EXIT_FAILURE); 
    }

    // ��ʼ������������ѡ��
    memset(this->buffer, 0, this->size * dataTypeSize);
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
        std::sort(buf, buf + actualSize); // �Ի�������������
        break;
    }
    case ENC_INT32: {
        // �� void* buffer ת��Ϊ int32_t* ����
        int32_t* buf = static_cast<int32_t*>(buffer);
        std::sort(buf, buf + actualSize); // �Ի�������������
        break;
    }
    case ENC_INT64: {
        // �� void* buffer ת��Ϊ int64_t* ����
        int64_t* buf = static_cast<int64_t*>(buffer);
        std::sort(buf, buf + actualSize); // �Ի�������������
        break;
    }
    case ENC_FLOAT: {
        // �� void* buffer ת��Ϊ float* ����
        float* buf = static_cast<float*>(buffer);
        std::sort(buf, buf + actualSize); // �Ի�������������
        break;
    }
    case ENC_DOUBLE: {
        // �� void* buffer ת��Ϊ double* ����
        double* buf = static_cast<double*>(buffer);
        std::sort(buf, buf + actualSize); // �Ի�������������
        break;
    }
    case ENC_STRING: {
        // �� void* buffer ת��Ϊ char** ���飨�����ַ������� char* �洢�ģ�
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
        return sizeof(int16_t);  // ���� 16 λ���͵Ĵ�С
    case ENC_INT32:
        return sizeof(int32_t);  // ���� 32 λ���͵Ĵ�С
    case ENC_INT64:
        return sizeof(int64_t);  // ���� 64 λ���͵Ĵ�С
    case ENC_FLOAT:
        return sizeof(float);    // ���ظ����͵Ĵ�С
    case ENC_DOUBLE:
        return sizeof(double);   // ����˫���ȸ����͵Ĵ�С
    case ENC_STRING:
        //opt,�����string��ô��ȡ��ʽ���᲻ͬ
        //return sizeof(char);     // �ַ��������ַ���ȡ������ 1 �ֽ�
    default:
        std::cerr << "Unknown encoding type: " << enc << std::endl;
        return 0;  // Ĭ�Ϸ��� 0����ʾδ֪��������
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

    //bufferObj.bufInternalSort();  // Ӧ�ö� int32_t �����������

    //for (int i = 0; i < 5; i++) {
    //    std::cout << data[i] << " ";  // �����1 3 5 7 9
    //}

    return 0;
}
#endif // !BUF_TEST
