#pragma once
#ifndef EXTERNAL_2_WAY_MERGE_H
#define EXTERNAL_2_WAY_MERGE_H
#include <cassert>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include "../Gdefine.h"
using namespace std;

extern struct project p;

//总生成的runfile数量的最大索引，用于生成不同名的runfile文件
extern unsigned long long hisRun;

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

#endif // !EXTERNAL_2_WAY_MERGE_H
