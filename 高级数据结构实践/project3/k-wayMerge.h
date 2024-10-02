#pragma once
#ifndef K_WAY_MERGE_H
#define K_WAY_MERGE_H
#include<iostream>
#include<queue>
#include"../Gdefine.h"
#include"../project2/LoserTree.h"
using namespace std;

struct KWayMerge
{
	FileProcessor* fp;//源文件处理器
	FileProcessor* runfiles;//k个runfile文件，用于读取文件
	Buf* obuf1, * obuf2;//输出缓冲区
	queue<Buf*>* kq;//归并段输入缓冲区队列
	queue<Buf*>* bufPool;//空闲缓冲区队列
	//LoserTree for merge k runs
	//LoserTree for deciding which runfile to read into free buffer in bufPool
	int toReadRunfile;//将要添加缓冲区的
};

//根据文件数据类型的不同，创建相对应的loser tree
void* createDiffTypeLoserTree(int datatype);

//获取下一个将要读的runfile队列的队列号
int getToReadRunfile(void* losertree, int type);

//
void treadRead(int& toReadRunfile, KWayMerge& kwm);

//自己根据当前MAIN TREAD操作的输出文件号创建输出文件，并检测是否需要创建一个新的文件
void treadWrite(int& runfileMaxNum, KWayMerge& kwm);

void mergeKRunfiles();

void kMergePass();

void kMerge();

#endif // !K_WAY_MERGE_H
