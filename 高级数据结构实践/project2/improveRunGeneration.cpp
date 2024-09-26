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
void threadReadFile(int& loadingBuf, project& p)
{
    //��һ�����������꣬����activeBuf
    int res = CONTINUE;
    do {
        //�������ļ�
        {
            lock_guard<mutex> lock(loadingBuf == 0 ? buf1 : buf2);
            if (loadingBuf == 0 && p.input1->actualSize <= 0) {
                res = p.fp->readfile2buffer(*(p.input1));
            }
            else if (loadingBuf == 1 && p.input2->actualSize <= 0) {
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
            loadingBuf ^= 1;//��0��1���л�
        }
    } while (res != DONE);

    cout << "src file done reading!\nquiting child process!" << endl;

    //Դ�ļ��Ѿ����꣬�ս����߳�
    return;
}

//����븳ֵ
void custom_E(int& activeBuf)
{

}

//������ͬ�鲢��ʱ���ö��߳�
void createDiffLenRuns(project& p, int k)
{
    //Ԥ����k
    if (k > p.fp->dataAmount)
        k = p.fp->dataAmount;//opt, ���Ǻܺ���

    //��������ʱ�ı�workingState
    int activeBuf = 0, workingState = 0, loadingBuf = 0;
    long curRunfile = 0;//��ǰ������runfile��
    int32_t curRunflleMin = INT32_MAX;
    Buf* curOpBuf = nullptr;
    int leaveActSize = 0;
    int fillPos = 0;//leaves����ָ���������
    vector<int32_t> leaves(k, 0);
    //opt,��ָ���������ͣ�������Ҫ�޸�֧�����ݶ�����
    LoserTree<int32_t>* lt = nullptr;

    //�����߳�
    thread reader(threadReadFile, ref(loadingBuf), ref(p));
    thread writer(threadWriteFile, ref(p), ref(workingState), ref(curRunfile));
    
    //��������������������������
    //���ʵ���λ�õ��� obufCv.notify_one() �� obufCv.notify_all()
    //����һ����СΪk��vector��buf�л�ȡ���ݣ������������⣩
    //ʹ��vector����k·�鲢��
    //��activeBuf��obuf������obuf������inputbuffer��ʱ����
    //ʹ��obufCv.notifyone()����д�߳�
    //goto 132;
    //until ����inputbuffer��Ϊ��
    while (1)//��Ҫȷ����ֹ����
    {

        {
            //ͨ��activeBufȷ����ǰ��Ҫ������buffer
            lock_guard<mutex> ibuflock((activeBuf == 0) ? buf1 : buf2);
            lock_guard<mutex> obuflock(obuf);
            curOpBuf = (activeBuf == 0) ? p.input1 : p.input2;
            //opt
            int32_t* nums = reinterpret_cast<int32_t*>(curOpBuf->buffer);
            if (curOpBuf->actualSize <= 0)//��ǰ������buf��û������
            {
                //����buf
                activeBuf ^= 1;
                continue;//������һ�ֽ��н������б�Ҫ��
            }
            //buf��������ʼ
            else
            {
                //��ʼ������Ƿ����ֱ�ӿ�ʼ�Ƚϣ���ʼ��ʱʹ��
                //��Ϊinputbuffer-size���ܴ���k������ڳ�ʼ����ʱ��
                bool canStart = false;//�����һ��inputbuffer����k
                bool justInit = false;
                //����Ƿ��ʼ��������
                //û���򴴽�
                if (lt == nullptr)
                {
                    justInit = true;

                    //����bufferSize > kʱ��ʼ�������ֱ������
                    if (k < curOpBuf->actualSize)
                        canStart = true;

                    //�������vector-leaves
                    while (curOpBuf->pos < curOpBuf->actualSize && curOpBuf->pos < k)
                    {
                        leaves[fillPos++] = nums[curOpBuf->pos++];//Ӧ��ʹ�ö���븳ֵ
                        curOpBuf->actualSize--;
                    }

                    //��ʼ����������
                    if (fillPos >= k)
                        lt = new LoserTree<int32_t>(k, leaves);
                }

                int need2ChangeInputBuf = 0;
                //�Ѿ���ʼ����ɣ���ʼ
                if ((!justInit || canStart) && lt)
                {
                    int32_t* outputBuf = reinterpret_cast<int32_t*>(p.output->buffer);
                    while (curOpBuf->actualSize > 0)
                    {
                        //�����������
                        if (p.output->actualSize >= p.output->size)
                        {
                            need2ChangeInputBuf = 1;
                            break;
                        }

                        //Ӧ�ò����µĹ鲢����
                        if (lt->banCount == k)
                        {
                            {
                                lock_guard<mutex> lock(curRunfileMtx);
                                curRunfile++;
                                curRunflleMin = INT32_MAX;
                                lt->reCompete();
                                break;
                            }
                        }

                        //����Сֵ�������������
                        outputBuf[p.output->pos++] = lt->getWinner();
                        if (lt->getWinner() < curRunflleMin)
                            curRunflleMin = lt->getWinner();

                        //���°�����
                        //�ж��Ƿ���Ҫ����
                        if (nums[curOpBuf->pos] < curRunflleMin)
                            lt->disqualify(lt->tree[0]);
                        lt->replaceWinner(nums[curOpBuf->pos++]);
                        curOpBuf->actualSize--;
                    }
                }

                if (need2ChangeInputBuf)
                    activeBuf ^= 1;
            }
        }
        //�������������д�߳�
        obufCv.notify_one();
    }

    reader.join();
    writer.join();
    return;
}


//huffman�ϲ�ʱ���õ��߳�
void huffmanMerge() {
    //ʹ�����ȶ��У��Լ�hisRun����
}


//#define HUFFMAN_MERGE
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
