
/*
 *  Description: 定义共享内存的存取方式及接口
 *  Author: chen.xiaowen
 */


#ifndef _SHARE_MEM_H_
#define _SHARE_MEM_H_

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <log4cxx/logger.h>



typedef enum _Status{
    eEmpty = 0,
    eInit =  1,
	eWait  = 2,
	eOver  = 3,
	eDeath = 4
}Status;

class ShareMemory{
public:
	ShareMemory():m_shmid(0),m_pBuffer(NULL), mHead(NULL){
		mShareLog = log4cxx::Logger::getLogger("shmd");
	}

	//共享内存发起者使用接口，创建共享内存和填数据；
	bool  CreateMemory(const char *str, int len, unsigned int &key);
	bool  FillShareKey(unsigned int key = 0);
	int   FillShareData(int type, int len, const char *pdata);
	void  NotifyFinish();

	//共享内存接收者使用，访问地址和读取数据；
	bool AccessMemory(unsigned int key);
	bool GetShareKey(unsigned int &o_key);
	int  ReadShareData(int &type, int &len, char *pData);
	bool SnapshotData(int &type, int &len, char *pData);

	void  Destroy(bool flag);

private:
	bool  Proberen(int semnum);
	bool  Verhogen(int semnum);
	unsigned int GeneratKey(const char *str, int len);


private:
	typedef struct{
		int semid;
		volatile int endflag;
		volatile int head;
		volatile int tail;
		volatile int useNum;
	}T_HeadInfo;

	enum{eShmEmpty = 0, eShmFull = 1,
		 eHeadLen = sizeof(T_HeadInfo),
		 eMaxLineLen = 512,
		 eMaxCount = 4096,
	     eDataLen = eMaxLineLen*eMaxCount};

	unsigned int mMagicKey;
	int   m_shmid;
	char  *m_pBuffer;
	T_HeadInfo *mHead;
	char mLogBuff[eMaxLineLen];
	log4cxx::LoggerPtr mShareLog;
};


#if 0
void ExitDestroy(int signal);
int CatchSignal(int signal, void(*handler)(int));
#endif


ShareMemory *InitReadShare();
int ReadShareData(ShareMemory *shm, int &type, int &len, char *pData);


#endif

