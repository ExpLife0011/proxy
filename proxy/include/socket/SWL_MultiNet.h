// SWL_MultiNetComm.h: interface for the CSWL_MultiNetComm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SWL_MULTINET_H__
#define __SWL_MULTINET_H__

#include "SWL_Public.h"
#include "TVT_PubCom.h"

#include <string.h>
#include <list>

using namespace std;

typedef struct _recv_data_buffer_
{
	long			bufferSize;	//����������
	long			dataSize;	//�����������ݴ�С
	char			*pData;		//�������ݵĻ�����
	long			dataType;	//���ڽ���һЩ�������ݣ�Ϊ������ǰ�Ĵ���ģ�����
}RECV_DATA_BUFFER;

//���ݴ���ɹ�����0�����򷵻�-1����ʱ���������״̬
typedef int (*RECVASYN_CALLBACK)(long clientID, void* pParam, RECV_DATA_BUFFER *pDataBuffer);

class CSWL_MultiNet 
{
public:
	CSWL_MultiNet();
	virtual ~CSWL_MultiNet();

public:
	virtual bool Init();

	virtual void Destroy();
	//�����µ���������
	virtual int AddComm(long deviceID, TVT_SOCKET sock);

	//ɾ��ָ��ID����������
	virtual int RemoveComm(long deviceID);

	//ɾ�����е���������
	virtual int RemoveAllConnect();

	virtual int SendData(long deviceID, const void *pBuf, int iLen, bool bBlock);

	//�����Զ��������ݵĻص�����
	virtual int SetAutoRecvCallback(long deviceID, RECVASYN_CALLBACK pfRecvCallBack, void *pParam){return 0;}

	//����ָ�����ӵĽ��ջ�������С
	virtual void SetRecvBufferLen(long deviceID, long bufferLen){}
};

#endif
