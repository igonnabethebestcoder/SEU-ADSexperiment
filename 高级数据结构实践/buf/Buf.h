#pragma once
#ifndef BUF_H
#define BUF_H
#include<iostream>
#include"../Gdefine.h"
using namespace std;
/*
 * 缓冲区默认大小，无单位
 */
#define DEFAULT_INPUT_BUF_SIZE 10
#define DEFAULT_OUTPUT_BUF_SIZE 10


/*
 * 缓冲区类型，对应不同的写入操作
 */
#define INPUT_BUF 0
#define OUTPUT_BUF 1


/// <summary>
/// 输入输出通用缓冲区
/// 1.支持内部排序
/// 2.支持不同类型数据读取和写入文件
/// 3.
/// </summary>
class Buf
{
public:
	Buf(){}
	Buf(int type, long long size = 0);//
	~Buf();
	void setEncodingAndMalloc(int enc);//设置编码，并分配内存空间
	void bufInternalSort();//ok, 将数据从文件读入缓冲区后进行内部排序
	static size_t getEncodingSize(int enc);


public:
	size_t size;//缓冲区可容纳数据量大小

	size_t actualSize;//缓冲区实际存放数据的个数

	int type;//缓冲区类型,检查是否合法

	int encoding;//数据类型编码,文件头元数据包含信息

	void* buffer;//数据存储区域,可能需要sds

	off_t pos;//当前buffer的偏移量
};

#endif // !BUF_H