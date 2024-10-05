#include"k-wayMerge.h"
//
mutex kwmMtx, kqMtx, obuf1Mtx, obuf2Mtx, activeBufMtx;
condition_variable kqCv;
extern condition_variable obufCv;


void initkwm(KWayMerge& kwm, int& maxRunfileNum, int k)
{
	//��ǰk���ļ�
	//�������Ű�����
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
	//ͨ��������������Ҫ����һ���鲢��
	//��
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

void treadWrite(int& runfileMaxNum, KWayMerge& kwm, int& activeBuf)
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

void mergeKRunfiles()
{
	//��ȫ��Ҷ�ӽڵ㶼��������˵���ϲ����
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