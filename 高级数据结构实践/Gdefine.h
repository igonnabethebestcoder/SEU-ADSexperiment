#pragma once
#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H
#include <iostream>
#include <mutex>
#include "./fileprocess/FileProcessor.h"
#include "./buf/Buf.h"
using namespace std;

#ifndef PROGRAM_STATE
#define PROGRAM_STATE
/*
 * �����Ƿ������궨��
 */
#define OK 0 //��ʾ״̬�������Ǻϲ����
#define ERR -1 //��ʾ����
#define MERGE 1 //��ʾ����Ҫ�����ϲ�
#endif // !PROGRAM_STATE
/*
 * �ⲿ��������
 */
#define TWO_WAY 1 //�ⲿ��·�鲢����
#define HUFFMAN 2 //��ͬrun length�ϲ�
#define K_WAY 3 //ͬrun length k·�鲢
#define QUICKSORT 4 //�ⲿ��������

struct project;

 //����run����ָ��
typedef void (*generateRun)(project&);

extern unsigned long long ioReadCount;
extern unsigned long long ioWriteCount;
extern mutex ioReadMtx;
extern mutex ioWriteMtx;

struct project {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcessor* fp;//������Դ�ļ�
    FileProcessor** runfile;//Դ�ļ��и��Ϊ��ͬ��run�ļ�
    unsigned long long runAmount;//��ǰmerge pass�Ĺ鲢�θ���
    generateRun genFunc;
    //FileProcessor* ofp;//
    //bool hasRead[];//��ʾ��ǰrunfile�Ƿ�����
};

//ȫ������ָ�룬ָ��Buf.buffer
extern int16_t* buf1_i16;
extern int16_t* buf2_i16;
extern int16_t* obuf_i16;
extern int32_t* buf1_i32;
extern int32_t* buf2_i32;
extern int32_t* obuf_i32;
extern int64_t* buf1_i64;
extern int64_t* buf2_i64;
extern int64_t* obuf_i64;
extern float* buf1_f;
extern float* buf2_f;
extern float* obuf_f;
extern double* buf1_d;
extern double* buf2_d;
extern double* obuf_d;


//��ʼ��p�ṹ��
void initP(project& p, size_t inputBufSize, size_t outputBufSize, int type, const char* filename = "temp.dat");

//�ͷ�p�ṹ��
void freePstruct(project& p);

//��ʼ��ȫ��ָ�����
void initGlobal(project& p);

//����鲢��,��ex2waymerge.cpp�ж���
void creatInitRuns(project& p);

//ʹ�ð��������ɲ�ͬ���ȵ�run,��inproveRunGeneration.cpp�ж���
void createDiffLenRuns(project& p, int k);

//չʾio����
void showIOstatistic();

//ʹ�ú���Ҫע���ͷ�
inline char* newString(const char* str)
{
    size_t len = strlen(str);

    // ʹ�� malloc ��̬�����ڴ�
    char* newStr = nullptr;
    newStr = (char*)malloc(len + 1);

    if (newStr != nullptr) {
        // �� input ���ݸ��Ƶ� this->filename
        strncpy_s(newStr, len + 1, str, len);  // �޸��ĵط�: Ŀ�껺������СΪ len + 1
        newStr[len] = '\0';  // ȷ���Կ��ַ���β
    }

    return newStr;
}

#endif // !GLOBAL_DEFINE_H
