#pragma once
#ifndef BUF_H
#define BUF_H
#include<iostream>
using namespace std;

#ifndef PROGRAM_STATE
#define PROGRAM_STATE
/*
 * �����Ƿ������궨��
 */
#define OK 0 //��ʾ״̬�������Ǻϲ����
#define ERR -1 //��ʾ����
#define MERGE 1 //��ʾ����Ҫ�����ϲ�
#endif // !PROGRAM_STATE


/*
 * ������Ĭ�ϴ�С���޵�λ
 */
#define DEFAULT_INPUT_BUF_SIZE 10
#define DEFAULT_OUTPUT_BUF_SIZE 10


/*
 * ��������Ҫ�������������
 */
#define ENC_NOTKNOW -1
#define ENC_STRING 0
#define ENC_INT16 1
#define ENC_INT32 2
#define ENC_INT64 3
#define ENC_FLOAT 4
#define ENC_DOUBLE 5


/*
 * ���������ͣ���Ӧ��ͬ��д�����
 */
#define INPUT_BUF 0
#define OUTPUT_BUF 1


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
	int setEncodingAndMalloc(int enc);//���ñ��룬�������ڴ�ռ�,�����Ƿ�Ҫ����ȫ�ֱ�����־
	void bufInternalSort();//ok, �����ݴ��ļ����뻺����������ڲ�����
	static size_t getEncodingSize(int enc);


public:
	size_t size;//��������������������С

	size_t actualSize;//������ʵ�ʴ�����ݵĸ���

	int type;//����������,����Ƿ�Ϸ�

	int encoding;//�������ͱ���,�ļ�ͷԪ���ݰ�����Ϣ

	void* buffer;//���ݴ洢����,������Ҫsds

	off_t pos;//��ǰbuffer��ƫ����
};

#endif // !BUF_H