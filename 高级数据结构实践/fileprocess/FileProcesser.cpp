#include"FileProcesser.h"

FileProcesser::FileProcesser()
{
    filename = "temp.dat";
}

int FileProcesser::readfile2buffer(Buf& buf)
{
    std::ifstream infile(filename, std::ios::binary);

    if (!infile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // 跳到文件的 getp 偏移位置开始读取
    infile.seekg(getp);

    // 读取文件标识符，确认文件格式
    char identifier[11];
    infile.read(identifier, 10);
    identifier[10] = '\0'; // 末尾添加 null 终止符
    if (std::string(identifier) != "TRIOMAXBUF") {
        std::cerr << "Invalid file format!" << std::endl;
        return ERR;
    }

    // 读取版本号
    float version;
    infile.read(reinterpret_cast<char*>(&version), sizeof(version));

    // 读取数据类型编码并设置到缓冲区
    int encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    buf.setEncodingAndMalloc(encoding); // 设置编码并分配内存

    // 读取数据大小
    size_t dataSize;
    infile.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

    // 确保缓冲区大小足够
    if (dataSize > buf.size) {
        std::cerr << "Buffer size is too small!" << std::endl;
        return ERR;
    }

    // 读取实际数据到缓冲区
    infile.read(reinterpret_cast<char*>(buf.buffer), dataSize * sizeof(int32_t)); // 这里假设使用int32_t数据

    // 更新文件偏移量
    getp = infile.tellg();

    infile.close();
    return OK;
}

int FileProcesser::writebuffer2file(Buf& buf)
{
    std::ofstream outfile(filename, std::ios::binary | std::ios::app); // 以追加模式打开文件

    if (!outfile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // 跳到文件的 putp 偏移位置开始写入
    outfile.seekp(putp);

    // 写入文件标识符
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // 写入版本号
    float version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // 写入数据类型编码
    outfile.write(reinterpret_cast<const char*>(&buf.encoding), sizeof(buf.encoding));

    // 写入数据大小
    size_t dataSize = buf.size;
    outfile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    // 写入缓冲区中的数据
    outfile.write(reinterpret_cast<const char*>(buf.buffer), dataSize * sizeof(int32_t)); // 这里假设缓冲区是int32_t类型数据

    // 更新文件偏移量
    putp = outfile.tellp();

    outfile.close();
    return OK;
}

int FileProcesser::saveData2File(int32_t* data, size_t size)
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

int FileProcesser::loadFile()
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

int FileProcesser::directLoadDataSet() {
    std::ifstream infile(filename, std::ios::binary);
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

//#define FILE_MAIN_TEST
#ifndef FILE_MAIN_TEST
int main()
{
    FileProcesser fp = FileProcesser();
    int32_t data[] = { 10, 20, 30, 40, 50, 60, 80 };
    fp.saveData2File(data, 7);

    fp.directLoadDataSet();
    return 0;
}
#endif // !FILE_MAIN_TEST
