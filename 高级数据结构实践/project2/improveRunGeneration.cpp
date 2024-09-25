#include"improveRunGeneration.h"

struct project p2;

mutex activeBufMtx, workingStateMtx, curRunfileMtx;//activeBuf����,�Լ�workingState�ľ�����

mutex buf1, buf2, obuf;//������������buf lock, �����������

condition_variable obufCv;//output buffer ���������������ڹ���ȴ�outputbuffer����д������

//�����ɲ�ͬ��runfile
void threadWriteFile(project& p, int workingState, long& curRunfile)
{
    //�����������
    //1.����������
    //2.������û�����ǣ��ļ������Լ�������
    //������ʵ����Ҫ���ǵ�ǰӦ��д���ļ���������ʵ�ʴ�С
    //ֻ��Ҫ�ȴ��������ͷ����Ϳ���
    long tempRunfile = curRunfile;
    string runfileName = "run_" + to_string(tempRunfile) + ".dat";
    //Ϊ��ǰ�ļ������ļ�������
    FileProcessor* curFp = new FileProcessor(runfileName.c_str());

    //��д�ļ�������������дmeta data, 8�ֽ�
    uint64_t curRunfileDataAmount = 0;

    while (1)
    {
        //ѭ����ֹ����
        {
            lock_guard<mutex> lock(workingStateMtx);
            if (workingState)
            {
                //��runfile������д����file��
                curFp->updateMetaDataAmount(curRunfileDataAmount);
                delete curFp;
                return;
            }
        }

        //����Ƿ�����µĹ鲢��
        {
            lock_guard<mutex> lock(curRunfileMtx);
            //���߳���curRunfile����˵���Ѿ������µĹ鲢��      
            if (curRunfile != tempRunfile)
            {
                tempRunfile = curRunfile;

                //��runfile������д����file��
                curFp->updateMetaDataAmount(curRunfileDataAmount);

                delete curFp;

                //�����µ�runfile
                runfileName = "run_" + to_string(tempRunfile) + ".dat";
                curFp = new FileProcessor(runfileName.c_str());
            }
        }

        //�ȴ��������ͷ�����˵����д��
        //�ȴ�����������д��
        unique_lock<mutex> lock(obuf);
        obufCv.wait(lock);//�����������

        //����д�����
        //д֮ǰ�ȸ���������
        curRunfileDataAmount += p.output->actualSize;
        curFp->writebuffer2file(*(p.output));
    }

    if (curFp)
        delete curFp;
    return;
}

//��Դ�ļ��ж����ݵ�������
void threadReadFile(int& activeBuf, project& p)
{
    //��һ�����������꣬����activeBuf
    int res = CONTINUE;
    do {
        //�������ļ�
        {
            lock_guard<mutex> lock(activeBuf == 0 ? buf1 : buf2);
            if (activeBuf == 0 && p.input1->actualSize <= 0) {
                res = p.fp->readfile2buffer(*(p.input1));
            }
            else if (activeBuf == 1 && p.input2->actualSize <= 0) {
                res = p.fp->readfile2buffer(*(p.input2));
            }
            else {
                cout << "something wrong!" << endl;
            }
        }

        //���ļ�ʧ��
        if (res != DONE && res != CONTINUE)
        {
            cerr << "Unknow read file state !" << endl;
            exit(1);
        }

        //�������ʾ���ļ��ɹ�
        {
            lock_guard<mutex> lock(activeBufMtx);
            activeBuf ^= 1;//��0��1���л�
        }
    } while (res != DONE);

    cout << "src file done reading!\nquiting child process!" << endl;

    //Դ�ļ��Ѿ����꣬�ս����߳�
    return;
}


//������ͬ�鲢��ʱ���ö��߳�
void createDiffLenRuns(project& p, int k)
{
    //��������ʱ�ı�workingState
    int activeBuf = 0, workingState = 0;
    long curRunfile = 0;

    //�����߳�
    thread reader(threadReadFile, ref(activeBuf), ref(p));
    thread writer(threadWriteFile, ref(p), ref(workingState), ref(curRunfile));
    
    

    //��������������������������
    //���ʵ���λ�õ��� obufCv.notify_one() �� obufCv.notify_all()

    reader.join();
    writer.join();
    return;
}


//huffman�ϲ�ʱ���õ��߳�
void huffmanMerge() {

}


#define HUFFMAN_MERGE
#ifndef HUFFMAN_MERGE
int main()
{
    //p�����������뻺������һ�����������
    initP(p, 10, 20, HUFFMAN);
    cout << "--------ԭʼ����---------" << endl;
    p.fp->directLoadDataSet();
    cout << "--------ԭʼ����---------" << endl << endl;
    cout << "���ɵ�runfile���� :" << p.runAmount << endl;
	return 0;
}
#endif // !HUFFMAN_MERGE
