#include <cassert>
#include "../fileprocess/FileProcessor.h"
#include <vector>
#include <string>
#include <algorithm>
#include<iostream>
using namespace std;

//总生成的runfile数量的最大索引，用于生成不同名的runfile文件
unsigned long long hisRun = 0;

struct project1 {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcessor* fp;//用来打开源文件
    FileProcessor** runfile;//源文件切割成为不同的run文件
    //FileProcessor* ofp;//
    //bool hasRead[];//表示当前runfile是否读完或
    unsigned long long runAmount;
}p;

void freePstruct()
{
    if (p.input1)
        delete p.input1;
    if (p.input2)
        delete p.input2;
    if (p.output)
        delete p.output;
    if (p.fp)
        delete p.fp;
    if (p.runfile)
    {
        for (int i = 0; i < p.runAmount; ++i)
        {
            if (p.runfile[i])
                delete p.runfile[i];
        }
        delete[] p.runfile;
    }
}

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

void initGlobal()
{
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
}


//被initP()函数调用，用于生成并排序初始run文件
void creatInitRuns()
{
    // 假设最大run文件数量
    size_t maxRuns = (p.fp->dataAmount % p.input1->size == 0)? 
        (p.fp->dataAmount / p.input1->size) : (p.fp->dataAmount / p.input1->size) + 1;
    p.runfile = new FileProcessor*[maxRuns];  // 动态分配FileProcesser指针数组

    //当前run的索引
    int runIndex = 0;

    int readState = CONTINUE;
    while (readState == CONTINUE && runIndex < maxRuns) {
        readState = p.fp->readfile2buffer(*(p.input1));
        if (readState == CONTINUE)
        {
            // 写入有序小文件，文件名“run_[index].dat”
            std::string runFile = "run_" + std::to_string(runIndex) + ".dat";
            //注意释放
            p.runfile[runIndex] = new FileProcessor(runFile.c_str());

            // 对缓冲区内的数据进行排序
            p.input1->bufInternalSort();

            //写文件前需要更新，大小
            p.runfile[runIndex]->dataAmount = p.input1->actualSize;
            p.runfile[runIndex]->writebuffer2file(*(p.input1));  // 将buffer写入run文件

            runIndex++;
        }
        else
        {
            switch (readState)
            {
            case DONE:
                cout << "Done spliting" << endl; break;
            case ERR:
                cerr << "ERR reading file" << endl; break;
            default:
                break;
            }
        }
    }

    //runIndex目前相当于runAmount
    hisRun = runIndex;

    p.runAmount = maxRuns;  // 存储生成的run文件数量，不是索引
}

void initP(size_t intputBufSize, size_t outputBufSize)
{
    p.input1 = new Buf(INPUT_BUF, intputBufSize);
    p.input2 = new Buf(INPUT_BUF, intputBufSize);
    p.output = new Buf(OUTPUT_BUF, outputBufSize);
    p.fp = new FileProcessor();
    //p.ofp = new FileProcessor("res.dat");

    //一致化三个缓冲区的编码
    p.fp->loadMetaDataAndMallocBuf(*(p.input1));
    p.input2->setEncodingAndMalloc(p.input1->encoding);
    p.output->setEncodingAndMalloc(p.input1->encoding);

    initGlobal();

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
                obuf_i16[output->pos++] = buf1_i16[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            if (output->pos)
            obuf_i16[output->pos++] = buf1_i16[input1->pos++];
            input1->actualSize--;
        }
        else if (input2->actualSize > 0)
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
                obuf_i32[output->pos++] = buf1_i32[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i32[output->pos++] = buf1_i32[input1->pos++];
            input1->actualSize--;
        }
        else if (input2->actualSize > 0)
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
                obuf_i64[output->pos++] = buf1_i64[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i64[output->pos++] = buf1_i64[input1->pos++];
            input1->actualSize--;
        }
        else if (input2->actualSize > 0)
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
                obuf_d[output->pos++] = buf1_d[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_d[output->pos++] = buf1_d[input1->pos++];
            input1->actualSize--;
        }
        else if (input2->actualSize > 0)
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
                obuf_f[output->pos++] = buf1_f[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_f[output->pos++] = buf1_f[input1->pos++];
            input1->actualSize--;
        }
        else if (input2->actualSize > 0)
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


//将缓冲区中的数据合并并写入文件中
void mergeBuf(Buf*& input1, Buf*& input2, Buf*& output, FileProcessor*& newRun)
{
    int enc = input1->encoding;
    //三个缓冲区的编码类型应该相同
    assert(input1->encoding == input2->encoding && input1->encoding == output->encoding);

    //两个输入缓冲区的实际数据量
    int totalCount = input1->actualSize + input2->actualSize;

    //inputbuffer不空，或是outputbuffer没满
    while (input1->actualSize > 0 || input2->actualSize > 0)
    {
        compareOnceAndPut(input1, input2, output);
        if (output->actualSize >= output->size) {
            newRun->writebuffer2file(*output);
            output->actualSize = 0;  // 重置 output 的大小，准备写入更多数据
        }
    }

    //无论如何先写一次
    if (output->actualSize > 0)
        newRun->writebuffer2file(*output);

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
        newRun->writebuffer2file(*output);
}

//将两个runfile合并，并产生一个新的runfile
FileProcessor* mergeRunfile(FileProcessor*& run1, FileProcessor*& run2)
{
    // 写入有序小文件，文件名“run_[index].dat”
    string runFile = "run_" + std::to_string(hisRun++) + ".dat";
    //注意释放
    FileProcessor* newRun = new FileProcessor(runFile.c_str());

    newRun->dataAmount = run1->dataAmount + run2->dataAmount;

    //將两个文件中的内容分别读入缓冲区
    //并做归并
    int res1 = CONTINUE, res2 = CONTINUE;
    do
    {
        if(res1 == CONTINUE)
            res1 = run1->readfile2buffer(*(p.input1));
        if (res1 != CONTINUE && res1 != DONE) {
            cerr << "Failed to read file " << 1 << " into buffer." << endl;
            exit(1);
        }
        else
            p.input1->pos = 0;
        if (res2 == CONTINUE)
            res2 = run2->readfile2buffer(*(p.input2));
        if (res2 != CONTINUE && res2 != DONE) {
            cerr << "Failed to read file " << 2 << " into buffer." << endl;
            exit(1);
        }
        else
            p.input2->pos = 0;

        mergeBuf(p.input1, p.input2, p.output, newRun);

        //newRun->directLoadDataSet();
    } while (res1 != DONE || res2 != DONE);

    return newRun;
}

//一轮合并，更新p中的runfile和runAmount属性
int mergePass()
{
    if (p.runAmount < 2)
    {
        cout << "merge done !" << endl;
        return OK;
    }

    //创建新的runfile，计算新的runAmount
    unsigned long long newRunIndex = 0;
    size_t maxRuns = (p.runAmount % 2 == 0) ?
        (p.runAmount / 2) : (p.runAmount / 2) + 1;
    FileProcessor** newRunfile = new FileProcessor * [maxRuns];  // 动态分配FileProcesser指针数组

    // 从所有 run 文件中进行二路归并
    unsigned long long i = 0;
    for (i = 0; i < p.runAmount - 1; i += 2) {
        FileProcessor* newRun = mergeRunfile(p.runfile[i], p.runfile[i + 1]);  // 执行合并
        newRunfile[newRunIndex++] = newRun;
    }

    //合并完后续工作
    if (p.runAmount % 2 != 0)
        newRunfile[newRunIndex] = p.runfile[p.runAmount - 1];

    //释放原runfile空间
    for (int j = 0; j < p.runAmount; ++j)
    {
        //不释放最后一个
        if (j == p.runAmount - 1 && p.runAmount % 2 != 0)
            break;

        char* runfileName = newString(p.runfile[j]->filename);

        //释放原先创建的FileProcesser
        delete p.runfile[j];

        //删除旧的runfile文件,不检查有没有删除成功
        if(remove(runfileName) != 0) {
            cerr << "can not remove file :" << runfileName << endl;
        }

        free(runfileName);

        //cout << "free run_" << j << endl;
    }

    delete[] p.runfile;

    p.runAmount = maxRuns;
    p.runfile = newRunfile;

    return MERGE;
}

void externalMerge()
{
    int flag = MERGE;
    do {
        flag = mergePass();
    } while (flag != OK);

    //释放p结构体
    freePstruct();

    //重命名结果文件
    string filename = "run_" + std::to_string(hisRun - 1) + ".dat";
    remove("result.dat");
    //改名前需要释放对应文件的fileProcessor
    if (rename(filename.c_str(), "result.dat") != 0) {
        perror("Error renaming file");
    }

    FileProcessor file("result.dat");
    file.directLoadDataSet();
}

//#define EXTENAL_2WAYMERGE_MAIN
#ifndef EXTENAL_2WAYMERGE_MAIN
int main() {

    initP(1,20);
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

    externalMerge();

    return 0;
}
#endif // !EXTENAL_2WAYMERGE_MAIN


