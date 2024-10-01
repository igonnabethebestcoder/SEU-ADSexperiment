#pragma once
#ifndef FILE_PROCESSER_H
#define FILE_PROCESSER_H
#include <cstdint>
#include <fstream>
#include "../buf/Buf.h"
#include<iostream>
#include<mutex>
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
 * �ļ������������Ͷ���
 */
typedef int (*readfile)(const char*);//param��filename
typedef int (*writefile)(const char*);//param��filename


/*
 * �ļ����ݶ��ֽ�ƫ��
 */
#define DATASESSION_OFFSET 26 //ǰ26�ֽ����ļ�Ԫ���ݣ�[triomaxbuf][version][enc][count]

/*
 * ���ļ���ʶ
 */
#define CONTINUE 1
#define DONE 2

extern unsigned long long ioReadCount;
extern unsigned long long ioWriteCount;
extern mutex ioReadMtx, ioWriteMtx;

class FileProcessor
{
public:
	FileProcessor(const char* filename = "temp.dat");
	~FileProcessor();
	int loadMetaDataAndMallocBuf(Buf& buf);//�����ļ�Ԫ���ݣ��������������ͱ�������bufָ���ռ�
	int readfile2buffer(Buf& buf);//�������ݣ�������buf��actualSize
	int writebuffer2file(Buf& buf);//��buf�е�����д���ļ���
	int updateMetaDataAmount(uint64_t da);//����runfile��������С�������ɲ�ͬ���ȵĹ鲢����ʹ��

	//����Ŀ���ļ�����
	int saveData2File(int32_t* data, size_t size);//opt,Ŀǰֻ֧��32bits

	//�����ļ���ȡ��ȷ�Ժ���
	int loadFile();//�����ļ�ʹ��
	int directLoadDataSet();//�����ļ�ʹ��

public:
	char* filename;
	off_t getp, putp;//�ļ�ƫ��
	fstream file; //���Զ���ͬ���ļ����ж���д�����ָ���
	//ifstream infile;  // �����ļ����������ļ���״̬
	//ofstream outfile; // ����ļ����������ļ���״̬
	uint64_t dataAmount;
};
#endif // !FILE_PROCESSER_H
