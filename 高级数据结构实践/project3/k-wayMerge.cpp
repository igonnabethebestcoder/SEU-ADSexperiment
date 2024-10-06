#include"k-wayMerge.h"

KWayMerge kwm;
int activeBuf = 0;
mutex kwmMtx, kqMtx, obuf1Mtx, obuf2Mtx, activeBufMtx, bufPoolMtx;
condition_variable kqCv;
extern condition_variable obufCv;

LoserTree<int32_t> *lt = nullptr, *decidetree = nullptr;

//每k个runfile合并完调用，runfilesSize属性需要动态调整
void freeCurKRunfiles(KWayMerge& kwm)
{
	// 释放 runfiles（文件处理器）的内存
	if (kwm.runfiles != nullptr) {
		for (int i = 0; i < kwm.runfilesSize; ++i) {
			if (kwm.runfiles[i] != nullptr) {
				delete kwm.runfiles[i];  // 释放每个 FileProcessor 对象
				kwm.runfiles[i] = nullptr;
			}
		}
	}
}

//初始化新的k个runfile的合并
void anotherKRunfilesAndLoadQ(KWayMerge& kwm)
{
	string openFilename = "run_" + to_string(kwm.maxOpRunfileNum++) + ".dat";
	//分配k个runfile处理器
	kwm.runfiles = new FileProcessor * [kwm.runfilesSize];  // 为 kq 分配指向 queue<Buf*> 的指针数组
	for (int i = 0; i < kwm.runfilesSize; ++i) {
		openFilename = "run_" + to_string(kwm.maxOpRunfileNum++) + ".dat";
		kwm.runfiles[i] = new FileProcessor(openFilename.c_str());  // 为每个指针分配一个新的 queue<Buf*>
	}

	for (int i = 0; i < kwm.runfilesSize; ++i)
	{
		logger.logAssert(!kwm.bufPool->empty(), "buffer pool is empty, EXIT!");
		Buf* freeBuf = kwm.bufPool->front();
		int res = kwm.runfiles[i]->readfile2buffer(*freeBuf);
		if (res == DONE)
			(*kwm.readDone)[i] = true;
		kwm.kq[i]->push(freeBuf);
		kwm.bufPool->pop();
	}

	//
}

void initkwm(KWayMerge& kwm, int& runfileNum, int inputBufSize, int outputBufSize, int k, const char* filename)
{
	string openFilename = "run_" + to_string(0) + ".dat";
	Buf* curOpiBuf = nullptr;
	int encode = -1;
	kwm.fp = new FileProcessor(filename);
	kwm.obuf1 = new Buf(OUTPUT_BUF, outputBufSize);
	kwm.obuf2 = new Buf(OUTPUT_BUF, outputBufSize);
	kwm.fp->loadMetaDataAndMallocBuf(*(kwm.obuf1));//读取文件元数据
	encode = kwm.obuf1->encoding;
	kwm.obuf2->setEncodingAndMalloc(encode);
	kwm.curRunfileNum = runfileNum;
	kwm.maxRunfileNum = runfileNum - 1;
	kwm.maxOpRunfileNum = 0;
	//打开前k个文件
	//构造两颗败者树
	if (runfileNum < k)
		k = runfileNum;

	kwm.k = k;
	kwm.runfilesSize = k;
	kwm.kqSize = k;

	//分配readDone空间
	kwm.readDone = new vector<bool>(kwm.runfilesSize, false);

	//分配k路缓冲队列
	kwm.kq = new queue<Buf*>*[kwm.kqSize];  // 为 kq 分配指向 queue<Buf*> 的指针数组
	for (int i = 0; i < kwm.kqSize; ++i) {
		kwm.kq[i] = new queue<Buf*>();  // 为每个指针分配一个新的 queue<Buf*>
	}

	//分配2k个输入缓冲区，和1个空闲缓冲区队列
	kwm.bufPool = new queue<Buf*>();
	for (int i = 0; i < 2 * k; ++i)
	{
		curOpiBuf = new Buf(INPUT_BUF, inputBufSize);
		curOpiBuf->setEncodingAndMalloc(encode);
		kwm.bufPool->push(curOpiBuf);
	}

	//创建两个树
	//在读线程中创建还是，主线程
	//创建树前还需要先读入数据
	anotherKRunfilesAndLoadQ(kwm);

	vector<int32_t> lastInQ(k, 0);//用来初始化决策树,即下一个读哪个归并段的数据
	int32_t* opbuf = nullptr;
	//用读入缓冲区的最后一个数据初始化，用来决定谁读下一个缓冲区
	for (int i = 0; i < kwm.runfilesSize; ++i)
	{
		if (kwm.kq[i]->front()->actualSize > 0)
		{
			opbuf = reinterpret_cast<int32_t*>(kwm.kq[i]->front()->buffer);
			lastInQ[i] = opbuf[kwm.kq[i]->front()->actualSize - 1];
		}
		else
			logger.log(Log::ERROR, "[func initkwm] runfile ", i, " 's actualSize is 0!");
	}

	decidetree = new LoserTree<int32_t>(k, lastInQ);

	//用读入缓冲区的第一个数据初始化，归并树初始化
	for (int i = 0; i < kwm.runfilesSize; ++i)
	{
		if (kwm.kq[i]->front()->actualSize > 0)
		{
			opbuf = reinterpret_cast<int32_t*>(kwm.kq[i]->front()->buffer);
			lastInQ[i] = opbuf[0];
		}
		else
			logger.log(Log::ERROR, "[func initkwm] runfile ", i, " 's actualSize is 0!");
	}

	lt = new LoserTree<int32_t>(k, lastInQ);
}

//释放kwm结构体
void freekwm(KWayMerge& kwm)
{
	// 释放输出缓冲区 obuf1 和 obuf2
	if (kwm.obuf1 != nullptr) {
		delete kwm.obuf1;
		kwm.obuf1 = nullptr;
	}

	if (kwm.obuf2 != nullptr) {
		delete kwm.obuf2;
		kwm.obuf2 = nullptr;
	}

	// 释放 kq（k路缓冲队列）的内存
	if (kwm.kq != nullptr) {
		for (int i = 0; i < kwm.kqSize; ++i) {
			if (kwm.kq[i] != nullptr) {
				delete kwm.kq[i];  // 释放每个 queue<Buf*> 对象
				kwm.kq[i] = nullptr;
			}
		}
		delete[] kwm.kq;  // 释放 kq 指针数组
		kwm.kq = nullptr;
	}

	// 释放 runfiles（文件处理器）的内存
	if (kwm.runfiles != nullptr) {
		for (int i = 0; i < kwm.runfilesSize; ++i) {
			if (kwm.runfiles[i] != nullptr) {
				delete kwm.runfiles[i];  // 释放每个 FileProcessor 对象
				kwm.runfiles[i] = nullptr;
			}
		}
		delete[] kwm.runfiles;  // 释放 runfiles 指针数组
		kwm.runfiles = nullptr;
	}

	// 释放 bufPool（输入缓冲区池）的内存
	if (kwm.bufPool != nullptr) {
		while (!kwm.bufPool->empty()) {
			Buf* buf = kwm.bufPool->front();
			delete buf;  // 释放每个缓冲区对象
			kwm.bufPool->pop();
		}
		delete kwm.bufPool;  // 释放 bufPool 队列对象
		kwm.bufPool = nullptr;
	}

	// 释放 FileProcessor 对象 fp
	if (kwm.fp != nullptr) {
		delete kwm.fp;
		kwm.fp = nullptr;
	}

	if (lt)
		free(lt);

	if (decidetree)
		free(decidetree);
}

void* createDiffTypeLoserTree(vector<void*>& input, int datatype)
{
	switch (datatype) {
	case ENC_STRING: {
		// 使用 string 类型的数据
		vector<string> stringInput;
		for (void* elem : input) {
			stringInput.push_back(*static_cast<string*>(elem));
		}
		return new LoserTree<string>(stringInput.size(), stringInput);
	}
	case ENC_INT16: {
		// 使用 int16_t 类型的数据
		vector<int16_t> int16Input;
		for (void* elem : input) {
			int16Input.push_back(*static_cast<int16_t*>(elem));
		}
		return new LoserTree<int16_t>(int16Input.size(), int16Input);
	}
	case ENC_INT32: {
		// 使用 int32_t 类型的数据
		vector<int32_t> int32Input;
		for (void* elem : input) {
			int32Input.push_back(*static_cast<int32_t*>(elem));
		}
		return new LoserTree<int32_t>(int32Input.size(), int32Input);
	}
	case ENC_INT64: {
		// 使用 int64_t 类型的数据
		vector<int64_t> int64Input;
		for (void* elem : input) {
			int64Input.push_back(*static_cast<int64_t*>(elem));
		}
		return new LoserTree<int64_t>(int64Input.size(), int64Input);
	}
	case ENC_FLOAT: {
		// 使用 float 类型的数据
		vector<float> floatInput;
		for (void* elem : input) {
			floatInput.push_back(*static_cast<float*>(elem));
		}
		return new LoserTree<float>(floatInput.size(), floatInput);
	}
	case ENC_DOUBLE: {
		// 使用 double 类型的数据
		vector<double> doubleInput;
		for (void* elem : input) {
			doubleInput.push_back(*static_cast<double*>(elem));
		}
		return new LoserTree<double>(doubleInput.size(), doubleInput);
	}
	default:
		// 未知数据类型，抛出异常
		throw invalid_argument("Unknown data type");
	}
}

int getToReadRunfile(void* losertree, int type)
{
	switch (type) {
	case ENC_STRING:
	{
		/*LoserTree<string>* tree = static_cast<LoserTree<string>*>(losertree);
		return tree->getWinner();*/
	}
	case ENC_INT16:
	{
		LoserTree<int16_t>* tree = static_cast<LoserTree<int16_t>*>(losertree);
		return tree->getWinnerIndex();
	}
	case ENC_INT32:
	{
		LoserTree<int32_t>* tree = static_cast<LoserTree<int32_t>*>(losertree);
		return tree->getWinnerIndex();
	}
	case ENC_INT64:
	{
		LoserTree<int64_t>* tree = static_cast<LoserTree<int64_t>*>(losertree);
		return tree->getWinnerIndex();
	}
	case ENC_FLOAT:
	{
		LoserTree<float>* tree = static_cast<LoserTree<float>*>(losertree);
		return tree->getWinnerIndex();
	}
	case ENC_DOUBLE:
	{
		LoserTree<double>* tree = static_cast<LoserTree<double>*>(losertree);
		return tree->getWinnerIndex();
	}
	default:
	{
		cerr << "Unsupported data type!" << endl;
		return ENC_NOTKNOW;
	}
	}
}

void threadRead(int& toReadRunfile, KWayMerge& kwm)
{
	//通过败者树决定需要读哪一个归并段
	int runIndex = -1;
	Buf* opBuf = nullptr;
	int32_t* nums = nullptr;
	int readStat = 0;

	while (true)
	{
		if (kwm.runfilesSize == 1)
			break;

		if (decidetree->isAllBan())
		{
			//需要等待主线程重新装载kwm.runfiles
			//需要更新decidetree
			//使用条件变量挂起
		}
		runIndex = getToReadRunfile(decidetree);
		//读
		{
			lock_guard<mutex> lock(kqMtx);
			//检查runfile是否已经读完，已经读完不应该成为胜者，终止
			logger.logAssert(decidetree->isCompetitor(runIndex), "try to read done file! EXIT!");
			{
				lock_guard<mutex> lock(bufPoolMtx);
				if (!kwm.bufPool->empty())
				{
					opBuf = kwm.bufPool->front();
					kwm.bufPool->pop();
				}
				else
					logger.log(Log::WARNING, "READ TREAD : buffer pool is empty, this should not happend!");
			}
			if (opBuf == nullptr)
				continue;

			//检查readfile2buffer的状态
			//文件可能刚好读完，实际应该检查buf
			readStat = kwm.runfiles[runIndex]->readfile2buffer(*opBuf);
			logger.logAssert(readStat == DONE || readStat == CONTINUE, "error reading file");
			nums = reinterpret_cast<int32_t*>(opBuf->buffer);
			if (opBuf->actualSize > 0)
				decidetree->replaceWinner(nums[opBuf->actualSize - 1]);
			else
				decidetree->disqualify(runIndex);//这个归并文件已经读完

			kwm.kq[runIndex]->push(opBuf);//将缓冲区加入对应队列
		}
	}

	
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

void threadWrite(int& runfileMaxNum, KWayMerge& kwm, int& activeBuf)
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

void mergeKRunfiles(KWayMerge& kwm)
{
	Buf* opbuf = nullptr;//kq队列中的Buf
	int32_t* nums = nullptr;//opbuf的buffer
	Buf* outputBuf = nullptr;
	


	//当全部叶子节点都被禁赛，说明合并完成
	logger.logAssert(lt != nullptr, "EXIT! lt is NULL, and try to merger");
	
	while (!lt->isAllBan())
	{
		//唤醒线程
		obufCv.notify_one();

		{
			lock_guard<mutex> obuflock((activeBuf == 0) ? obuf1Mtx : obuf2Mtx);
			lock_guard<mutex> kqlock(kqMtx);
			outputBuf = (activeBuf == 0) ? kwm.obuf1 : kwm.obuf2;
			while (true)
			{
				if (outputBuf->actualSize >= outputBuf->size || lt->isAllBan())
					break;


			}
		}
	}
}

int kMergePass(KWayMerge& kwm)
{
	if (kwm.curRunfileNum < 2)
	{
		cout << "k-way merge done !" << endl;
		return OK;
	}

	for (int i = 0; i <= kwm.curRunfileNum; i += kwm.runfilesSize)
	{
		//直接合并，因为在初始化阶段就已经载入缓冲区了
		mergeKRunfiles(kwm);

		//当前已处理的文件数量+将要处理的数量 < 当前pass的runfile总数量
		if (i + 2 * kwm.runfilesSize <= kwm.curRunfileNum)
			anotherKRunfilesAndLoadQ(kwm);
		else
			break;
	}

	//计算并更新剩余的runfile的数量,调整runfilesSize,并决定是否需要载入缓冲区
	int factor = kwm.curRunfileNum / kwm.runfilesSize;//既是处理因子，又是生成新文件的数量
	kwm.curRunfileNum = kwm.curRunfileNum - factor * kwm.runfilesSize + factor;

	//需要重构败者树吗
	if (kwm.curRunfileNum < kwm.k)
	{
		//更新kwm.runfilesSize
		//重构败者树
	}
	else
		anotherKRunfilesAndLoadQ(kwm);

	return MERGE;
}

void kMerge(KWayMerge& kwm)
{
	//创建线程
	thread reader(threadRead, ref(kwm));
	thread writer(threadWrite, ref(kwm.maxRunfileNum), ref(kwm), ref(activeBuf));
	int flag = MERGE;
	do {
		flag = kMergePass(kwm);
	} while (flag != OK);

	reader.join();
	writer.join();

	//释放kwm结构体
	freekwm(kwm);

	//重命名结果文件
	string filename = "run_" + to_string(kwm.maxRunfileNum) + ".dat";
	remove("result.dat");
	//改名前需要释放对应文件的fileProcessor
	if (rename(filename.c_str(), "result.dat") != 0) {
		perror("Error renaming file");
	}

	FileProcessor file("result.dat");
	file.directLoadDataSet();
}


#define K_WAY_MERGE_MAIN
#ifdef K_WAY_MERGE_MAIN
int main()
{
	logger.setLogFile("ADS_project3.log");
	logger.setLogLevel(Log::DEBUG);
	int runfileNum = 0;
	runfileNum = genDiffRunfileAndClear(p, 1000, 1000, 50, "temp80000.dat");
	freePstruct(p);
	initkwm(kwm, runfileNum, 1000, 1000, 8, "temp80000.dat");
	return 0;
}
#endif