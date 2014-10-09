// SWL_MultiNetComm.h: interface for the CNatMultiNetComm class.
//
//////////////////////////////////////////////////////////////////////

#ifndef  __NAT_MULTI_NET_MAT__
#define __NAT_MULTI_NET_MAT__

#include "SWL_Public.h"
#include "TVT_PubCom.h"
#include "NatSocket.h"

#include "SWL_MultiNet.h"

#define		NAT_MAX_KEEP_TIME		60000
//����������Դ�ṹ��
 typedef struct _nat_link_resource_
 {
 	NatSocket 		    sock;
 	long				deviceID;
 	long				bufferMaxLen;		//�����������ݵ�ͨ������ָ̬���������û����С
 	RECV_DATA_BUFFER	recvBuffer;			//Ҫ���յ�����
 	RECVASYN_CALLBACK	pfRecvCallBack;		//�������ݵĻص�����
 	void*				pParam;				//�ص���������
 	bool				bBroken;			//�Ƿ��ѶϿ�
 }NAT_LINK_RESOURCE;

class CNatMultiNetComm : public CSWL_MultiNet
{
public:
	CNatMultiNetComm();
	virtual ~CNatMultiNetComm();

public:
	bool Init();

	void Destroy();
	//�����µ���������
	int AddComm(long deviceID, TVT_SOCKET sock);

	//ɾ��ָ��ID����������
	int RemoveComm(long deviceID);

	//ɾ�����е���������
	int RemoveAllConnect();

	int SendData(long deviceID, const void *pBuf, int iLen, bool bBlock);

	//�����Զ��������ݵĻص�����
	int SetAutoRecvCallback(long deviceID, RECVASYN_CALLBACK pfRecvCallBack, void *pParam);

	//����ָ�����ӵĽ��ջ�������С
	void SetRecvBufferLen(long deviceID, long bufferLen);


private:
	int SendBuff(long deviceID, const void *pBuf, int iLen, bool bBlock, long lBlockTime);
	int RecvBuff(long deviceID, void *pBuf, int iLen, bool bBlock, long lBlockTime);
	static PUB_THREAD_RESULT PUB_THREAD_CALL RecvThread(void *pParam);
	void RecvThreadRun();

	bool GetLinkResourceByLinkID(long deviceID, NAT_LINK_RESOURCE * &pLinkResource);

	void UpdateRemoveComm();

private:
	list<NAT_LINK_RESOURCE *>				m_listNetLink;
	list<NAT_LINK_RESOURCE *>				m_tempDelNetLink;
	CPUB_Lock								m_lstLinkLock;
	CPUB_Lock								m_lstLinkLockForCallback;
	
//	int										m_maxSockInt;
	bool									m_bSockThreadRun;
 	NatSocketSet							m_fdReadSet;
	PUB_thread_t							m_threadId;
};

#endif // !defined(AFX_SWL_MULTINETCOMM_H__9E723A1A_97DE_445E_80BD_820A85DC30B6__INCLUDED_)
