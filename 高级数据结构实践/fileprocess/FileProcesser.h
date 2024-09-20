#pragma once
#ifndef FILE_PROCESSER_H
#define FILE_PROCESSER_H
#include <cstdint>
#include <fstream>
#include "../Gdefine.h"
#include "../buf/Buf.h"
#include<iostream>
using namespace std;

/*
 * �ļ������������Ͷ���
 */
typedef int (*readfile)(const char*);//param��filename
typedef int (*writefile)(const char*);//param��filename


/*
 * �ļ����ݶ��ֽ�ƫ��
 */
#define DATASESSION_OFFSET 26 //ǰ26�ֽ����ļ�Ԫ���ݣ�[triomaxbuf][version][enc][count]

class FileProcesser
{
public:
	FileProcesser();

	int readfile2buffer(Buf& buf);//�������ݣ�
	int writebuffer2file(Buf& buf);


	int saveData2File(int32_t* data, size_t size);//opt,Ŀǰֻ֧��32bits
	int loadFile();//�����ļ�ʹ��
	int directLoadDataSet();//�����ļ�ʹ��

public:
	const char* filename;
	off_t getp, putp;//�ļ�ƫ��
};
#endif // !FILE_PROCESSER_H
