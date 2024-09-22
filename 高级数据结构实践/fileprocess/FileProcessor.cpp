#include"FileProcessor.h"
#include <random>
#include <cassert>
#include<cstring>

FileProcessor::FileProcessor(const char* filename)
{
    this->filename = newString(filename);

    getp = 0;
    putp = 0; // DATASESSION_OFFSET;
    dataAmount = 0;

    file.open(this->filename, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        //文件不存在，则创建文件
        std::cerr << "file not exits! creat file : " << this->filename << std::endl;
        file.open(this->filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
        if (!file.is_open())
            exit(EXIT_FAILURE);  // 或者使用 return
    }
}


FileProcessor::~FileProcessor()
{
    cout << "fileprocessor 析构函数调用" << endl;
    free(this->filename);
    file.close();
}


/// <summary>
/// 读取文件的元数据，并设置buffer编码和分配内存
/// </summary>
/// <param name="buf"></param>
/// <returns></returns>
int FileProcessor::loadMetaDataAndMallocBuf(Buf& buf)
{
    if (!file.is_open())
    {
        cerr << "Failed to open file!" << std::endl;
        return ERR;
    }

    if (getp != 0)
    {
        cerr << "Meta data may have been load!" << endl;
        cerr << "current file get offset is :" << getp << endl;
        return ERR;
    }

    //不需要了，与readfile2buffer内容冲突
    //防止已经被分配内存的buffer重新调用
    //assert(buf.encoding == ENC_NOTKNOW);

    // 跳到文件的 getp 偏移位置开始读取
    file.seekg(getp);

    // 读取文件标识符，确认文件格式
    char identifier[11];
    file.read(identifier, 10);
    identifier[10] = '\0'; // 末尾添加 null 终止符
    if (string(identifier) != "TRIOMAXBUF") {
        cerr << "Invalid file format!" << std::endl;
        return ERR;
    }

    // 读取版本号
    int32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // 读取数据类型编码并设置到缓冲区
    int32_t encoding;
    file.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    buf.setEncodingAndMalloc(encoding); // 设置编码并分配内存

    // 读取数据个数
    file.read(reinterpret_cast<char*>(&dataAmount), sizeof(dataAmount));

    getp = file.tellg();

    return OK;
}



/// <summary>
/// 将数据读取进入缓冲区
/// 区分第一次read和后续read
/// 第一次read时会调用Buf中的setEncAndMallco()
/// </summary>
/// <param name="buf"></param>
/// <returns></returns>
int FileProcessor::readfile2buffer(Buf& buf)
{
    //确保文件是打开的
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // 首次读取时初始化缓冲区
    if (buf.encoding == ENC_NOTKNOW || getp == 0) {
        // 读取元数据并设置缓冲区
        if (loadMetaDataAndMallocBuf(buf) != OK) {
            return ERR;
        }
    }

    // 跳到文件的 getp 偏移位置开始读取
    file.seekg(getp);

    // 计算还需要读取的字节数，避免超过文件数据总量
    size_t totalBytes = dataAmount * buf.getEncodingSize(buf.encoding);
    size_t remainingData = totalBytes - (getp - DATASESSION_OFFSET);
    //buf.size * Buf::getEncodingSize(buf.encoding)是buf的字节容量
    size_t bytesToRead = std::min(remainingData, buf.size * Buf::getEncodingSize(buf.encoding));

    if (bytesToRead == 0) {
        //std::cerr << "No more data to read from file!" << std::endl;
        buf.actualSize = 0;
        return DONE;
    }

    // 确保缓冲区足够大来容纳即将读取的数据
    if (buf.buffer == nullptr) {
        std::cerr << "Buffer is not allocated!" << std::endl;
        return ERR;
    }

    // 读取数据到缓冲区
    file.read(reinterpret_cast<char*>(buf.buffer), bytesToRead);
    size_t bytesRead = file.gcount();

    // 检查是否成功读取完整数据
    if (bytesRead != bytesToRead) {
        if (file.eof()) {
            std::cerr << "Reached end of file while reading." << std::endl;
            return EOF;
        }
        else {
            std::cerr << "Failed to read the expected amount of data!" << std::endl;
            return ERR;
        }
    }

    buf.actualSize = bytesRead / Buf::getEncodingSize(buf.encoding);
    // 更新缓冲区和文件的偏移量
    getp += bytesToRead;
    buf.pos = 0;//每次调用read函数一定会将之前buffer中的数据覆盖
    if (remainingData < buf.size * Buf::getEncodingSize(buf.encoding))
        return DONE;
    return CONTINUE;
}

int FileProcessor::writebuffer2file(Buf& buf)
{
    if (putp == DATASESSION_OFFSET + dataAmount * Buf::getEncodingSize(buf.encoding))
    {
        cout << "NOTICING : file has been merge pass once!" << endl;
        return OK;
    }

    // 确保文件是打开的
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // 首次写入时进行缓冲区和元数据的初始化
    if (buf.encoding == ENC_NOTKNOW) {
        cout << "Unknow encoding of buf !" << endl;
        return ERR;
    }

    // 跳到文件的 getp 偏移位置开始写入
    file.seekp(putp);

    if (putp == 0)
    {
        cout << "Need to write meta data to file !" << endl;
        // 写入文件标识符,10字节
        const char* identifier = "TRIOMAXBUF";
        file.write(identifier, 10);

        // 写入版本号，4字节
        int32_t version = 1.0;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // 写入数据类型 (如 ENC_INT32) , 4字节
        int32_t encoding = ENC_INT32;
        file.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

        // 写入数据量, 8字节
        file.write(reinterpret_cast<const char*>(&dataAmount), sizeof(dataAmount));
    }

    //更新写文件偏移
    putp = file.tellp();

    if (putp < DATASESSION_OFFSET)
    {
        cerr << "putp can not be DATASESSION_OFFSET" << endl;
        exit(1);
    }

    // 计算还需要写入的字节数，避免超过文件最大容量
    size_t bytesToWrite =buf.actualSize * buf.getEncodingSize(buf.encoding); // 计算当前写入字节数

    if (bytesToWrite == 0) {
        std::cerr << "No more data to write to file!" << std::endl;
        return ERR;
    }

    // 确保缓冲区中有数据要写入
    if (buf.buffer == nullptr) {
        std::cerr << "Buffer is not allocated!" << std::endl;
        return ERR;
    }

    // 根据编码类型写入不同的数据到文件
    file.write(reinterpret_cast<const char*>(buf.buffer), bytesToWrite);
    if (!file) {
        std::cerr << "Failed to write complete data to file!" << std::endl;
        return ERR;
    }

    // 更新文件偏移量
    putp += bytesToWrite;
    buf.actualSize = 0;//重置缓冲区大小
    buf.pos = 0;
    file.flush();

    return OK;
}

/// <summary>
/// 生成指定格式的外部文件
/// </summary>
/// <returns></returns>
int FileProcessor::saveData2File(int32_t* data, size_t size)
{
    std::ofstream outfile(filename, std::ios::binary);

    // 写入文件标识符,10字节
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // 写入版本号，4字节
    int32_t version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // 写入数据类型 (如 ENC_INT32) , 4字节
    int32_t encoding = ENC_INT32;
    outfile.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

    // 写入数据量, 8字节
    uint64_t bufcount = size;//无符号整数
    outfile.write(reinterpret_cast<const char*>(&bufcount), sizeof(bufcount));

    // 写入实际数据
    outfile.write(reinterpret_cast<const char*>(data), size * sizeof(int32_t));

    outfile.close();

    return OK;
}

/// <summary>
/// 测试使用，将整个文件载入
/// </summary>
/// <returns></returns>
int FileProcessor::loadFile()
{
    std::ifstream infile(filename, std::ios::binary);

    // 读取文件标识符
    char identifier[11];
    infile.read(identifier, 10);
    identifier[10] = '\0'; // 末尾添加 null 终止符
    if (std::string(identifier) != "TRIOMAXBUF") {
        std::cerr << "Invalid file format!" << std::endl;
        return ERR;
    }

    // 读取版本号
    int32_t version;
    infile.read(reinterpret_cast<char*>(&version), sizeof(version));

    // 读取数据类型
    int32_t encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    if (encoding != ENC_INT32) {
        std::cerr << "Unsupported encoding type!" << std::endl;
        return ERR;
    }

    // 读取数据大小
    uint64_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));

    // 读取实际数据
    int32_t* data = new int32_t[size];
    infile.read(reinterpret_cast<char*>(data), size * sizeof(int32_t));

    // 输出数据
    for (size_t i = 0; i < size; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;

    delete[] data;
    infile.close();

    return OK;
}


int FileProcessor::directLoadDataSet() {
    std::ifstream infile(filename, ios::in | std::ios::binary);
    infile.seekg(18);
    // 读取数据大小
    uint64_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));

    // 读取实际数据
    int32_t* data = new int32_t[size];
    infile.read(reinterpret_cast<char*>(data), size * sizeof(int32_t));

    // 输出数据
    for (size_t i = 0; i < size; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;

    delete[] data;
    infile.close();

    return OK;
}

#define FILE_MAIN_TEST
#ifndef FILE_MAIN_TEST
int main()
{
    FileProcessor fp;
    const int size = 100;  // 数组大小
    int32_t data[size];

    // 随机数生成器
    std::random_device rd;  // 获取随机数种子
    std::mt19937 gen(rd());  // 随机数引擎
    std::uniform_int_distribution<int32_t> dis(1, 100);  // 生成 1 到 100 之间的随机整数

    // 填充数组
    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    fp.saveData2File(data, size);

    fp.directLoadDataSet();
    return 0;
}
#endif // !FILE_MAIN_TEST
