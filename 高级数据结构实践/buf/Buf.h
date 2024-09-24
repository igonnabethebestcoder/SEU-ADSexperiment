#pragma once
#ifndef BUF_H
#define BUF_H
#include<iostream>
using namespace std;

#ifndef PROGRAM_STATE
#define PROGRAM_STATE
/*
 * 程序是否正常宏定义
 */
#define OK 0 //表示状态正常或是合并完成
#define ERR -1 //表示错误
#define MERGE 1 //表示还需要继续合并
#endif // !PROGRAM_STATE


/*
 * 缓冲区默认大小，无单位
 */
#define DEFAULT_INPUT_BUF_SIZE 10
#define DEFAULT_OUTPUT_BUF_SIZE 10


/*
 * 缓冲区需要载入的数据类型
 */
#define ENC_NOTKNOW -1
#define ENC_STRING 0
#define ENC_INT16 1
#define ENC_INT32 2
#define ENC_INT64 3
#define ENC_FLOAT 4
#define ENC_DOUBLE 5


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
	int setEncodingAndMalloc(int enc);//设置编码，并分配内存空间,返回是否要更新全局变量标志
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