#pragma once
#ifndef EXTERNAL_2_WAY_MERGE_H
#define EXTERNAL_2_WAY_MERGE_H
#include <cassert>
#include "../fileprocess/FileProcessor.h"
#include <vector>
#include <string>
#include <algorithm>
#include<iostream>
using namespace std;

struct project1 {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcessor* fp;//������Դ�ļ�
    FileProcessor** runfile;//Դ�ļ��и��Ϊ��ͬ��run�ļ�
    unsigned long long runAmount;//��ǰmerge pass�Ĺ鲢�θ���
    //FileProcessor* ofp;//
    //bool hasRead[];//��ʾ��ǰrunfile�Ƿ�����
};

//�����ɵ�runfile����������������������ɲ�ͬ����runfile�ļ�
extern unsigned long long hisRun;

extern struct project1 p;

//�ͷ�p�ṹ��
void freePstruct();

//��ʼ��ȫ��ָ�����
void initGlobal();

//����鲢��
void creatInitRuns();

//��ʼ��p�ṹ��
void initP(size_t inputBufSize, size_t outputBufSize);

//����һ�αȽϣ������������output���������
void compareOnceAndPut(Buf*& input1, Buf*& input2, Buf*& output);

//�ϲ�����input������
void mergeBuf(Buf*& input1, Buf*& input2, Buf*& output, FileProcessor*& newRun);

//�ϲ�����runfile��������һ���ϲ����runfile
FileProcessor* mergeRunfile(FileProcessor*& run1, FileProcessor*& run2);

//a merge pass�� �ϲ���ǰ����runfile�������µ�һ��merge pass
int mergePass();

//���ⲿ��·�鲢���������ڣ����ɽ���ļ�result.dat
void externalMerge();


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

#endif // !EXTERNAL_2_WAY_MERGE_H
