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
	FileProcessor* fp;//Դ�ļ�������

	int runfilesSize;
	FileProcessor** runfiles;//k��runfile�ļ������ڶ�ȡ�ļ�
	vector<bool> *readDone;

	Buf* obuf1, * obuf2;//���������

	int kqSize;//һ�����Ϊk
	queue<Buf*>** kq;//�鲢�����뻺�������У���������Ϊnullptr

	queue<Buf*>* bufPool;//���л���������

	int k;
	//LoserTree for merge k runs
	//LoserTree for deciding which runfile to read into free buffer in bufPool
	int curRunfileNum;//��ǰӵ�е�runfile������
	int maxRunfileNum;//��ǰӵ�е�����runfile��
	int maxOpRunfileNum;//��ǰ�Ѳ���������runfile��
};

//ÿk��runfile�ϲ�����ã�runfilesSize������Ҫ��̬����
void freeCurKRunfiles(KWayMerge& kwm);

//��ʼ���µ�k��runfile�ĺϲ�
void anotherKRunfilesAndLoadQ(KWayMerge& kwm);

//��ʼ��kWayMerge�ṹ��
void initkwm(KWayMerge& kwm, int& maxRunfileNum,int inputBufSize, int outputBufSize, int k, const char* filename);

//�ͷ�kwm�ṹ��
void freekwm(KWayMerge& kwm);

//�����ļ��������͵Ĳ�ͬ���������Ӧ��loser tree
void* createDiffTypeLoserTree(vector<void*>& input, int datatype = ENC_INT32);

//��ȡ��һ����Ҫ����runfile���еĶ��к�
int getToReadRunfile(void* losertree, int type = ENC_INT32);

//
void threadRead(int& toReadRunfile, KWayMerge& kwm);

//���㵱ǰkwm.runfiles��Ӧ��д�����������ͬ����curRunfileSize��
void countDataAmount(KWayMerge& kwm, uint64_t& curRunfileSize);

//�Լ����ݵ�ǰMAIN TREAD����������ļ��Ŵ�������ļ���������Ƿ���Ҫ����һ���µ��ļ�
void threadWrite(int& runfileMaxNum, KWayMerge& kwm, int& activeBuf);

//����һ����k���ļ��ĺϲ�
void mergeKRunfiles(KWayMerge& kwm);

//����һ�������ļ��ĺϲ�
int kMergePass(KWayMerge& kwm);

//������ֺϲ�
void kMerge(KWayMerge& kwm);

#endif // !K_WAY_MERGE_H
