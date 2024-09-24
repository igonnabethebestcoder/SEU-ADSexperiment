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
    FileProcessor* fp;//用来打开源文件
    FileProcessor** runfile;//源文件切割成为不同的run文件
    unsigned long long runAmount;//当前merge pass的归并段个数
    //FileProcessor* ofp;//
    //bool hasRead[];//表示当前runfile是否读完或
};

//总生成的runfile数量的最大索引，用于生成不同名的runfile文件
extern unsigned long long hisRun;

extern struct project1 p;

//释放p结构体
void freePstruct();

//初始化全局指针变量
void initGlobal();

//创造归并段
void creatInitRuns();

//初始化p结构体
void initP(size_t inputBufSize, size_t outputBufSize);

//进行一次比较，并将结果放入output输出缓冲区
void compareOnceAndPut(Buf*& input1, Buf*& input2, Buf*& output);

//合并两个input缓冲区
void mergeBuf(Buf*& input1, Buf*& input2, Buf*& output, FileProcessor*& newRun);

//合并两个runfile，并产生一个合并后的runfile
FileProcessor* mergeRunfile(FileProcessor*& run1, FileProcessor*& run2);

//a merge pass， 合并当前所有runfile并产生新的一轮merge pass
int mergePass();

//总外部二路归并排序调用入口，生成结果文件result.dat
void externalMerge();


//全局属性指针，指向Buf.buffer
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
