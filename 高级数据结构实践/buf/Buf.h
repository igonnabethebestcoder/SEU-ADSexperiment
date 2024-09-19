#pragma once
#ifndef BUF_H
#define BUF_H
#include<iostream>
#include"../Gdefine.h"
using namespace std;
/*
 * ������Ĭ�ϴ�С���޵�λ
 */
#define DEFAULT_INPUT_BUF_SIZE 10
#define DEFAULT_OUTPUT_BUF_SIZE 10


/*
 * ���������ͣ���Ӧ��ͬ��д�����
 */
#define INPUT_BUF 0
#define OUTPUT_BUF 1

/*
 * �ļ������������Ͷ���
 */
typedef int (*readfile)(const char*);//param��filename
typedef int (*writefile)(const char*);//param��filename


/// <summary>
/// �������ͨ�û�����
/// 1.֧���ڲ�����
/// 2.֧�ֲ�ͬ�������ݶ�ȡ��д���ļ�
/// 3.
/// </summary>
class Buf
{
public:
	Buf(){}
	Buf(int type, long long size = 0);//
	~Buf();
	void bufInternalSort();//ok, �����ݴ��ļ����뻺����������ڲ�����


public:
	size_t size;//��������С

	int type;//����������,����Ƿ�Ϸ�

	int encoding;//�������ͱ���,�ļ�ͷԪ���ݰ�����Ϣ

	void* buffer;//���ݴ洢����,������Ҫsds

	off_t pos;//��ǰbuffer��ƫ����



	//ͨ�ú���ָ��
	readfile readfilefuc;
	writefile writefilefunc;
};

#endif // !BUF_H