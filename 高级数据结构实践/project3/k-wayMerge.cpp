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
	//�����������ͣ�����д
	//ʲôʱ��������ļ��أ���Ҫ��¼�����˶����µ�runfile��
	//��д���������������runfiles����֮��ʱ�������µ�runfile
	//��ֹ������ʲô��
	uint64_t totalWriteSize = 0;
	uint64_t curRunfileSize = 0;
	string newRunfileName = "run_" + to_string(runfileMaxNum++) + ".dat";
	FileProcessor* newRunfile = new FileProcessor(newRunfileName.c_str());
	Buf* opBuf = nullptr;//��Ҫִ��д����������������

	//�����¹鲢��Ӧ��д���������
	

	while (true)
	{
		//�жϵ�ǰk���鲢���Ƿ�鲢����



		//�Ƿ�����µ�runfile
		{

		}

		//��ȡ��׼��д
		//�費��Ҫ��activebuf��һ����
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