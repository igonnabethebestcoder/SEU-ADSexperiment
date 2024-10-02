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
	FileProcessor* fp;//Դ�ļ�������
	FileProcessor* runfiles;//k��runfile�ļ������ڶ�ȡ�ļ�
	Buf* obuf1, * obuf2;//���������
	queue<Buf*>* kq;//�鲢�����뻺��������
	queue<Buf*>* bufPool;//���л���������
	//LoserTree for merge k runs
	//LoserTree for deciding which runfile to read into free buffer in bufPool
	int toReadRunfile;//��Ҫ��ӻ�������
};

//�����ļ��������͵Ĳ�ͬ���������Ӧ��loser tree
void* createDiffTypeLoserTree(int datatype);

//��ȡ��һ����Ҫ����runfile���еĶ��к�
int getToReadRunfile(void* losertree, int type);

//
void treadRead(int& toReadRunfile, KWayMerge& kwm);

//�Լ����ݵ�ǰMAIN TREAD����������ļ��Ŵ�������ļ���������Ƿ���Ҫ����һ���µ��ļ�
void treadWrite(int& runfileMaxNum, KWayMerge& kwm);

void mergeKRunfiles();

void kMergePass();

void kMerge();

#endif // !K_WAY_MERGE_H
