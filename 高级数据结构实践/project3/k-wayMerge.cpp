#include"k-wayMerge.h"
//
mutex kwmMtx, kqMtx, obuf1Mtx, obuf2Mtx, activeBufMtx;
condition_variable kqCv;
extern condition_variable obufCv;


void initkwm(KWayMerge& kwm, int& maxRunfileNum, int k)
{
	//打开前k个文件
	//构造两颗败者树
}

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
	curRunfileSize = 0;//
	{
		lock_guard<mutex> lock(kwmMtx);
		logger.logAssert(kwm.runfiles != nullptr, "runfiles is not init yet! EXIT!");

		for (int i = 0; i < kwm.runfilesSize; ++i)
		{
			if (kwm.runfiles[i] == nullptr)
				continue;

			//如果dataAmount等于0，可能时因为没有读元数据
			int res = -2;
			if (kwm.runfiles[i]->dataAmount <= 0)
				res = kwm.runfiles[i]->checkMetaData();

			switch (res)
			{
			case ERR:
				logger.log(Log::ERROR, "file is not open pleas check!"); break;
			case META_ERR:
				logger.logAssert(false, "[func countDataAmount] runfile is invalid!"); break;
			default:
				curRunfileSize += kwm.runfiles[i]->dataAmount;
				break;
			}
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
	uint64_t curRunfileWriteSize = 0;
	string newRunfileName = "run_" + to_string(runfileMaxNum++) + ".dat";
	FileProcessor* newRunfile = new FileProcessor(newRunfileName.c_str());
	Buf* opBuf = nullptr;//需要执行写入操作的输出缓冲区
	int outputActiveBuf = 0;
	int eachWriteSize = 0;

	//计算新归并段应该写入的数据量
	countDataAmount(kwm, curRunfileSize);

	while (true)
	{
		//判断整一个归并排序上是否已经结束，终止写线程
		//通过workingStat来判断


		//判断当前k个归并段是否归并结束
		//是否产生新的runfile
		if (curRunfileWriteSize == curRunfileSize)
		{
			newRunfile->updateMetaDataAmount(curRunfileSize);
			curRunfileSize = 0;
			delete newRunfile;

			//创建新的runfile
			newRunfileName = "run_" + to_string(runfileMaxNum++) + ".dat";
			newRunfile = new FileProcessor(newRunfileName.c_str());

			countDataAmount(kwm, curRunfileSize);
			curRunfileWriteSize = 0;
		}
		
		if (totalWriteSize >= kwm.fp->dataAmount)
		{
			logger.log(Log::DEBUG, "A merge pass has been done, pass totalWriteSize = ", totalWriteSize);
			totalWriteSize = 0;
		}

		//获取锁准备写
		//需不需要给activebuf加一个锁
		{
			lock_guard<mutex> activeBufLock(activeBufMtx);
			outputActiveBuf = (activeBuf ^ 1);
		}
		opBuf = (outputActiveBuf == 0) ? kwm.obuf1 : kwm.obuf2;
		unique_lock<mutex> lock((outputActiveBuf == 0) ? obuf1Mtx : obuf2Mtx);
		logger.log(Log::DEBUG, "WRITE THREAD: Waiting for buffer to be available...");
		obufCv.wait(lock);
		logger.log(Log::DEBUG, "WRITE THREAD: Buffer available, proceeding to write to obuf", activeBuf ^ 1);

		if (opBuf->actualSize <= 0)
		{
			logger.log(Log::DEBUG, "WRITE THREAD: opBuf size is 0, can't write!");
			continue;
		}
		eachWriteSize = opBuf->actualSize;
		//将缓冲区写入文件，如果失败则终止程序
		logger.logAssert(newRunfile->writebuffer2file(*opBuf) == OK, "TREAD WRITE write fail!");

		//更新写入数据量
		totalWriteSize += eachWriteSize;
		curRunfileWriteSize += eachWriteSize;
		eachWriteSize = 0;
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