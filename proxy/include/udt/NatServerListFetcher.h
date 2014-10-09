// NatServerListFetcher.h: interface for the CNatServerListFetcher class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __NAT_SERVER_LIST_FETCHER__
#define __NAT_SERVER_LIST_FETCHER__

#include "NatPublicTypes.h"
#include "tinyxml.h"
#include "TVTHttp.h"
#include <vector>

class CNatServerListFetcher
{
public:
	CNatServerListFetcher();
	virtual ~CNatServerListFetcher();
public:
    /**
     * ����Ĭ�Ϸ�������ַ��ͨ��Ϊ����
     */
    void SetDefaultServerAddr(const char* addr);
    /**
     * ���÷������б�
     */
    void SetServerList(const NAT_SERVER_INFO* pServerList, int iCount);
    /**
     * �����Ƿ����ò��٣�Ĭ��Ϊ false
     */
    void SetTestSpeed(bool bTestSpeed);

    /**
     * ��ʼִ�л�ȡ����
     * �ڲ������������߳�
     */
    bool Start();

    /**
     * �Ƿ������
     * ��ʼ״̬Ϊtrue����ִ��Start()��Ϊfalse���ڴ�����ɺ�Ϊtrue����ʱ�ڲ��̻߳��Զ��˳�
     */
    bool IsCompleted();

    // ȡ��ִ�й��̣����ȴ�������ɺ󷵻�
    void Cancel();
    
	int GetServerCount();
    /**
     * ��ȡ�������б�
     * Ӧ���� IsCompleted() �� IsTestParted() �Ժ󣬲ſ��Ի�ȡ
     */
    bool GetServerInfo(NAT_SERVER_INFO *pServerInfo, int index);

    bool IsTestSpeed();

private:

    static PUB_THREAD_RESULT PUB_THREAD_CALL WorkThreadFunc(void *pParam);
    int RunWork();
private:
	typedef struct __SERVER_SPEED_INFO__
	{
		NAT_SERVER_INFO serverInfo;
		unsigned long remoteIp;
		unsigned long lastSendTime;
		int sendCount;
		bool hasReceived;
	}SERVER_SPEED_INFO;

	typedef std::vector<NAT_SERVER_INFO> NatServerVector;
	typedef std::list<SERVER_SPEED_INFO> NatServerList;
private:
	bool InitSpeedTestList(NatServerVector &serverVector);
	//���ط������б�
	bool FetchListFromServ(NatServerVector &serverVector, const char *pServerAddr);
	//�����������б�
	bool ParseServList(NatServerVector &serverVector, const char* pXmlSrc, int nXmlType = 0);
	//������������Ϣ
	int ParseServInfo(TiXmlElement* pXmlDoc, NAT_SERVER_INFO* pServInfo);
	//���
	void Clear();

	void ProcTestSpeed();
private:
	bool m_isCompleted;
	//����������б�
	NatServerVector m_retServerVector;
	//��ʱ�������б�
	NatServerVector m_cacheServerVector;
	// ���ط������б�
	NatServerVector m_downloadServerVector;
	// �����б�
	NatServerList   m_speedTestList;
	CTVTHttp *m_pHttp;
	CPUB_Lock m_lock;
	char m_szDefaultServerAddr[256];
    bool m_isTestSpeed;

    PUB_thread_t m_workThreadID;
    bool m_workThreadRunning;

};
#endif//__NAT_SERVER_LIST_FETCHER__