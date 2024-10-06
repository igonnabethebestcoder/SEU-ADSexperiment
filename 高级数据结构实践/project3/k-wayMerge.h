#pragma once
#ifndef K_WAY_MERGE_H
#define K_WAY_MERGE_H
#include <iostream>
#include <queue>
#include "../global/Gdefine.h"
#include "../project2/LoserTree.h"
#include "../project2/improveRunGeneration.h"
using namespace std;

struct KWayMerge
{
	FileProcessor* fp;//源文件处理器

	int runfilesSize;
	FileProcessor** runfiles;//k个runfile文件，用于读取文件
	vector<bool> *readDone;

	Buf* obuf1, * obuf2;//输出缓冲区

	int kqSize;//一般情况为k
	queue<Buf*>** kq;//归并段输入缓冲区队列，被禁赛的为nullptr

	queue<Buf*>* bufPool;//空闲缓冲区队列

	int k;
	//LoserTree for merge k runs
	//LoserTree for deciding which runfile to read into free buffer in bufPool
	int curRunfileNum;//当前拥有的runfile的数量
	int maxRunfileNum;//当前拥有的最大的runfile号
	int maxOpRunfileNum;//当前已操作的最大的runfile号
};

//每k个runfile合并完调用，runfilesSize属性需要动态调整
void freeCurKRunfiles(KWayMerge& kwm);

//初始化新的k个runfile的合并
void anotherKRunfilesAndLoadQ(KWayMerge& kwm);

//初始化kWayMerge结构体
void initkwm(KWayMerge& kwm, int& maxRunfileNum,int inputBufSize, int outputBufSize, int k, const char* filename);

//释放kwm结构体
void freekwm(KWayMerge& kwm);

//根据文件数据类型的不同，创建相对应的loser tree
void* createDiffTypeLoserTree(vector<void*>& input, int datatype = ENC_INT32);

//获取下一个将要读的runfile队列的队列号
int getToReadRunfile(void* losertree, int type = ENC_INT32);

//
void threadRead(int& toReadRunfile, KWayMerge& kwm);

//计算当前kwm.runfiles中应该写入的数据量，同步到curRunfileSize中
void countDataAmount(KWayMerge& kwm, uint64_t& curRunfileSize);

//自己根据当前MAIN TREAD操作的输出文件号创建输出文件，并检测是否需要创建一个新的文件
void threadWrite(int& runfileMaxNum, KWayMerge& kwm, int& activeBuf);

//负责一轮内k个文件的合并
void mergeKRunfiles(KWayMerge& kwm);

//负责一轮所有文件的合并
int kMergePass(KWayMerge& kwm);

//负责多轮合并
void kMerge(KWayMerge& kwm);

#endif // !K_WAY_MERGE_H
