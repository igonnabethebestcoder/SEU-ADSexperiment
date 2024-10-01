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
 * 程序是否正常宏定义
 */
#define OK 0 //表示状态正常或是合并完成
#define ERR -1 //表示错误
#define MERGE 1 //表示还需要继续合并
#endif // !PROGRAM_STATE

/*
 * 文件操作函数类型定义
 */
typedef int (*readfile)(const char*);//param是filename
typedef int (*writefile)(const char*);//param是filename


/*
 * 文件数据段字节偏移
 */
#define DATASESSION_OFFSET 26 //前26字节是文件元数据，[triomaxbuf][version][enc][count]

/*
 * 读文件标识
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
	int loadMetaDataAndMallocBuf(Buf& buf);//读入文件元数据，并根据数据类型编码分配给buf指定空间
	int readfile2buffer(Buf& buf);//读入数据，并更新buf的actualSize
	int writebuffer2file(Buf& buf);//将buf中的数据写入文件中
	int updateMetaDataAmount(uint64_t da);//更新runfile数据量大小，在生成不同长度的归并段中使用

	//生成目标文件函数
	int saveData2File(int32_t* data, size_t size);//opt,目前只支持32bits

	//测试文件读取正确性函数
	int loadFile();//测试文件使用
	int directLoadDataSet();//测试文件使用

public:
	char* filename;
	off_t getp, putp;//文件偏移
	fstream file; //可以对相同的文件进行读和写，部分覆盖
	//ifstream infile;  // 输入文件流，保持文件打开状态
	//ofstream outfile; // 输出文件流，保持文件打开状态
	uint64_t dataAmount;
};
#endif // !FILE_PROCESSER_H
