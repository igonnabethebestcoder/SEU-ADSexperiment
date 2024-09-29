#pragma once
#ifndef IMPROVE_RUN_GEN_H
#define IMPROVE_RUN_GEN_H
#include <iostream>
#include <thread>
#include "LoserTree.h"
#include "../project1/ex2waymerge.h"
#include "../Gdefine.h"
#include "LoserTree.h"
#include <mutex>
using namespace std;

//线程写文件
void threadWriteFile(project& p, int& workingState, long& curRunfile);

//线程读文件
void threadReadFile(int& activeBuf, project& p);

//创建不同大小的runfile,在Gdefine中声明
extern void createDiffLenRuns(project& p, int k);

//使用霍夫曼树合并runfile,产生result.dat文件
void huffmanMerge();


#endif // !IMPROVE_RUN_GEN_H
