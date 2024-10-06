#pragma once
#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H
#include <iostream>
#include <mutex>
#include "../fileprocess/FileProcessor.h"
#include "../buf/Buf.h"
#include "../log/log.h"
using namespace std;

#ifndef PROGRAM_STATE
#define PROGRAM_STATE
/*
 * 程序是否正常宏定义
 */
#define OK 0 //表示状态正常或是合并完成
#define ERR -1 //表示错误
#define MERGE 1 //表示还需要继续合并
#endif // !PROGRAM_STATE
/*
 * 外部排序类型
 */
#define TWO_WAY 1 //外部二路归并排序
#define HUFFMAN 2 //不同run length合并
#define K_WAY 3 //同run length k路归并
#define QUICKSORT 4 //外部快速排序

struct project;

 //产生run函数指针
typedef void (*generateRun)(project&);

extern unsigned long long ioReadCount;
extern unsigned long long ioWriteCount;
extern mutex ioReadMtx;
extern mutex ioWriteMtx;
extern Log& logger;

struct project {
    Buf* input1;
    Buf* input2;
    Buf* output;
    FileProcessor* fp;//用来打开源文件
    FileProcessor** runfile;//源文件切割成为不同的run文件
    unsigned long long runAmount;//当前merge pass的归并段个数
    generateRun genFunc;
    //FileProcessor* ofp;//
    //bool hasRead[];//表示当前runfile是否读完或
};

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

extern Log& logger;

//初始化p结构体
void initP(project& p, size_t inputBufSize, size_t outputBufSize, int type, const char* filename = "temp.dat");

//释放p结构体
void freePstruct(project& p);

//初始化全局指针变量
void initGlobal(project& p);

//创造归并段,在ex2waymerge.cpp中定义
void creatInitRuns(project& p);

//使用败者树生成不同长度的run,在inproveRunGeneration.cpp中定义
void createDiffLenRuns(project& p, int k);

//展示io次数
void showIOstatistic();

//使用后需要注意释放
inline char* newString(const char* str)
{
    size_t len = strlen(str);

    // 使用 malloc 动态分配内存
    char* newStr = nullptr;
    newStr = (char*)malloc(len + 1);

    if (newStr != nullptr) {
        // 将 input 内容复制到 this->filename
        strncpy_s(newStr, len + 1, str, len);  // 修复的地方: 目标缓冲区大小为 len + 1
        newStr[len] = '\0';  // 确保以空字符结尾
    }

    return newStr;
}

#endif // !GLOBAL_DEFINE_H
