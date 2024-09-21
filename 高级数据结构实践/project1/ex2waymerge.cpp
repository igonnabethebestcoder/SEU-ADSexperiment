#include <cassert>
#include "../fileprocess/FileProcesser.h"
#include <vector>
#include <string>
#include <algorithm>
#include<iostream>
using namespace std;

struct project1 {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcesser* fp;//������Դ�ļ�
    FileProcesser** runfile;//Դ�ļ��и��Ϊ��ͬ��run�ļ�
    //bool hasRead[];//��ʾ��ǰrunfile�Ƿ�����
    unsigned long long runAmount;
}p;

int16_t* buf1_i16 = nullptr;
int16_t* buf2_i16 = nullptr;
int16_t* obuf_i16 = nullptr;
int32_t* buf1_i32 = nullptr;
int32_t* buf2_i32 = nullptr;
int32_t* obuf_i32 = nullptr;
int64_t* buf1_i64 = nullptr;
int64_t* buf2_i64 = nullptr;
int64_t* obuf_i64 = nullptr;
float* buf1_f = nullptr;
float* buf2_f = nullptr;
float* obuf_f = nullptr;
double* buf1_d = nullptr;
double* buf2_d = nullptr;
double* obuf_d = nullptr;

//��initP()�������ã��������ɲ������ʼrun�ļ�
void creatInitRuns()
{
    // �������run�ļ�����
    size_t maxRuns = (p.fp->dataAmount % p.input1->size == 0)? 
        (p.fp->dataAmount / p.input1->size) : (p.fp->dataAmount / p.input1->size) + 1;
    p.runfile = new FileProcesser*[maxRuns];  // ��̬����FileProcesserָ������

    //��ǰrun������
    int runIndex = 0;

    while (p.fp->readfile2buffer(*(p.input1)) == OK && runIndex < maxRuns) {
        // д������С�ļ����ļ�����run_[index].dat��
        std::string runFile = "run_" + std::to_string(runIndex) + ".dat";
        //ע���ͷ�
        p.runfile[runIndex] = new FileProcesser(runFile.c_str());

        // �Ի������ڵ����ݽ�������
        p.input1->bufInternalSort();

        //д�ļ�ǰ��Ҫ���£���С
        p.runfile[runIndex]->dataAmount = p.input1->actualSize;
        p.runfile[runIndex]->writebuffer2file(*(p.input1));  // ��bufferд��run�ļ�
        
        runIndex++;
        if (runIndex >= maxRuns) {
            std::cerr << "Exceeded maximum number of runs!" << std::endl;
            break;
        }
    }

    p.runAmount = maxRuns;  // �洢���ɵ�run�ļ���������������
}

void initP(size_t intputBufSize, size_t outputBufSize)
{
    p.input1 = new Buf(INPUT_BUF, intputBufSize);
    p.input2 = new Buf(INPUT_BUF, intputBufSize);
    p.output = new Buf(OUTPUT_BUF, outputBufSize);
    p.fp = new FileProcesser();

    //һ�»������������ı���
    p.fp->loadMetaDataAndMallocBuf(*(p.input1));
    p.input2->setEncodingAndMalloc(p.input1->encoding);
    p.output->setEncodingAndMalloc(p.input1->encoding);

    switch (p.input1->encoding)
    {
    case ENC_INT16:
        buf1_i16 = reinterpret_cast<int16_t*>(p.input1->buffer);
        buf2_i16 = reinterpret_cast<int16_t*>(p.input2->buffer);
        obuf_i16 = reinterpret_cast<int16_t*>(p.output->buffer);
        break;
    case ENC_INT32:
        buf1_i32 = reinterpret_cast<int32_t*>(p.input1->buffer);
        buf2_i32 = reinterpret_cast<int32_t*>(p.input2->buffer);
        obuf_i32 = reinterpret_cast<int32_t*>(p.output->buffer);
        break;
    case ENC_INT64:
        buf1_i64 = reinterpret_cast<int64_t*>(p.input1->buffer);
        buf2_i64 = reinterpret_cast<int64_t*>(p.input2->buffer);
        obuf_i64 = reinterpret_cast<int64_t*>(p.output->buffer);
        break;
    case ENC_DOUBLE:
        buf1_d = reinterpret_cast<double*>(p.input1->buffer);
        buf2_d = reinterpret_cast<double*>(p.input2->buffer);
        obuf_d = reinterpret_cast<double*>(p.output->buffer);
        break;
    case ENC_FLOAT:
        buf1_f = reinterpret_cast<float*>(p.input1->buffer);
        buf2_f = reinterpret_cast<float*>(p.input2->buffer);
        obuf_f = reinterpret_cast<float*>(p.output->buffer);
        break;
    case ENC_STRING:
    default:
        cerr << "Unknow encoding type!" << endl;
        exit(1);
        break;
    }

    creatInitRuns();
}

void compareOnceAndPut(Buf*& input1, Buf*& input2, Buf*& output)
{
    //��ǰoutputbuffer����
    if (output->actualSize >= output->size)
    {
        p.fp->writebuffer2file(*output);
    }

    assert(input1->encoding == input2->encoding && input1->encoding == output->encoding && output->actualSize < output->size);

    switch (input1->encoding)
    {
    case ENC_INT16:
        if(input1->actualSize > 0 && input2->actualSize > 0)
        {
            if (buf1_i16[input1->pos] > buf2_i16[input2->pos])
            {
                obuf_i16[output->pos++] = buf2_i16[input2->pos++];
                input2->actualSize--;
            }
            else
            {
                obuf_i16[output->pos++] = buf1_i16[input2->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i16[output->pos++] = buf1_i16[input2->pos++];
            input1->actualSize--;
        }
        else
        {
            obuf_i16[output->pos++] = buf2_i16[input2->pos++];
            input2->actualSize--;
        }
        break;
    case ENC_INT32:
        if (input1->actualSize > 0 && input2->actualSize > 0)
        {
            if (buf1_i32[input1->pos] > buf2_i32[input2->pos])
            {
                obuf_i32[output->pos++] = buf2_i32[input2->pos++];
                input2->actualSize--;
            }
            else
            {
                obuf_i32[output->pos++] = buf1_i32[input2->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i32[output->pos++] = buf1_i32[input2->pos++];
            input1->actualSize--;
        }
        else
        {
            obuf_i32[output->pos++] = buf2_i32[input2->pos++];
            input2->actualSize--;
        }
        break;
    case ENC_INT64:
        if (input1->actualSize > 0 && input2->actualSize > 0)
        {
            if (buf1_i64[input1->pos] > buf2_i64[input2->pos])
            {
                obuf_i64[output->pos++] = buf2_i64[input2->pos++];
                input2->actualSize--;
            }
            else
            {
                obuf_i64[output->pos++] = buf1_i64[input2->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i64[output->pos++] = buf1_i64[input2->pos++];
            input1->actualSize--;
        }
        else
        {
            obuf_i64[output->pos++] = buf2_i64[input2->pos++];
            input2->actualSize--;
        }
        break;
    case ENC_DOUBLE:
        if (input1->actualSize > 0 && input2->actualSize > 0)
        {
            if (buf1_d[input1->pos] > buf2_d[input2->pos])
            {
                obuf_d[output->pos++] = buf2_d[input2->pos++];
                input2->actualSize--;
            }
            else
            {
                obuf_d[output->pos++] = buf1_d[input2->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_d[output->pos++] = buf1_d[input2->pos++];
            input1->actualSize--;
        }
        else
        {
            obuf_d[output->pos++] = buf2_d[input2->pos++];
            input2->actualSize--;
        }
        break;
    case ENC_FLOAT:
        if (input1->actualSize > 0 && input2->actualSize > 0)
        {
            if (buf1_f[input1->pos] > buf2_f[input2->pos])
            {
                obuf_f[output->pos++] = buf2_f[input2->pos++];
                input2->actualSize--;
            }
            else
            {
                obuf_f[output->pos++] = buf1_f[input2->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_f[output->pos++] = buf1_f[input2->pos++];
            input1->actualSize--;
        }
        else
        {
            obuf_f[output->pos++] = buf2_f[input2->pos++];
            input2->actualSize--;
        }
        break;
    case ENC_STRING:
    default:
        cerr << "Unknow encoding type!" << endl;
        exit(1);
        break;
    }
    output->actualSize++;
}

void merge(Buf*& input1, Buf*& input2, Buf*& output)
{
    int enc = input1->encoding;
    //�����������ı�������Ӧ����ͬ
    assert(input1->encoding == input2->encoding && input1->encoding == output->encoding);

    //�������뻺������ʵ��������
    int totalCount = input1->actualSize + input2->actualSize;

    //inputbuffer���գ�����outputbufferû��
    while ((input1->actualSize > 0 && input2->actualSize > 0) 
        && output->actualSize <= output->size)
    {
        compareOnceAndPut(input1, input2, output);
    }

    //���������дһ��
    if (output->actualSize > 0)
        p.fp->writebuffer2file(*output);

    //��ʣ���д�뻺��������д���ļ�
    if (input1->actualSize <= 0)
    {
        while (input2->actualSize)
            compareOnceAndPut(input1, input2, output);
    }
    else
    {
        while (input1->actualSize)
            compareOnceAndPut(input1, input2, output);
    }

    if (output->actualSize > 0)
        //opt, ��ͬrun
        p.fp->writebuffer2file(*output);
}


//#define EXTENAL_2WAYMERGE_MAIN
#ifndef EXTENAL_2WAYMERGE_MAIN
int main() {

    initP(50,50);
    cout << "--------ԭʼ����---------" << endl;
    p.fp->directLoadDataSet();
    cout << "--------ԭʼ����---------" << endl << endl;
    cout << "���ɵ�runfile���� :" << p.runAmount << endl;

    cout << "runfile����" << endl;
    for (int i = 0; i < p.runAmount; i++)
    {
        p.runfile[i]->directLoadDataSet();
        cout << endl;
    }

    return 0;
}
#endif // !EXTENAL_2WAYMERGE_MAIN


