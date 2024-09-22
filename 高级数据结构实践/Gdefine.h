#pragma once
#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H

/* 
 * �����Ƿ������궨��
 */
#define OK 0
#define ERR -1
#define MERGE 1

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
#endif // !GLOBAL_DEFINE_H

//ʹ�ú���Ҫע���ͷ�
inline char* newString(const char* str)
{
    size_t len = strlen(str);

    // ʹ�� malloc ��̬�����ڴ�
    char* newStr = nullptr;
    newStr = (char*)malloc(len + 1);

    if (newStr != nullptr) {
        // �� input ���ݸ��Ƶ� this->filename
        strncpy_s(newStr, len + 1, str, len);  // �޸��ĵط�: Ŀ�껺������СΪ len + 1
        newStr[len] = '\0';  // ȷ���Կ��ַ���β
    }

    return newStr;
}
