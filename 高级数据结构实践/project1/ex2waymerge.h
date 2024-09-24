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

//�����ɵ�runfile����������������������ɲ�ͬ����runfile�ļ�
extern unsigned long long hisRun;

//����һ�αȽϣ������������output���������
void compareOnceAndPut(Buf*& input1, Buf*& input2, Buf*& output);

//�ϲ�����input������
void mergeBuf(Buf*& input1, Buf*& input2, Buf*& output, FileProcessor*& newRun);

//�ϲ�����runfile��������һ���ϲ����runfile
FileProcessor* mergeRunfile(FileProcessor*& run1, FileProcessor*& run2);

//a merge pass�� �ϲ���ǰ����runfile�������µ�һ��merge pass
int mergePass();

//���ⲿ��·�鲢���������ڣ����ɽ���ļ�result.dat
void externalMerge();

#endif // !EXTERNAL_2_WAY_MERGE_H
