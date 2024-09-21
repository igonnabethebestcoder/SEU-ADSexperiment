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
    FileProcesser* fp;//用来打开源文件
    FileProcesser** runfile;//源文件切割成为不同的run文件
    //bool hasRead[];//表示当前runfile是否读完或
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

//被initP()函数调用，用于生成并排序初始run文件
void creatInitRuns()
{
    // 假设最大run文件数量
    size_t maxRuns = (p.fp->dataAmount % p.input1->size == 0)? 
        (p.fp->dataAmount / p.input1->size) : (p.fp->dataAmount / p.input1->size) + 1;
    p.runfile = new FileProcesser*[maxRuns];  // 动态分配FileProcesser指针数组

    //当前run的索引
    int runIndex = 0;

    while (p.fp->readfile2buffer(*(p.input1)) == OK && runIndex < maxRuns) {
        // 写入有序小文件，文件名“run_[index].dat”
        std::string runFile = "run_" + std::to_string(runIndex) + ".dat";
        //注意释放
        p.runfile[runIndex] = new FileProcesser(runFile.c_str());

        // 对缓冲区内的数据进行排序
        p.input1->bufInternalSort();

        //写文件前需要更新，大小
        p.runfile[runIndex]->dataAmount = p.input1->actualSize;
        p.runfile[runIndex]->writebuffer2file(*(p.input1));  // 将buffer写入run文件
        
        runIndex++;
        if (runIndex >= maxRuns) {
            std::cerr << "Exceeded maximum number of runs!" << std::endl;
            break;
        }
    }

    p.runAmount = maxRuns;  // 存储生成的run文件数量，不是索引
}

void initP(size_t intputBufSize, size_t outputBufSize)
{
    p.input1 = new Buf(INPUT_BUF, intputBufSize);
    p.input2 = new Buf(INPUT_BUF, intputBufSize);
    p.output = new Buf(OUTPUT_BUF, outputBufSize);
    p.fp = new FileProcesser();

    //一致化三个缓冲区的编码
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
    //当前outputbuffer已满
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
    //三个缓冲区的编码类型应该相同
    assert(input1->encoding == input2->encoding && input1->encoding == output->encoding);

    //两个输入缓冲区的实际数据量
    int totalCount = input1->actualSize + input2->actualSize;

    //inputbuffer不空，或是outputbuffer没满
    while ((input1->actualSize > 0 && input2->actualSize > 0) 
        && output->actualSize <= output->size)
    {
        compareOnceAndPut(input1, input2, output);
    }

    //无论如何先写一次
    if (output->actualSize > 0)
        p.fp->writebuffer2file(*output);

    //将剩余的写入缓冲区，并写入文件
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
        //opt, 不同run
        p.fp->writebuffer2file(*output);
}


//#define EXTENAL_2WAYMERGE_MAIN
#ifndef EXTENAL_2WAYMERGE_MAIN
int main() {

    initP(50,50);
    cout << "--------原始数据---------" << endl;
    p.fp->directLoadDataSet();
    cout << "--------原始数据---------" << endl << endl;
    cout << "生成的runfile个数 :" << p.runAmount << endl;

    cout << "runfile数据" << endl;
    for (int i = 0; i < p.runAmount; i++)
    {
        p.runfile[i]->directLoadDataSet();
        cout << endl;
    }

    return 0;
}
#endif // !EXTENAL_2WAYMERGE_MAIN


