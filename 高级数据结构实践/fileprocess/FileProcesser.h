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
 * 文件操作函数类型定义
 */
typedef int (*readfile)(const char*);//param是filename
typedef int (*writefile)(const char*);//param是filename


/*
 * 文件数据段字节偏移
 */
#define DATASESSION_OFFSET 26 //前26字节是文件元数据，[triomaxbuf][version][enc][count]

class FileProcesser
{
public:
	FileProcesser();

	int readfile2buffer(Buf& buf);//读入数据，
	int writebuffer2file(Buf& buf);


	int saveData2File(int32_t* data, size_t size);//opt,目前只支持32bits
	int loadFile();//测试文件使用
	int directLoadDataSet();//测试文件使用

public:
	const char* filename;
	off_t getp, putp;//文件偏移
};
#endif // !FILE_PROCESSER_H
