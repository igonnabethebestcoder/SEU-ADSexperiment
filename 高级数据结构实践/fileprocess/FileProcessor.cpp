#include"FileProcessor.h"
#include <random>
#include <cassert>
#include<cstring>
extern char* newString(const char* str);
FileProcessor::FileProcessor(const char* filename)
{
    this->filename = newString(filename);

    getp = 0;
    putp = 0; // DATASESSION_OFFSET;
    dataAmount = 0;

    file.open(this->filename, ios::binary | ios::in | ios::out);
    if (!file.is_open()) {
        //�ļ������ڣ��򴴽��ļ�
        cerr << "file not exits! creat file : " << this->filename << endl;
        file.open(this->filename, ios::binary | ios::in | ios::out | ios::trunc);
        if (!file.is_open())
            exit(EXIT_FAILURE);  // ����ʹ�� return
    }
}


FileProcessor::~FileProcessor()
{
    //cout << "fileprocessor ������������" << endl;
    free(this->filename);
    file.close();
}


/// <summary>
/// ��ȡ�ļ���Ԫ���ݣ�������buffer����ͷ����ڴ�
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

    //����Ҫ�ˣ���readfile2buffer���ݳ�ͻ
    //��ֹ�Ѿ��������ڴ��buffer���µ���
    //assert(buf.encoding == ENC_NOTKNOW);

    // �����ļ��� getp ƫ��λ�ÿ�ʼ��ȡ
    file.seekg(getp);

    // ��ȡ�ļ���ʶ����ȷ���ļ���ʽ
    char identifier[11];
    file.read(identifier, 10);
    identifier[10] = '\0'; // ĩβ��� null ��ֹ��
    if (string(identifier) != "TRIOMAXBUF") {
        cerr << "Invalid file format!" << std::endl;
        return META_ERR;
    }

    // ��ȡ�汾��
    int32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // ��ȡ�������ͱ��벢���õ�������
    int32_t encoding;
    file.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));
    buf.setEncodingAndMalloc(encoding); // ���ñ��벢�����ڴ�

    // ��ȡ���ݸ���
    file.read(reinterpret_cast<char*>(&dataAmount), sizeof(dataAmount));

    getp = file.tellg();

    assert(getp == DATASESSION_OFFSET);

    return OK;
}

//����ļ���Ԫ���ݶ��Ƿ���ȷ
int FileProcessor::checkMetaData()
{
    if (!file.is_open())
    {
        cerr << "Failed to open file!" << std::endl;
        return ERR;
    }

    // �����ļ��� getp ƫ��λ�ÿ�ʼ��ȡ
    file.seekg(0);

    // ��ȡ�ļ���ʶ����ȷ���ļ���ʽ
    char identifier[11];
    file.read(identifier, 10);
    identifier[10] = '\0'; // ĩβ��� null ��ֹ��
    if (string(identifier) != "TRIOMAXBUF") {
        cerr << "Invalid file format!" << std::endl;
        return META_ERR;
    }

    // ��ȡ�汾��
    int32_t version;
    file.read(reinterpret_cast<char*>(&version), sizeof(version));

    // ��ȡ�������ͱ��벢���õ�������
    int32_t encoding;
    file.read(reinterpret_cast<char*>(&encoding), sizeof(encoding));

    // ��ȡ���ݸ���
    file.read(reinterpret_cast<char*>(&dataAmount), sizeof(dataAmount));

    assert(file.tellg() == DATASESSION_OFFSET);

    return OK;
}

/// <summary>
/// �����ݶ�ȡ���뻺����
/// ���ֵ�һ��read�ͺ���read
/// ��һ��readʱ�����Buf�е�setEncAndMallco()
/// </summary>
/// <param name="buf"></param>
/// <returns>
/// DONE���ܱ�ʾһ��û�ж���Ҳ���ܱ�ʾʣ�µĶ�������
/// </returns>
int FileProcessor::readfile2buffer(Buf& buf)
{
    //ȷ���ļ��Ǵ򿪵�
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // �״ζ�ȡʱ��ʼ��������
    if (buf.encoding == ENC_NOTKNOW || getp == 0) {
        // ��ȡԪ���ݲ����û�����
        if (loadMetaDataAndMallocBuf(buf) != OK) {
            return ERR;
        }
    }

    // �����ļ��� getp ƫ��λ�ÿ�ʼ��ȡ
    file.seekg(getp);

    // ���㻹��Ҫ��ȡ���ֽ��������ⳬ���ļ���������
    size_t totalBytes = dataAmount * buf.getEncodingSize(buf.encoding);
    size_t remainingData = totalBytes - (getp - DATASESSION_OFFSET);
    //buf.size * Buf::getEncodingSize(buf.encoding)��buf���ֽ�����
    size_t bytesToRead = std::min(remainingData, buf.size * Buf::getEncodingSize(buf.encoding));

    if (bytesToRead == 0) {
        //std::cerr << "No more data to read from file!" << std::endl;
        buf.actualSize = 0;
        return DONE;
    }

    // ȷ���������㹻�������ɼ�����ȡ������
    if (buf.buffer == nullptr) {
        std::cerr << "Buffer is not allocated!" << std::endl;
        return ERR;
    }

    // ��ȡ���ݵ�������
    file.read(reinterpret_cast<char*>(buf.buffer), bytesToRead);
    size_t bytesRead = file.gcount();

    // ����Ƿ�ɹ���ȡ��������
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
    // ���»��������ļ���ƫ����
    getp += bytesToRead;
    buf.pos = 0;//ÿ�ε���read����һ���Ὣ֮ǰbuffer�е����ݸ���

    {
        lock_guard<mutex> lock(ioReadMtx);
        ioReadCount++;
    }

    if (remainingData <= buf.size * Buf::getEncodingSize(buf.encoding))
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

    // ȷ���ļ��Ǵ򿪵�
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return ERR;
    }

    // �״�д��ʱ���л�������Ԫ���ݵĳ�ʼ��
    if (buf.encoding == ENC_NOTKNOW) {
        cout << "Unknow encoding of buf !" << endl;
        return ERR;
    }

    // �����ļ��� getp ƫ��λ�ÿ�ʼд��
    file.seekp(putp);

    if (putp == 0)
    {
        cout << "Need to write meta data to file !" << endl;
        // д���ļ���ʶ��,10�ֽ�
        const char* identifier = "TRIOMAXBUF";
        file.write(identifier, 10);

        // д��汾�ţ�4�ֽ�
        int32_t version = 1.0;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));

        // д���������� (�� ENC_INT32) , 4�ֽ�
        int32_t encoding = ENC_INT32;
        file.write(reinterpret_cast<const char*>(&encoding), sizeof(encoding));

        // д��������, 8�ֽ�
        file.write(reinterpret_cast<const char*>(&dataAmount), sizeof(dataAmount));
    }

    //����д�ļ�ƫ��
    putp = file.tellp();

    assert(putp >= DATASESSION_OFFSET);
    /*if (putp < DATASESSION_OFFSET)
    {
        cerr << "putp can not be DATASESSION_OFFSET" << endl;

        exit(1);
    }*/

    // ���㻹��Ҫд����ֽ��������ⳬ���ļ��������
    size_t bytesToWrite =buf.actualSize * buf.getEncodingSize(buf.encoding); // ���㵱ǰд���ֽ���

    if (bytesToWrite == 0) {
        std::cerr << "No more data to write to file!" << std::endl;
        return ERR;
    }

    // ȷ����������������Ҫд��
    if (buf.buffer == nullptr) {
        std::cerr << "Buffer is not allocated!" << std::endl;
        return ERR;
    }

    // ���ݱ�������д�벻ͬ�����ݵ��ļ�
    file.write(reinterpret_cast<const char*>(buf.buffer), bytesToWrite);
    if (!file) {
        std::cerr << "Failed to write complete data to file!" << std::endl;
        return ERR;
    }

    // �����ļ�ƫ����
    putp += bytesToWrite;
    buf.actualSize = 0;//���û�������С
    buf.pos = 0;
    file.flush();//ˢ�½������

    {
        lock_guard<mutex> lock(ioWriteMtx);
        ioWriteCount++;
    }

    return OK;
}

//����runfile������������
int FileProcessor::updateMetaDataAmount(uint64_t da)
{
    //�����ļ�ƫ��
    file.seekp(DATASESSION_OFFSET - 8);

    if (da == 0)
        cout << "WARNING: runfile data amount will be set to 0!" << endl;
    dataAmount = da;

    // д��������, 8�ֽ�
    file.write(reinterpret_cast<const char*>(&dataAmount), sizeof(dataAmount));
    if (!file) {
        std::cerr << "Failed to update data amount to file!" << std::endl;
        return ERR;
    }

    file.flush();//ˢ�½������

    return OK;
}

/// <summary>
/// ����ָ����ʽ���ⲿ�ļ�
/// </summary>
/// <returns></returns>
int FileProcessor::saveData2File(int32_t* data, size_t size)
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

/// <summary>
/// ����ʹ�ã��������ļ�����
/// </summary>
/// <returns></returns>
int FileProcessor::loadFile()
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


int FileProcessor::directLoadDataSet() {
    std::ifstream infile(filename, ios::in | std::ios::binary);
    infile.seekg(18);
    // ��ȡ���ݴ�С
    uint64_t size;
    infile.read(reinterpret_cast<char*>(&size), sizeof(size));
    this->dataAmount = size;
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

#define FILE_MAIN_TEST
#ifndef FILE_MAIN_TEST
int main()
{
    FileProcessor fp("temp100000.dat");
    const int size = 100000;  // �����С
    int32_t data[size];

    // �����������
    std::random_device rd;  // ��ȡ���������
    std::mt19937 gen(rd());  // ���������
    std::uniform_int_distribution<int32_t> dis(1, 1000000);  // ���� 1 �� 100 ֮����������

    // �������
    for (int i = 0; i < size; ++i) {
        data[i] = dis(gen);
    }
    fp.saveData2File(data, size);

    fp.directLoadDataSet();
    return 0;
}
#endif // !FILE_MAIN_TEST
