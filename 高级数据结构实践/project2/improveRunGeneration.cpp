#include"improveRunGeneration.h"
#include "../Gdefine.h"
struct project p2;

uint64_t totalWriteAmount = 0;
long maxRunfileNum = 0;

mutex activeBufMtx, workingStateMtx, curRunfileMtx;//activeBuf的锁,以及workingState的君子锁

mutex buf1, buf2, obuf;//两个缓冲区的buf lock, 输出缓冲区锁

mutex totalWriteAmountMtx;

condition_variable obufCv;//output buffer 的条件变量，用于挂起等待outputbuffer满足写入条件

//将生成不同的runfile
void threadWriteFile(project& p, int workingState, long& curRunfile)
{
    //考虑两种情况
    //1.缓冲区满了
    //2.缓冲区没满但是，文件数据以及处理完
    //但是其实不需要考虑当前应该写入文件数据量的实际大小
    //只需要等待父进程释放锁就可以
    long tempRunfile = curRunfile;
    string runfileName = "run_" + to_string(tempRunfile) + ".dat";
    //为当前文件创建文件处理器
    FileProcessor* curFp = new FileProcessor(runfileName.c_str());

    //先写文件的数据区，后写meta data, 8字节
    uint64_t curRunfileDataAmount = 0;

    while (1)
    {
        //循环终止条件
        {
            lock_guard<mutex> lock(workingStateMtx);
            if (workingState)
            {
                //将runfile数据量写入润file中
                curFp->updateMetaDataAmount(curRunfileDataAmount);
                delete curFp;
                return;
            }
        }

        //检查是否产生新的归并段
        {
            lock_guard<mutex> lock(curRunfileMtx);
            //主线程中curRunfile更新说明已经产生新的归并段      
            if (curRunfile != tempRunfile)
            {
                tempRunfile = curRunfile;

                //将runfile数据量写入润file中
                curFp->updateMetaDataAmount(curRunfileDataAmount);

                delete curFp;

                //创建新的runfile
                runfileName = "run_" + to_string(tempRunfile) + ".dat";
                curFp = new FileProcessor(runfileName.c_str());
            }
        }

        //等待败者树释放锁，说明可写入
        //等待缓冲区可以写入
        unique_lock<mutex> lock(obuf);
        obufCv.wait(lock);//挂起，提高性能
        cout << "WRITE THREAD: writing buffer into runfile : " << tempRunfile << endl;
        //进行写入操作
        //写之前先更新数据量
        curRunfileDataAmount += p.output->actualSize;
        curFp->writebuffer2file(*(p.output));
        {
            lock_guard<mutex> lock(totalWriteAmountMtx);
            totalWriteAmount += p.output->actualSize;
            cout << "WRITE THREAD: total data amount write :" << totalWriteAmount << endl;
        }
    }

    if (curFp)
        delete curFp;
    return;
}



//从源文件中读数据到缓冲区
void threadReadFile(int& loadingBuf, project& p)
{
    //当一个缓冲区读完，更新activeBuf
    int res = CONTINUE;
    do {
        //保护读文件
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

        //读文件失败
        if (res != DONE && res != CONTINUE)
        {
            cerr << "Unknow read file state !" << endl;
            exit(1);
        }

        //到这里表示读文件成功
        {
            lock_guard<mutex> lock(activeBufMtx);
            loadingBuf ^= 1;//在0和1间切换
        }
    } while (res != DONE);

    cout << "src file done reading!\nquiting child process!" << endl;

    //源文件已经读完，终结子线程
    return;
}

//多编码赋值
void custom_E(int& activeBuf)
{

}

//创建不同归并段时采用多线程
void createDiffLenRuns(project& p, int k)
{
    //预处理k
    if (k > p.fp->dataAmount)
        k = p.fp->dataAmount;//opt, 不是很合理

    //败者树空时改变workingState
    int activeBuf = 0, workingState = 0, loadingBuf = 0;
    long curRunfile = 0;//当前产生的runfile号
    int32_t curRunflleMin = INT32_MAX;
    Buf* curOpBuf = nullptr;
    int leaveActSize = 0;
    int fillPos = 0;//leaves数组指向的索引，
    vector<int32_t> leaves(k, 0);
    //opt,先指定数据类型，后续需要修改支持数据多样化
    LoserTree<int32_t>* lt = nullptr;

    //创建线程
    thread reader(threadReadFile, ref(loadingBuf), ref(p));
    thread writer(threadWriteFile, ref(p), ref(workingState), ref(curRunfile));
    
    //启动其它操作，激活条件变量
    //在适当的位置调用 obufCv.notify_one() 或 obufCv.notify_all()
    //创建一个大小为k的vector从buf中获取数据（数据容量问题）
    //使用vector创建k路归并树
    //给activeBuf，obuf上锁，obuf满或是inputbuffer空时解锁
    //使用obufCv.notifyone()唤醒写线程
    //goto 132;
    //until 两个inputbuffer都为空
    //this_thread::sleep_for(std::chrono::seconds(1));
    while (1)//需要确定终止条件
    {
        {
            //通过activeBuf确定当前需要上锁的buffer
            lock_guard<mutex> ibuflock((activeBuf == 0) ? buf1 : buf2);
            lock_guard<mutex> obuflock(obuf);

            {
                lock_guard<mutex> lock(totalWriteAmountMtx);
                if (totalWriteAmount == p.fp->dataAmount)
                {
                    cout << "createRunfile DONE!" << endl;
                    {
                        lock_guard<mutex> lock(workingStateMtx);
                        workingState = 1;
                    }
                    break;
                }               
            }

            curOpBuf = (activeBuf == 0) ? p.input1 : p.input2;
            //opt
            int32_t* nums = reinterpret_cast<int32_t*>(curOpBuf->buffer);
            if (curOpBuf->actualSize <= 0)//当前交互的buf并没有数据
            {
                //交换buf
                activeBuf ^= 1;
                cout << "MAIN THREAD: switching input buffer from " << (activeBuf ^ 1) << " to " << activeBuf << endl;
                //continue;//跳过这一轮进行交换，有必要吗
            }

            //buf正常，开始
            else
            {
                cout << "MAIN THREAD: run loser tree phrase " << endl;
                //初始化完成是否可以直接开始比较，初始化时使用
                //因为inputbuffer-size可能大于k，因此在初始化的时候
                bool canStart = false;//代表第一次inputbuffer大于k
                bool justInit = false;
                //检查是否初始化败者树
                //没有则创建
                if (lt == nullptr)
                {
                    cout << "MAIN THREAD: creating loser tree!" << endl;

                    justInit = true;

                    //仅当bufferSize > k时初始化后可以直接运行
                    if (k < curOpBuf->actualSize)
                        canStart = true;

                    //首先填充vector-leaves
                    while (curOpBuf->pos < curOpBuf->actualSize && curOpBuf->pos < k)
                    {
                        leaves[fillPos++] = nums[curOpBuf->pos++];//应该使用多编码赋值
                        curOpBuf->actualSize--;
                    }

                    //初始化数组已满
                    if (fillPos >= k)
                    {
                        lt = new LoserTree<int32_t>(k, leaves);
                        cout << "MAIN THREAD: loser tree created!" << endl;
                    }
                }

                int need2ChangeInputBuf = 0;
                //已经初始化完成，开始
                if ((!justInit || canStart) && lt)
                {
                    cout << "MAIN THREAD: running loser tree!" << endl;
                    int32_t* outputBuf = reinterpret_cast<int32_t*>(p.output->buffer);

                    {
                        lock_guard<mutex> lock(totalWriteAmountMtx);
                        //只剩下树中的数据
                        if (totalWriteAmount == p.fp->dataAmount - k)
                        {
                            cout << "MAIN THREAD : clearing Loser Tree !" << endl;
                            while (1)
                            {
                                if (p.output->actualSize >= p.output->size)
                                {
                                    cout << "MAIN THREAD: output buffer full, Stop to write!" << endl;
                                    need2ChangeInputBuf = 1;
                                    break;
                                }
                                try {
                                    int32_t popVal = lt->pop();
                                    outputBuf[p.output->pos++] = popVal;
                                }
                                catch (const out_of_range& e)
                                {
                                    cout << "Caught an exception: " << e.what() << endl;
                                    break;
                                }
                            }
                        }
                    }
                    
                    while (curOpBuf->actualSize > 0)
                    {
                        //输出缓冲区满
                        if (p.output->actualSize >= p.output->size)
                        {
                            cout << "MAIN THREAD: output buffer full, Stop to write!" << endl;
                            need2ChangeInputBuf = 1;
                            break;
                        }

                        //应该产生新的归并段了
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

                        //将最小值放入输出缓冲区
                        int32_t curWinner = lt->getWinner();
                        cout << "MAIN THREAD: curWinner is: " << curWinner << endl;
                        outputBuf[p.output->pos++] = curWinner;
                        p.output->actualSize++;
                        if (curWinner < curRunflleMin)
                            curRunflleMin = curWinner;

                        //更新败者树
                        //判断是否需要竞赛
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
        //处理结束，激活写线程
        obufCv.notify_one();
    }
    obufCv.notify_one();
    reader.join();
    writer.join();
    return;
}


//huffman合并时采用单线程
void huffmanMerge() {
    //使用优先队列，以及hisRun属性
}


#define HUFFMAN_MERGE
#ifndef HUFFMAN_MERGE
int main()
{
    //p中有两个输入缓冲区和一个输出缓冲区
    initP(p, 20, 20, HUFFMAN);
    cout << "--------原始数据---------" << endl;
    p.fp->directLoadDataSet();
    cout << "--------原始数据---------" << endl << endl;

    //opt, 不需要手动调用，在initP中调用
    createDiffLenRuns(p, 8);
    cout << "maxRunfileNum : " << maxRunfileNum << endl;
	return 0;
}
#endif // !HUFFMAN_MERGE
