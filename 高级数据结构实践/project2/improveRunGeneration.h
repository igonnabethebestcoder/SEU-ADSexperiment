#pragma once
#ifndef IMPROVE_RUN_GEN_H
#define IMPROVE_RUN_GEN_H
#include <iostream>
#include <thread>
#include "LoserTree.h"
#include "../project1/ex2waymerge.h"
#include "../global/Gdefine.h"
#include "LoserTree.h"
#include <mutex>
#include <queue>
using namespace std;

//�߳�д�ļ�
void threadWriteFile(project& p, int& workingState, long& curRunfile);

//�̶߳��ļ�
void threadReadFile(int& activeBuf, project& p);

//������ͬ��С��runfile,��Gdefine������
extern void createDiffLenRuns(project& p, int k);

//ʹ�û��������ϲ�runfile,����result.dat�ļ�
void huffmanMerge();

//������ͬ�鲢���ļ���runfile���ĺ���
void genDiffRunfile(project& p, int inputBufSize, int outputBufSize, int k, const char* filename);

#endif // !IMPROVE_RUN_GEN_H
