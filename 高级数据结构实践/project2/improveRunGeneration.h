#pragma once
#ifndef IMPROVE_RUN_GEN_H
#define IMPROVE_RUN_GEN_H
#include <iostream>
#include <thread>
#include "LoserTree.h"
#include "../project1/ex2waymerge.h"
using namespace std;

//使用败者树生成不同长度的run
void createDiffLenRuns();

//使用霍夫曼树合并runfile
void huffmanMergePass();



#endif // !IMPROVE_RUN_GEN_H
