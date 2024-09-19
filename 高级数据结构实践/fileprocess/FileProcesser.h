#pragma once
#ifndef FILE_PROCESSER_H
#define FILE_PROCESSER_H
#include <cstdint>
#include <fstream>
#include "../Gdefine.h"
#include<iostream>
using namespace std;


class FileProcesser
{
public:
	FileProcesser();

	static int readfile(const char* filename);
	static int writefile(const char* filename);


	int saveData2File(int32_t* data, size_t size);
	int loadFile();//测试文件使用


public:
	const char* filename;
};
#endif // !FILE_PROCESSER_H
