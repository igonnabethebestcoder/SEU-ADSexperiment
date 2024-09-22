#pragma once
#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

/* 
 * 程序是否正常宏定义
 */
#define OK 0
#define ERR -1
#define MERGE 1

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
#endif // !GLOBAL_DEFINE_H

//使用后需要注意释放
inline char* newString(const char* str)
{
    size_t len = strlen(str);

    // 使用 malloc 动态分配内存
    char* newStr = nullptr;
    newStr = (char*)malloc(len + 1);

    if (newStr != nullptr) {
        // 将 input 内容复制到 this->filename
        strncpy_s(newStr, len + 1, str, len);  // 修复的地方: 目标缓冲区大小为 len + 1
        newStr[len] = '\0';  // 确保以空字符结尾
    }

    return newStr;
}
