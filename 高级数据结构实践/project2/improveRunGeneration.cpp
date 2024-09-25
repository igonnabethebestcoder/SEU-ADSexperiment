#include"improveRunGeneration.h"

struct project p2;

mutex activeBufMtx, workingStateMtx, curRunfileMtx;//activeBuf的锁,以及workingState的君子锁

mutex buf1, buf2, obuf;//两个缓冲区的buf lock, 输出缓冲区锁

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

        //进行写入操作
        //写之前先更新数据量
        curRunfileDataAmount += p.output->actualSize;
        curFp->writebuffer2file(*(p.output));
    }

    if (curFp)
        delete curFp;
    return;
}

//从源文件中读数据到缓冲区
void threadReadFile(int& activeBuf, project& p)
{
    //当一个缓冲区读完，更新activeBuf
    int res = CONTINUE;
    do {
        //保护读文件
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

        //读文件失败
        if (res != DONE && res != CONTINUE)
        {
            cerr << "Unknow read file state !" << endl;
            exit(1);
        }

        //到这里表示读文件成功
        {
            lock_guard<mutex> lock(activeBufMtx);
            activeBuf ^= 1;//在0和1间切换
        }
    } while (res != DONE);

    cout << "src file done reading!\nquiting child process!" << endl;

    //源文件已经读完，终结子线程
    return;
}


//创建不同归并段时采用多线程
void createDiffLenRuns(project& p, int k)
{
    //败者树空时改变workingState
    int activeBuf = 0, workingState = 0;
    long curRunfile = 0;

    //创建线程
    thread reader(threadReadFile, ref(activeBuf), ref(p));
    thread writer(threadWriteFile, ref(p), ref(workingState), ref(curRunfile));
    
    

    //启动其它操作，激活条件变量
    //在适当的位置调用 obufCv.notify_one() 或 obufCv.notify_all()

    reader.join();
    writer.join();
    return;
}


//huffman合并时采用单线程
void huffmanMerge() {

}


#define HUFFMAN_MERGE
#ifndef HUFFMAN_MERGE
int main()
{
    //p中有两个输入缓冲区和一个输出缓冲区
    initP(p, 10, 20, HUFFMAN);
    cout << "--------原始数据---------" << endl;
    p.fp->directLoadDataSet();
    cout << "--------原始数据---------" << endl << endl;
    cout << "生成的runfile个数 :" << p.runAmount << endl;
	return 0;
}
#endif // !HUFFMAN_MERGE
