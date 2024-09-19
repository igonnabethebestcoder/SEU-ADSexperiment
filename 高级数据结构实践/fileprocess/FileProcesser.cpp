#include"FileProcesser.h"

FileProcesser::FileProcesser()
{
    filename = "temp.dat";
}

int FileProcesser::readfile(const char* filename)
{
	return 0;
}

int FileProcesser::writefile(const char* filename)
{
	return 0;
}

int FileProcesser::saveData2File(int32_t* data, size_t size)
{
    std::ofstream outfile(filename, std::ios::binary);

    // 写入文件标识符
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // 写入版本号
    float version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // 写入数据类型 (如 ENC_INT32)
    int encoding = ENC_INT32;
    outfile.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

    // 写入数据量
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));

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
    float version;
    infile.read(reinterpret_cast<char*>(&version), sizeof(version));

    // 读取数据类型
    int encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    if (encoding != ENC_INT32) {
        std::cerr << "Unsupported encoding type!" << std::endl;
        return ERR;
    }

    // 读取数据大小
    size_t size;
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
    int32_t data[] = { 10, 20, 30, 40, 50 };
    fp.saveData2File(data, 5);

    fp.loadFile();
    return 0;
}
#endif // !FILE_MAIN_TEST
