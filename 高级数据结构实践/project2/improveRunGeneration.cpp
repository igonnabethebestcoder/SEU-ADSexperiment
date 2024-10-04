#include"improveRunGeneration.h"
#include "../global/Gdefine.h"
struct project p2;

uint64_t totalWriteAmount = 0;
long maxRunfileNum = 0;

mutex activeBufMtx, workingStateMtx, curRunfileMtx;//activeBuf����,�Լ�workingState�ľ�����

mutex buf1, buf2, obuf;//������������buf lock, �����������

mutex totalWriteAmountMtx;

condition_variable obufCv;//output buffer ���������������ڹ���ȴ�outputbuffer����д������

//�����ɲ�ͬ��runfile
void threadWriteFile(project& p, int& workingState, long& curRunfile)
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
                curRunfileDataAmount = 0;
                delete curFp;

                //�����µ�runfile
                runfileName = "run_" + to_string(tempRunfile) + ".dat";
                curFp = new FileProcessor(runfileName.c_str());
            }
        }

        //�ȴ��������ͷ�����˵����д��
        //�ȴ�����������д��
        unique_lock<mutex> lock(obuf);
        cout << "WRITE THREAD: Waiting for buffer to be available..." << endl;
        obufCv.wait(lock);
        cout << "WRITE THREAD: Buffer available, proceeding to write." << endl;
        cout << "WRITE THREAD: writing buffer into runfile : " << tempRunfile << endl;
        //logger.log(Log::INFO, );
        //����д�����
        //д֮ǰ�ȸ���������
        int tempWriteAmount = p.output->actualSize;
        curRunfileDataAmount += p.output->actualSize;
        curFp->writebuffer2file(*(p.output));
        {
            lock_guard<mutex> lock(totalWriteAmountMtx);
            totalWriteAmount += tempWriteAmount;
            cout << "WRITE THREAD: total data amount write :" << totalWriteAmount << endl;
        }
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
                //readamount += p.input1->actualSize;
                cout << "READ THREAD: reading buffer1" << endl;
            }
            else if (loadingBuf == 1 && p.input2->actualSize <= 0) {
                res = p.fp->readfile2buffer(*(p.input2));
                //readamount += p.input2->actualSize;
                cout << "READ THREAD: reading buffer2" << endl;
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
    //this_thread::sleep_for(std::chrono::seconds(1));
    while (1)//��Ҫȷ����ֹ����
    {
        {
            //ͨ��activeBufȷ����ǰ��Ҫ������buffer
            lock_guard<mutex> ibuflock((activeBuf == 0) ? buf1 : buf2);
            lock_guard<mutex> obuflock(obuf);

            {
                lock_guard<mutex> lock(totalWriteAmountMtx);
                if (totalWriteAmount >= p.fp->dataAmount)
                {
                    {
                        lock_guard<mutex> lock(workingStateMtx);
                        workingState = 1;
                        cout << "changing workingState to 1" << endl;
                    }
                    cout << "createRunfile DONE!" << endl;
                    //obufCv.notify_one();
                    break;
                }               
            }

            curOpBuf = (activeBuf == 0) ? p.input1 : p.input2;
            //opt
            int32_t* nums = reinterpret_cast<int32_t*>(curOpBuf->buffer);
            if (curOpBuf->actualSize <= 0)//��ǰ������buf��û������
            {
                //����buf
                activeBuf ^= 1;
                cout << "MAIN THREAD: switching input buffer from " << (activeBuf ^ 1) << " to " << activeBuf << endl;
                //continue;//������һ�ֽ��н������б�Ҫ��

                //����Ƿ�ֻʣ�°������е�����
                {
                    lock_guard<mutex> lock(totalWriteAmountMtx);
                    //ֻʣ�����е�����
                    if (totalWriteAmount >= p.fp->dataAmount - k)
                    {
                        int32_t* outputBuf = reinterpret_cast<int32_t*>(p.output->buffer);
                        cout << "MAIN THREAD : clearing Loser Tree !" << endl;
                        while (1)
                        {
                            if (p.output->actualSize >= p.output->size)
                            {
                                cout << "MAIN THREAD: output buffer full, Stop to write!" << endl;
                                break;
                            }
                            try {
                                int32_t popVal = lt->pop();
                                outputBuf[p.output->pos++] = popVal;
                                p.output->actualSize++;
                            }
                            catch (const out_of_range& e)
                            {
                                cout << "Caught an exception: " << e.what() << endl;
                                break;
                            }
                        }
                    }
                }
            }

            //buf��������ʼ
            else
            {
                cout << "MAIN THREAD: run loser tree phrase " << endl;
                //��ʼ������Ƿ����ֱ�ӿ�ʼ�Ƚϣ���ʼ��ʱʹ��
                //��Ϊinputbuffer-size���ܴ���k������ڳ�ʼ����ʱ��
                bool canStart = false;//�����һ��inputbuffer����k
                bool justInit = false;
                //����Ƿ��ʼ��������
                //û���򴴽�
                if (lt == nullptr)
                {
                    cout << "MAIN THREAD: creating loser tree!" << endl;

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
                    {
                        lt = new LoserTree<int32_t>(k, leaves);
                        cout << "MAIN THREAD: loser tree created!" << endl;
                    }
                }

                int need2ChangeInputBuf = 0;
                //�Ѿ���ʼ����ɣ���ʼ
                if ((!justInit || canStart) && lt)
                {
                    cout << "MAIN THREAD: running loser tree!" << endl;
                    int32_t* outputBuf = reinterpret_cast<int32_t*>(p.output->buffer);
                    
                    while (curOpBuf->actualSize > 0)
                    {
                        //�����������
                        if (p.output->actualSize >= p.output->size)
                        {
                            cout << "MAIN THREAD: output buffer full, Stop to write!" << endl;
                            need2ChangeInputBuf = 1;
                            break;
                        }

                        //Ӧ�ò����µĹ鲢����
                        if (lt->banCount == k)
                        {
                            {
                                lock_guard<mutex> lock(curRunfileMtx);
                                curRunfile++;
                                maxRunfileNum = curRunfile;
                                curRunflleMin = INT32_MAX;
                                lt->reCompete();
                                cout << "MAIN THREAD: NEW RUNFILE IS CREATING!" << endl;
                                break;
                            }
                        }

                        //����Сֵ�������������
                        int32_t curWinner = lt->getWinner();
                        cout << "MAIN THREAD: curWinner is: " << curWinner << endl;
                        outputBuf[p.output->pos++] = curWinner;
                        p.output->actualSize++;
                        curRunflleMin = curWinner;//ÿ�ζ���Ҫ����

                        //���°�����
                        //�ж��Ƿ���Ҫ����
                        if (nums[curOpBuf->pos] < curRunflleMin)
                            lt->replaceWinnerAndBan(nums[curOpBuf->pos++]);
                        else 
                            lt->replaceWinner(nums[curOpBuf->pos++]);
                        curOpBuf->actualSize--;
                    }
                }

                if (need2ChangeInputBuf)
                {
                    activeBuf ^= 1;
                    cout << "MAIN THREAD: switching input buffer from " << (activeBuf ^ 1) << " to " << activeBuf << endl;
                }
            }
        }
        cout << "MAIN THREAD: wake up WRITE THREAD!" << endl;
        //�������������д�߳�
        obufCv.notify_one();
    }

    obufCv.notify_one();
    this_thread::sleep_for(std::chrono::seconds(1));
    obufCv.notify_one();
    reader.join();
    writer.join();
    return;
}

// �Զ���Ƚ���
struct compare {
    bool operator()(const pair<uint64_t, FileProcessor*>& a, const pair<uint64_t, FileProcessor*>& b) {
        // ���Ȱ��յ�һ��Ԫ�ش�С��������
        return a.first > b.first;
    }
};

void initHuffmanTree(priority_queue<pair<uint64_t, FileProcessor*>, vector<pair<uint64_t, FileProcessor*>>, compare>& pq, int& runfileCount)
{
    int maxRunfileCount = runfileCount;
    for (int i = 0; i < maxRunfileCount; ++i)
    {
        string filename = "run_" + to_string(i) + ".dat";
        FileProcessor* fp = new FileProcessor(filename.c_str());//������ǰrunfile��fp
        if (p.input1)
        {
            //��ȡrunfile��Ԫ����
            if (fp->loadMetaDataAndMallocBuf(*(p.input1)) == META_ERR)
            {
                logger.log(Log::ERROR, "INIT_HUFFMAN : runfile ", i, " 's meta data is wrong!");
                char* runfileName1 = newString(fp->filename);

                delete fp;

                //ɾ�������runfile�ļ�,�����û��ɾ���ɹ�
                if (remove(runfileName1) != 0)
                    logger.log(Log::ERROR, "INIT_HUFFMAN : can not remove file :", runfileName1);

                free(runfileName1);
                
                fp = nullptr;

                runfileCount--;//����ʵ��ӵ�е�runfile����
            }
        }
        if (fp != nullptr)
            pq.push({fp->dataAmount, fp});
    }
}

//huffman�ϲ�ʱ���õ��߳�
void huffmanMerge() {
    //ʹ�����ȶ��У��Լ�hisRun����
    int runfileCount = maxRunfileNum + 1;//��ǰӵ�е�runfile����
    int runfileMaxNum = runfileCount;//��һ������������runfile�ĺ���
    FileProcessor *file1 = nullptr, *file2 = nullptr;

    logger.log(Log::DEBUG, "before clear current runfileCount = ", runfileCount);

    hisRun = runfileCount;

    //���ȶ���ʵ��huffman�鲢
    priority_queue<pair<uint64_t, FileProcessor*>, vector<pair<uint64_t, FileProcessor*>>, compare> pq;

    initHuffmanTree(pq, runfileCount);
    logger.log(Log::DEBUG, "after clear, current runfileCount = ", runfileCount);
    while (runfileCount > 1)
    {
        pair<uint64_t, FileProcessor*> min1 = pq.top();
        file1 = min1.second;
        pq.pop(); runfileCount--;
        pair<uint64_t, FileProcessor*> min2 = pq.top();
        file2 = min2.second;
        pq.pop(); runfileCount--;
        
        //�ϲ�����runfile������һ���µ�
        FileProcessor* newRunfile = mergeRunfile(file1, file2);
        //���µ�runfile����huffman����
        pq.push({file1->dataAmount+ file2->dataAmount, newRunfile});
        runfileCount++;
        char* runfileName1 = newString(file1->filename);
        char* runfileName2 = newString(file2->filename);
        //�ͷ�ԭ�ȴ�����FileProcesser
        delete file1;
        delete file2;

        //ɾ���ɵ�runfile�ļ�,�������û��ɾ���ɹ�
        if (remove(runfileName1) != 0) 
            cerr << "can not remove file :" << runfileName1 << endl;
        if (remove(runfileName2) != 0)
            cerr << "can not remove file :" << runfileName2 << endl;

        free(runfileName1);
        free(runfileName2);
    }

    assert(!pq.empty());

    //�ͷ�ԭ�е�
    pair<uint64_t, FileProcessor*> res = pq.top();
    delete res.second;

    //�ͷ�p�ṹ��
    freePstruct(p);

    //����������ļ�
    string filename = "run_" + to_string(hisRun - 1) + ".dat";
    remove("result.dat");
    //����ǰ��Ҫ�ͷŶ�Ӧ�ļ���fileProcessor
    if (rename(filename.c_str(), "result.dat") != 0) {
        perror("Error renaming file");
    }

    FileProcessor file("result.dat");
    file.directLoadDataSet();

    logger.log(Log::INFO, "result.dat's dataAmount = ", file.dataAmount);
}

//������ͬ�鲢���ļ���runfile��
void genDiffRunfile(project& p, int inputBufSize, int outputBufSize, int k, const char* filename)
{
    assert(k >= 2);
    initP(p, inputBufSize, outputBufSize, HUFFMAN, filename);
    createDiffLenRuns(p, k);
}

//������ͬ�鲢���ļ���runfile������ȥ���հ��ļ�
int genDiffRunfileAndClear(project& p, int inputBufSize, int outputBufSize, int k, const char* filename)
{
    assert(k >= 2);
    initP(p, inputBufSize, outputBufSize, HUFFMAN, filename);
    createDiffLenRuns(p, k);
    int runfileCount = maxRunfileNum + 1;//��ǰӵ�е�runfile����
    int maxRunfileCount = runfileCount;
    FileProcessor* fp = nullptr;
    int actualRunfileNum = 0;
    char* runfileName = nullptr;
    bool isEmpty = false;
    for (int i = 0; i < maxRunfileCount; ++i)
    {
        isEmpty = false;
        string filename = "run_" + to_string(i) + ".dat";
        fp = new FileProcessor(filename.c_str());//������ǰrunfile��fp
        //��ȡrunfile��Ԫ����
        if (fp->checkMetaData() == META_ERR)
        {
            logger.log(Log::INFO, "Clear empty runfile ", i);
            runfileName = newString(fp->filename);

            delete fp;

            //ɾ�������runfile�ļ�,�����û��ɾ���ɹ�
            if (remove(runfileName) != 0)
                logger.log(Log::ERROR, "[func genDiffRunfileAndClear()] can not remove file :", runfileName);

            free(runfileName);

            fp = nullptr;

            runfileName = nullptr;

            runfileCount--;//����ʵ��ӵ�е�runfile����

            isEmpty = true;
        }
        if (fp != nullptr)
            delete fp;

        //������ǰrunfile����
        if (!isEmpty && i != actualRunfileNum)
        {
            string actualFilename = "run_" + to_string(actualRunfileNum++) + ".dat";
            if (rename(filename.c_str(), actualFilename.c_str()) != 0) {
                logger.log(Log::ERROR, "[func genDiffRunfileAndClear()] Error renaming file: ", filename, " to ", actualFilename);
            }
        }
        else if (!isEmpty && i == actualRunfileNum)
            actualRunfileNum++;
    }
    logger.log(Log::DEBUG, "total runfile number is ", runfileCount);
    logger.log(Log::DEBUG, "actualRunfileNum = ", actualRunfileNum);
    return runfileCount;
}

#define HUFFMAN_MERGE
#ifndef HUFFMAN_MERGE
int main()
{
    logger.setLogFile("ADS_project2.log");
    logger.setLogLevel(Log::DEBUG);
#define RUN
#ifdef RUN
    ////p�����������뻺������һ�����������
    //initP(p, 1000, 1000, HUFFMAN, "temp10000.dat");
    //cout << "--------ԭʼ����---------" << endl;
    ////p.fp->directLoadDataSet();
    //cout << "--------ԭʼ����---------" << endl << endl;

    ////opt, ����Ҫ�ֶ����ã���initP�е���
    //createDiffLenRuns(p, 50);
    genDiffRunfile(p, 1000, 1000, 50, "temp80000.dat");
    huffmanMerge();
    //hisRun������ͨ�ⲿ��·�鲢�б�ʹ��
    //cout << "runfileCount : " << hisRun << endl;
    showIOstatistic();
    //cout << "maxRunfileNum : " << maxRunfileNum << endl;
#endif // RUN
    
//#define CHECK_RESULT
#ifdef CHECK_RESULT
    FileProcessor file("run_1.dat");
    file.directLoadDataSet();
    cout << "--------------" << endl;
    cout << "file data amount : " << file.dataAmount << endl;
#endif

//#define TEST_GEN_CLEAR
#ifdef TEST_GEN_CLEAR
    genDiffRunfileAndClear(p, 1000, 1000, 50, "temp20000.dat");
#endif // TEST_GEN_CLEAR

	return 0;
}
#endif // !HUFFMAN_MERGE
