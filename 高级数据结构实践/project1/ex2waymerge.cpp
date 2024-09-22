#include <cassert>
#include "../fileprocess/FileProcessor.h"
#include <vector>
#include <string>
#include <algorithm>
#include<iostream>
using namespace std;

//�����ɵ�runfile����������������������ɲ�ͬ����runfile�ļ�
unsigned long long hisRun = 0;

struct project1 {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcessor* fp;//������Դ�ļ�
    FileProcessor** runfile;//Դ�ļ��и��Ϊ��ͬ��run�ļ�
    //FileProcessor* ofp;//
    //bool hasRead[];//��ʾ��ǰrunfile�Ƿ�����
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

//��initP()�������ã��������ɲ������ʼrun�ļ�
void creatInitRuns()
{
    // �������run�ļ�����
    size_t maxRuns = (p.fp->dataAmount % p.input1->size == 0)? 
        (p.fp->dataAmount / p.input1->size) : (p.fp->dataAmount / p.input1->size) + 1;
    p.runfile = new FileProcessor*[maxRuns];  // ��̬����FileProcesserָ������

    //��ǰrun������
    int runIndex = 0;

    int readState = CONTINUE;
    while (readState == CONTINUE && runIndex < maxRuns) {
        readState = p.fp->readfile2buffer(*(p.input1));
        if (readState == CONTINUE)
        {
            // д������С�ļ����ļ�����run_[index].dat��
            std::string runFile = "run_" + std::to_string(runIndex) + ".dat";
            //ע���ͷ�
            p.runfile[runIndex] = new FileProcessor(runFile.c_str());

            // �Ի������ڵ����ݽ�������
            p.input1->bufInternalSort();

            //д�ļ�ǰ��Ҫ���£���С
            p.runfile[runIndex]->dataAmount = p.input1->actualSize;
            p.runfile[runIndex]->writebuffer2file(*(p.input1));  // ��bufferд��run�ļ�

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

    //runIndexĿǰ�൱��runAmount
    hisRun = runIndex;

    p.runAmount = maxRuns;  // �洢���ɵ�run�ļ���������������
}

void initP(size_t intputBufSize, size_t outputBufSize)
{
    p.input1 = new Buf(INPUT_BUF, intputBufSize);
    p.input2 = new Buf(INPUT_BUF, intputBufSize);
    p.output = new Buf(OUTPUT_BUF, outputBufSize);
    p.fp = new FileProcessor();
    //p.ofp = new FileProcessor("res.dat");

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
                obuf_i32[output->pos++] = buf1_i32[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i32[output->pos++] = buf1_i32[input1->pos++];
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
                obuf_i64[output->pos++] = buf1_i64[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_i64[output->pos++] = buf1_i64[input1->pos++];
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
                obuf_d[output->pos++] = buf1_d[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_d[output->pos++] = buf1_d[input1->pos++];
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
                obuf_f[output->pos++] = buf1_f[input1->pos++];
                input1->actualSize--;
            }
        }
        else if (input1->actualSize > 0)
        {
            obuf_f[output->pos++] = buf1_f[input1->pos++];
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


//���������е����ݺϲ���д���ļ���
void mergeBuf(Buf*& input1, Buf*& input2, Buf*& output, FileProcessor*& newRun)
{
    int enc = input1->encoding;
    //�����������ı�������Ӧ����ͬ
    assert(input1->encoding == input2->encoding && input1->encoding == output->encoding);

    //�������뻺������ʵ��������
    int totalCount = input1->actualSize + input2->actualSize;

    //inputbuffer���գ�����outputbufferû��
    while (input1->actualSize > 0 || input2->actualSize > 0)
    {
        compareOnceAndPut(input1, input2, output);
        if (output->actualSize >= output->size) {
            newRun->writebuffer2file(*output);
            output->actualSize = 0;  // ���� output �Ĵ�С��׼��д���������
        }
    }

    //���������дһ��
    if (output->actualSize > 0)
        newRun->writebuffer2file(*output);

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
        newRun->writebuffer2file(*output);
}

//������runfile�ϲ���������һ���µ�runfile
FileProcessor* mergeRunfile(FileProcessor*& run1, FileProcessor*& run2)
{
    // д������С�ļ����ļ�����run_[index].dat��
    string runFile = "run_" + std::to_string(hisRun++) + ".dat";
    //ע���ͷ�
    FileProcessor* newRun = new FileProcessor(runFile.c_str());

    newRun->dataAmount = run1->dataAmount + run2->dataAmount;

    //�������ļ��е����ݷֱ���뻺����
    //�����鲢
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
    } while (res1 != DONE || res2 != DONE);

    return newRun;
}

//һ�ֺϲ�������p�е�runfile��runAmount����
int mergePass()
{
    if (p.runAmount < 2)
    {
        cout << "merge done !" << endl;
        return OK;
    }

    //�����µ�runfile�������µ�runAmount
    unsigned long long newRunIndex = 0;
    size_t maxRuns = (p.runAmount % 2 == 0) ?
        (p.runAmount / 2) : (p.runAmount / 2) + 1;
    FileProcessor** newRunfile = new FileProcessor * [maxRuns];  // ��̬����FileProcesserָ������

    // ������ run �ļ��н��ж�·�鲢
    unsigned long long i = 0;
    for (i = 0; i < p.runAmount - 1; i += 2) {
        FileProcessor* newRun = mergeRunfile(p.runfile[i], p.runfile[i + 1]);  // ִ�кϲ�
        newRunfile[newRunIndex++] = newRun;
    }

    //�ϲ����������
    if (p.runAmount % 2 != 0)
        newRunfile[newRunIndex] = p.runfile[p.runAmount - 1];

    //�ͷ�ԭrunfile�ռ�
    for (int j = 0; j < p.runAmount; ++j)
    {
        //���ͷ����һ��
        if (j == p.runAmount - 1 && p.runAmount % 2 != 0)
            break;

        char* runfileName = newString(p.runfile[j]->filename);

        //�ͷ�ԭ�ȴ�����FileProcesser
        delete p.runfile[j];

        //ɾ���ɵ�runfile�ļ�,�������û��ɾ���ɹ�
        if(remove(runfileName) != 0) {
            cerr << "can not remove file :" << runfileName << endl;
        }

        free(runfileName);

        cout << "free run_" << j << endl;
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

    //�ͷ�p�ṹ��
    freePstruct();

    //����������ļ�
    string filename = "run_" + std::to_string(hisRun - 1) + ".dat";
    remove("result.dat");
    //����ǰ��Ҫ�ͷŶ�Ӧ�ļ���fileProcessor
    if (rename(filename.c_str(), "result.dat") != 0) {
        perror("Error renaming file");
    }

    FileProcessor file("result.dat");
    file.directLoadDataSet();
}

//#define EXTENAL_2WAYMERGE_MAIN
#ifndef EXTENAL_2WAYMERGE_MAIN
int main() {

    initP(20,20);
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

    externalMerge();

    return 0;
}
#endif // !EXTENAL_2WAYMERGE_MAIN


