#include"Gdefine.h"

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


void freePstruct(project& p)
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

void initGlobal(project& p)
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

/// <summary>
/// ��ȡԴ�ļ��е�meta data��
/// ����ʼ��ÿ��buffer���Լ�ȫ�ֱ���
/// </summary>
/// <param name="p"></param>
/// <param name="intputBufSize"></param>
/// <param name="outputBufSize"></param>
/// <param name="type"></param>
void initP(project& p, size_t intputBufSize, size_t outputBufSize, int type)
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

    initGlobal(p);//��ʼ�����ٸı�

    switch (type)
    {
    case TWO_WAY:
        p.genFunc = creatInitRuns;
        break;
    case HUFFMAN:
        p.genFunc = createDiffLenRuns;
        break;
    case QUICKSORT:
        p.genFunc = nullptr;
        break;
    case K_WAY:
        p.genFunc = nullptr;
        break;
    default:
        p.genFunc = nullptr;
        break;
    }

    if (p.genFunc)
        p.genFunc(p);
    else
        cerr << "no generate runfile function!" << endl;
}