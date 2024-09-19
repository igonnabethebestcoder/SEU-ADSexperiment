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

    // д���ļ���ʶ��
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // д��汾��
    float version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // д���������� (�� ENC_INT32)
    int encoding = ENC_INT32;
    outfile.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

    // д��������
    outfile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    // д��ʵ������
    outfile.write(reinterpret_cast<const char*>(data), size * sizeof(int32_t));

    outfile.close();

    return OK;
}

int FileProcesser::loadFile()
{
    std::ifstream infile(filename, std::ios::binary);

    // ��ȡ�ļ���ʶ��
    char identifier[11];
    infile.read(identifier, 10);
    identifier[10] = '\0'; // ĩβ��� null ��ֹ��
    if (std::string(identifier) != "TRIOMAXBUF") {
        std::cerr << "Invalid file format!" << std::endl;
        return ERR;
    }

    // ��ȡ�汾��
    float version;
    infile.read(reinterpret_cast<char*>(&version), sizeof(version));

    // ��ȡ��������
    int encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    if (encoding != ENC_INT32) {
        std::cerr << "Unsupported encoding type!" << std::endl;
        return ERR;
    }

    // ��ȡ���ݴ�С
    size_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));

    // ��ȡʵ������
    int32_t* data = new int32_t[size];
    infile.read(reinterpret_cast<char*>(data), size * sizeof(int32_t));

    // �������
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
