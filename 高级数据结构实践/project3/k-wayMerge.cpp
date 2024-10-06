#include"k-wayMerge.h"

KWayMerge kwm;
int activeBuf = 0;
mutex kwmMtx, kqMtx, obuf1Mtx, obuf2Mtx, activeBufMtx, bufPoolMtx;
condition_variable kqCv;
extern condition_variable obufCv;

LoserTree<int32_t> *lt = nullptr, *decidetree = nullptr;

//ÿk��runfile�ϲ�����ã�runfilesSize������Ҫ��̬����
void freeCurKRunfiles(KWayMerge& kwm)
{
	// �ͷ� runfiles���ļ������������ڴ�
	if (kwm.runfiles != nullptr) {
		for (int i = 0; i < kwm.runfilesSize; ++i) {
			if (kwm.runfiles[i] != nullptr) {
				delete kwm.runfiles[i];  // �ͷ�ÿ�� FileProcessor ����
				kwm.runfiles[i] = nullptr;
			}
		}
	}
}

//��ʼ���µ�k��runfile�ĺϲ�
void anotherKRunfilesAndLoadQ(KWayMerge& kwm)
{
	string openFilename = "run_" + to_string(kwm.maxOpRunfileNum++) + ".dat";
	//����k��runfile������
	kwm.runfiles = new FileProcessor * [kwm.runfilesSize];  // Ϊ kq ����ָ�� queue<Buf*> ��ָ������
	for (int i = 0; i < kwm.runfilesSize; ++i) {
		openFilename = "run_" + to_string(kwm.maxOpRunfileNum++) + ".dat";
		kwm.runfiles[i] = new FileProcessor(openFilename.c_str());  // Ϊÿ��ָ�����һ���µ� queue<Buf*>
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
	kwm.fp->loadMetaDataAndMallocBuf(*(kwm.obuf1));//��ȡ�ļ�Ԫ����
	encode = kwm.obuf1->encoding;
	kwm.obuf2->setEncodingAndMalloc(encode);
	kwm.curRunfileNum = runfileNum;
	kwm.maxRunfileNum = runfileNum - 1;
	kwm.maxOpRunfileNum = 0;
	//��ǰk���ļ�
	//�������Ű�����
	if (runfileNum < k)
		k = runfileNum;

	kwm.k = k;
	kwm.runfilesSize = k;
	kwm.kqSize = k;

	//����readDone�ռ�
	kwm.readDone = new vector<bool>(kwm.runfilesSize, false);

	//����k·�������
	kwm.kq = new queue<Buf*>*[kwm.kqSize];  // Ϊ kq ����ָ�� queue<Buf*> ��ָ������
	for (int i = 0; i < kwm.kqSize; ++i) {
		kwm.kq[i] = new queue<Buf*>();  // Ϊÿ��ָ�����һ���µ� queue<Buf*>
	}

	//����2k�����뻺��������1�����л���������
	kwm.bufPool = new queue<Buf*>();
	for (int i = 0; i < 2 * k; ++i)
	{
		curOpiBuf = new Buf(INPUT_BUF, inputBufSize);
		curOpiBuf->setEncodingAndMalloc(encode);
		kwm.bufPool->push(curOpiBuf);
	}

	//����������
	//�ڶ��߳��д������ǣ����߳�
	//������ǰ����Ҫ�ȶ�������
	anotherKRunfilesAndLoadQ(kwm);

	vector<int32_t> lastInQ(k, 0);//������ʼ��������,����һ�����ĸ��鲢�ε�����
	int32_t* opbuf = nullptr;
	//�ö��뻺���������һ�����ݳ�ʼ������������˭����һ��������
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

	//�ö��뻺�����ĵ�һ�����ݳ�ʼ�����鲢����ʼ��
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

//�ͷ�kwm�ṹ��
void freekwm(KWayMerge& kwm)
{
	// �ͷ���������� obuf1 �� obuf2
	if (kwm.obuf1 != nullptr) {
		delete kwm.obuf1;
		kwm.obuf1 = nullptr;
	}

	if (kwm.obuf2 != nullptr) {
		delete kwm.obuf2;
		kwm.obuf2 = nullptr;
	}

	// �ͷ� kq��k·������У����ڴ�
	if (kwm.kq != nullptr) {
		for (int i = 0; i < kwm.kqSize; ++i) {
			if (kwm.kq[i] != nullptr) {
				delete kwm.kq[i];  // �ͷ�ÿ�� queue<Buf*> ����
				kwm.kq[i] = nullptr;
			}
		}
		delete[] kwm.kq;  // �ͷ� kq ָ������
		kwm.kq = nullptr;
	}

	// �ͷ� runfiles���ļ������������ڴ�
	if (kwm.runfiles != nullptr) {
		for (int i = 0; i < kwm.runfilesSize; ++i) {
			if (kwm.runfiles[i] != nullptr) {
				delete kwm.runfiles[i];  // �ͷ�ÿ�� FileProcessor ����
				kwm.runfiles[i] = nullptr;
			}
		}
		delete[] kwm.runfiles;  // �ͷ� runfiles ָ������
		kwm.runfiles = nullptr;
	}

	// �ͷ� bufPool�����뻺�����أ����ڴ�
	if (kwm.bufPool != nullptr) {
		while (!kwm.bufPool->empty()) {
			Buf* buf = kwm.bufPool->front();
			delete buf;  // �ͷ�ÿ������������
			kwm.bufPool->pop();
		}
		delete kwm.bufPool;  // �ͷ� bufPool ���ж���
		kwm.bufPool = nullptr;
	}

	// �ͷ� FileProcessor ���� fp
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
		// ʹ�� string ���͵�����
		vector<string> stringInput;
		for (void* elem : input) {
			stringInput.push_back(*static_cast<string*>(elem));
		}
		return new LoserTree<string>(stringInput.size(), stringInput);
	}
	case ENC_INT16: {
		// ʹ�� int16_t ���͵�����
		vector<int16_t> int16Input;
		for (void* elem : input) {
			int16Input.push_back(*static_cast<int16_t*>(elem));
		}
		return new LoserTree<int16_t>(int16Input.size(), int16Input);
	}
	case ENC_INT32: {
		// ʹ�� int32_t ���͵�����
		vector<int32_t> int32Input;
		for (void* elem : input) {
			int32Input.push_back(*static_cast<int32_t*>(elem));
		}
		return new LoserTree<int32_t>(int32Input.size(), int32Input);
	}
	case ENC_INT64: {
		// ʹ�� int64_t ���͵�����
		vector<int64_t> int64Input;
		for (void* elem : input) {
			int64Input.push_back(*static_cast<int64_t*>(elem));
		}
		return new LoserTree<int64_t>(int64Input.size(), int64Input);
	}
	case ENC_FLOAT: {
		// ʹ�� float ���͵�����
		vector<float> floatInput;
		for (void* elem : input) {
			floatInput.push_back(*static_cast<float*>(elem));
		}
		return new LoserTree<float>(floatInput.size(), floatInput);
	}
	case ENC_DOUBLE: {
		// ʹ�� double ���͵�����
		vector<double> doubleInput;
		for (void* elem : input) {
			doubleInput.push_back(*static_cast<double*>(elem));
		}
		return new LoserTree<double>(doubleInput.size(), doubleInput);
	}
	default:
		// δ֪�������ͣ��׳��쳣
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
	//ͨ��������������Ҫ����һ���鲢��
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
			//��Ҫ�ȴ����߳�����װ��kwm.runfiles
			//��Ҫ����decidetree
			//ʹ��������������
		}
		runIndex = getToReadRunfile(decidetree);
		//��
		{
			lock_guard<mutex> lock(kqMtx);
			//���runfile�Ƿ��Ѿ����꣬�Ѿ����겻Ӧ�ó�Ϊʤ�ߣ���ֹ
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

			//���readfile2buffer��״̬
			//�ļ����ܸպö��꣬ʵ��Ӧ�ü��buf
			readStat = kwm.runfiles[runIndex]->readfile2buffer(*opBuf);
			logger.logAssert(readStat == DONE || readStat == CONTINUE, "error reading file");
			nums = reinterpret_cast<int32_t*>(opBuf->buffer);
			if (opBuf->actualSize > 0)
				decidetree->replaceWinner(nums[opBuf->actualSize - 1]);
			else
				decidetree->disqualify(runIndex);//����鲢�ļ��Ѿ�����

			kwm.kq[runIndex]->push(opBuf);//�������������Ӧ����
		}
	}

	
	//�����һ���鲢�����ȶ�������ô�죿
	//�Ǿ�ֱ�ӽ�����ǰҶ�ӾͿ���
	//���֪ͨ���߳��أ������ĸı�
	//
}

//���㵱ǰkwm.runfiles��Ӧ��д�����������ͬ����curRunfileSize��
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

			//���dataAmount����0������ʱ��Ϊû�ж�Ԫ����
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
	//�����������ͣ�����д
	//ʲôʱ��������ļ��أ���Ҫ��¼�����˶����µ�runfile��
	//��д���������������runfiles����֮��ʱ�������µ�runfile
	//��ֹ������ʲô��
	uint64_t totalWriteSize = 0;
	uint64_t curRunfileSize = 0;
	uint64_t curRunfileWriteSize = 0;
	string newRunfileName = "run_" + to_string(runfileMaxNum++) + ".dat";
	FileProcessor* newRunfile = new FileProcessor(newRunfileName.c_str());
	Buf* opBuf = nullptr;//��Ҫִ��д����������������
	int outputActiveBuf = 0;
	int eachWriteSize = 0;

	//�����¹鲢��Ӧ��д���������
	countDataAmount(kwm, curRunfileSize);

	while (true)
	{
		//�ж���һ���鲢�������Ƿ��Ѿ���������ֹд�߳�
		//ͨ��workingStat���ж�


		//�жϵ�ǰk���鲢���Ƿ�鲢����
		//�Ƿ�����µ�runfile
		if (curRunfileWriteSize == curRunfileSize)
		{
			newRunfile->updateMetaDataAmount(curRunfileSize);
			curRunfileSize = 0;
			delete newRunfile;

			//�����µ�runfile
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

		//��ȡ��׼��д
		//�費��Ҫ��activebuf��һ����
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
		//��������д���ļ������ʧ������ֹ����
		logger.logAssert(newRunfile->writebuffer2file(*opBuf) == OK, "TREAD WRITE write fail!");

		//����д��������
		totalWriteSize += eachWriteSize;
		curRunfileWriteSize += eachWriteSize;
		eachWriteSize = 0;
	}
}

void mergeKRunfiles(KWayMerge& kwm)
{
	Buf* opbuf = nullptr;//kq�����е�Buf
	int32_t* nums = nullptr;//opbuf��buffer
	Buf* outputBuf = nullptr;
	


	//��ȫ��Ҷ�ӽڵ㶼��������˵���ϲ����
	logger.logAssert(lt != nullptr, "EXIT! lt is NULL, and try to merger");
	
	while (!lt->isAllBan())
	{
		//�����߳�
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
		//ֱ�Ӻϲ�����Ϊ�ڳ�ʼ���׶ξ��Ѿ����뻺������
		mergeKRunfiles(kwm);

		//��ǰ�Ѵ�����ļ�����+��Ҫ��������� < ��ǰpass��runfile������
		if (i + 2 * kwm.runfilesSize <= kwm.curRunfileNum)
			anotherKRunfilesAndLoadQ(kwm);
		else
			break;
	}

	//���㲢����ʣ���runfile������,����runfilesSize,�������Ƿ���Ҫ���뻺����
	int factor = kwm.curRunfileNum / kwm.runfilesSize;//���Ǵ������ӣ������������ļ�������
	kwm.curRunfileNum = kwm.curRunfileNum - factor * kwm.runfilesSize + factor;

	//��Ҫ�ع���������
	if (kwm.curRunfileNum < kwm.k)
	{
		//����kwm.runfilesSize
		//�ع�������
	}
	else
		anotherKRunfilesAndLoadQ(kwm);

	return MERGE;
}

void kMerge(KWayMerge& kwm)
{
	//�����߳�
	thread reader(threadRead, ref(kwm));
	thread writer(threadWrite, ref(kwm.maxRunfileNum), ref(kwm), ref(activeBuf));
	int flag = MERGE;
	do {
		flag = kMergePass(kwm);
	} while (flag != OK);

	reader.join();
	writer.join();

	//�ͷ�kwm�ṹ��
	freekwm(kwm);

	//����������ļ�
	string filename = "run_" + to_string(kwm.maxRunfileNum) + ".dat";
	remove("result.dat");
	//����ǰ��Ҫ�ͷŶ�Ӧ�ļ���fileProcessor
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