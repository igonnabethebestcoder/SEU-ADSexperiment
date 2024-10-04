#include"k-wayMerge.h"
//
mutex kwmMtx, kqMtx, obuf1Mtx, obuf2Mtx, activeBufMtx;
condition_variable kqCv;
extern condition_variable obufCv;

void* createDiffTypeLoserTree(int datatype)
{
	return nullptr;
}

int getToReadRunfile(void* losertree, int type)
{
	return 0;
}

void treadRead(int& toReadRunfile, KWayMerge& kwm)
{
	//通过败者树决定需要读哪一个归并段
	//读
	//如果有一个归并段率先读完了怎么办？
	//那就直接禁赛当前叶子就可以
	//如何通知主线程呢，变量的改变
	//
}

//计算当前kwm.runfiles中应该写入的数据量，同步到curRunfileSize中
void countDataAmount(KWayMerge& kwm, uint64_t& curRunfileSize)
{
	{
		lock_guard<mutex> lock(kwmMtx);
		if (kwm.runfiles == nullptr)
		{
			logger.log(Log::ERROR, "kwm.runfiles is NULL, need to initialize!");
			exit(1);
		}

		for (int i = 0; i < kwm.runfilesSize; ++i)
		{
			assert(kwm.runfiles != nullptr);
			if (kwm.runfiles[i]->dataAmount <= 0)
				if(kwm.runfiles[i]->checkMetaData() == META_ERR)

		}
	}
}

void treadWrite(int& runfileMaxNum, KWayMerge& kwm, int& activeBuf)
{
	//两个缓冲区就，换着写
	//什么时候产生新文件呢？需要记录产生了多少新的runfile吗？
	//当写入的数据数量等于runfiles数量之和时，产生新的runfile
	//终止条件是什么呢
	uint64_t totalWriteSize = 0;
	uint64_t curRunfileSize = 0;
	string newRunfileName = "run_" + to_string(runfileMaxNum++) + ".dat";
	FileProcessor* newRunfile = new FileProcessor(newRunfileName.c_str());
	Buf* opBuf = nullptr;//需要执行写入操作的输出缓冲区

	//计算新归并段应该写入的数据量
	

	while (true)
	{
		//判断当前k个归并段是否归并结束



		//是否产生新的runfile
		{

		}

		//获取锁准备写
		//需不需要给activebuf加一个锁
		{
			lock_guard<mutex> activeBufLock(activeBufMtx);
			opBuf = ((activeBuf ^ 1) == 0) ? kwm.obuf1 : kwm.obuf2;
			unique_lock<mutex> lock(((activeBuf ^ 1) == 0) ? obuf1Mtx : obuf2Mtx);
			logger.log(Log::DEBUG, "WRITE THREAD: Waiting for buffer to be available...");
			obufCv.wait(lock);
			logger.log(Log::DEBUG, "WRITE THREAD: Buffer available, proceeding to write to obuf", activeBuf ^ 1);
		}
		
		if (opBuf->actualSize <= 0)
		{
			logger.log(Log::DEBUG, "WRITE THREAD: opBuf size is 0, can't write!");
			continue;
		}


	}

}

void mergeKRunfiles()
{
	//当全部叶子节点都被禁赛，说明合并完成
}

void kMergePass()
{
}

void kMerge()
{
}


#define K_WAY_MERGE_MAIN
#ifdef K_WAY_MERGE_MAIN
int main()
{
	logger.setLogFile("ADS_project3.log");
	logger.setLogLevel(Log::DEBUG);

	return 0;
}
#endif