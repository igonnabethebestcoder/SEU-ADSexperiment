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

    // �����ļ��� getp ƫ��λ�ÿ�ʼ��ȡ
    infile.seekg(getp);

    // ��ȡ�ļ���ʶ����ȷ���ļ���ʽ
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

    // ��ȡ�������ͱ��벢���õ�������
    int encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    buf.setEncodingAndMalloc(encoding); // ���ñ��벢�����ڴ�

    // ��ȡ���ݴ�С
    size_t dataSize;
    infile.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

    // ȷ����������С�㹻
    if (dataSize > buf.size) {
        std::cerr << "Buffer size is too small!" << std::endl;
        return ERR;
    }

    // ��ȡʵ�����ݵ�������
    infile.read(reinterpret_cast<char*>(buf.buffer), dataSize * sizeof(int32_t)); // �������ʹ��int32_t����

    // �����ļ�ƫ����
    getp = infile.tellg();

    infile.close();
    return OK;
}

int FileProcesser::writebuffer2file(Buf& buf)
{
    std::ofstream outfile(filename, std::ios::binary | std::ios::app); // ��׷��ģʽ���ļ�

    if (!outfile.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // �����ļ��� putp ƫ��λ�ÿ�ʼд��
    outfile.seekp(putp);

    // д���ļ���ʶ��
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // д��汾��
    float version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // д���������ͱ���
    outfile.write(reinterpret_cast<const char*>(&buf.encoding), sizeof(buf.encoding));

    // д�����ݴ�С
    size_t dataSize = buf.size;
    outfile.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    // д�뻺�����е�����
    outfile.write(reinterpret_cast<const char*>(buf.buffer), dataSize * sizeof(int32_t)); // ������軺������int32_t��������

    // �����ļ�ƫ����
    putp = outfile.tellp();

    outfile.close();
    return OK;
}

int FileProcesser::saveData2File(int32_t* data, size_t size)
{
    std::ofstream outfile(filename, std::ios::binary);

    // д���ļ���ʶ��,10�ֽ�
    const char* identifier = "TRIOMAXBUF";
    outfile.write(identifier, 10);

    // д��汾�ţ�4�ֽ�
    int32_t version = 1.0;
    outfile.write(reinterpret_cast<const char*>(&version), sizeof(version));

    // д���������� (�� ENC_INT32) , 4�ֽ�
    int32_t encoding = ENC_INT32;
    outfile.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

    // д��������, 8�ֽ�
    uint64_t bufcount = size;//�޷�������
    outfile.write(reinterpret_cast<const char*>(&bufcount), sizeof(bufcount));

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
    int32_t version;
    infile.read(reinterpret_cast<char*>(&version), sizeof(version));

    // ��ȡ��������
    int32_t encoding;
    infile.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    if (encoding != ENC_INT32) {
        std::cerr << "Unsupported encoding type!" << std::endl;
        return ERR;
    }

    // ��ȡ���ݴ�С
    uint64_t size;
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

int FileProcesser::directLoadDataSet() {
    std::ifstream infile(filename, std::ios::binary);
    infile.seekg(18);
    // ��ȡ���ݴ�С
    uint64_t size;
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
    int32_t data[] = { 10, 20, 30, 40, 50, 60, 80 };
    fp.saveData2File(data, 7);

    fp.directLoadDataSet();
    return 0;
}
#endif // !FILE_MAIN_TEST
