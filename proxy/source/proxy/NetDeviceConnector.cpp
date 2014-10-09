#include "NetDeviceConnector.h"
#include "DeviceSearcher.h"
#include "NatCommon.h"
#include "NatDeviceFetcher.h"
#include "SWL_ConnectProcEx.h"
#include "NatConnectProc.h"
#include "TVTHttp.h"
#include "NatServerListFetcher.h"
#include "AvosServer.h"
#include "tinyxml.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <string>
#include <string.h>
#include <WinSock2.h>

#ifndef WIN32
#include <sys/socoet.h>
#else
#include <winsock.h>
#endif

using namespace std;

#define NET_IS_INVALIDE_PORT(n)			(n>20)
#define NET_IS_INVALIDE_IP(ip)			(strlen(ip) > 6)

//************�������**************//

class CNatDevTimer
{
public:
	CNatDevTimer()
	{
		_start = clock();
		_end = _start;
	}

	~CNatDevTimer()
	{

	}

	void Start()
	{
		_start = clock();
	}

	clock_t Stop()
	{
		_end = clock();
		return (_end - _start);
	}

	static time_t GetCurTime()
	{
		return time(NULL);
	}

private: 
	clock_t _start;
	clock_t _end;
};

class DevConnect
{
public:
	virtual ~DevConnect(){}
	// ����
	virtual int	Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)=0;
};

/****************/
// NAT�����豸
class DevConnectNAT: public DevConnect
{
public:
	typedef struct _QueryDevThread_Param
	{
		NAT_DEVICE_FETCHER_CONFIG	config;
		NAT_SERVER_INFO*			pNetServerInfo;
		CPUB_Lock*					pLock;
		unsigned long				iTimeOut;
	}QueryDevThreadParam;

	DevConnectNAT(){}
	~DevConnectNAT(){}

	// clientͨ��NAT�����豸
	virtual int	Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult);
private:
	// ���±���NAT�б�
	int		UpdateNATList(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult);

	// ��NAT�������ϲ����豸
	int		QueryDev(NET_DEVICE_CONN_RESULT *pResult, bool *bIsCanceled, int iTimeout, NAT_SERVER_INFO& natSerInfo);

	// ����NAT������
	int		ConnectNAT(const NAT_CLIENT_CONFIG& config, NatSocket& sock, int iTimeout);

private:
	static PUB_THREAD_RESULT PUB_THREAD_CALL QueryDevThread(void *pParam);
};

/****************/
// TCP�����豸
class DevConnectTCP: public DevConnect
{
public:
	DevConnectTCP(const char* pDevIP, int nDevPort);
	~DevConnectTCP(){}

	// TCP�����豸
	virtual int	Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult);

private:
	char	m_szDevIP[64];
	int		m_nDevPort;
};

/****************/
// �����������豸
class DevConnectLocal: public DevConnect
{
public:
	DevConnectLocal(){}
	~DevConnectLocal(){}

	// ������ɨ�������豸
	virtual int	Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult);

private:
	// ��ȡ������ɨ����豸�б�
	int		LocalScanDev(const char* pDevSN, char* pDevIP, int& nDevPort, int iTimeout, bool *bIsCanceled);
};
//================�������==================//


//*****************ʵ��******************//

int DevConnectNAT::Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)
{
	int	nRet = NET_DEV_CON_RET_ERROR;
	NatSocket				natSocket = NULL;
	CNatConnectProc*		pConnect = NULL;
	NAT_CLIENT_CONFIG		config;

	// �������û�����豸��Ϣ����
	if(pConfig->pDeviceConnCache)
	{
		// ����Ĭ��ֵ
		nRet = NET_DEV_CON_RET_DEV_NOT_EXIST;

		// ʹ�ñ����豸��Ϣ����
		config.ConfigType = TRAVERSAL_CONFIG;
		config.TraversalConfig.nServerPort = pConfig->pDeviceConnCache->sServerInfo.sPort;
		strcpy(config.TraversalConfig.dwDeviceNo, pConfig->szDeviceNo);
		strcpy(config.TraversalConfig.pServerIp, pConfig->pDeviceConnCache->sServerInfo.szAddr);

		// �����豸
		nRet = ConnectNAT(config, natSocket, iTimeout);
		if(nRet == NET_DEV_CON_RET_SUCCESS)
		{
			cout << "����";
		}
	}

	// ����NAT����ʧ�ܣ��Ҳ����豸
	if(NET_DEV_CON_RET_ERROR == nRet || NET_DEV_CON_RET_DEV_NOT_EXIST == nRet)
	{
		// ����NAT��ַ�б�
		UpdateNATList(pConfig, iTimeout, bIsCanceled, pResult);

		// ��NAT�в����豸��Ϣ
		NAT_SERVER_INFO natSerInfo = {0};
		nRet = QueryDev(pResult, bIsCanceled, iTimeout, natSerInfo);
		if(NET_DEV_CON_RET_SUCCESS == nRet)
		{
			// �����ҵ����豸��NAT��Ϣ
			if(pResult->pDeviceConnCache == NULL)
			{
				pResult->pDeviceConnCache = new NET_DEVICE_CONN_CACHE();
			}
			if(pResult->pDeviceConnCache)
			{
				pResult->pDeviceConnCache->sServerInfo = natSerInfo;
				pResult->pDeviceConnCache->iLastConnectTime = CNatDevTimer::GetCurTime();
			}

			config.ConfigType = TRAVERSAL_CONFIG;
			config.TraversalConfig.nServerPort = natSerInfo.sPort;
			strcpy(config.TraversalConfig.pServerIp, natSerInfo.szAddr);
			strcpy(config.TraversalConfig.dwDeviceNo, pConfig->szDeviceNo);

			// ʹ�û�ȡ���豸��Ϣ�����豸
			nRet = ConnectNAT(config, natSocket, iTimeout);
		}
	}

	if(nRet == NET_DEV_CON_RET_SUCCESS)
	{
		pResult->sock.natSock = natSocket;
		pResult->sockType = NET_TRANS_NAT;
		pResult->pDeviceConnCache->iLastConnectTime = CNatDevTimer::GetCurTime();

		cout << "NAT���ӳɹ�" << endl;

	}
	return nRet;
}

int DevConnectNAT::UpdateNATList(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)
{
	int nRet = NET_DEV_CON_RET_ERROR;
	CNatServerListFetcher fetcher;
	CNatDevTimer natTime;

	if(strlen(pConfig->szServerAddr))
	{
		fetcher.SetDefaultServerAddr(pConfig->szServerAddr);
		fetcher.Start();
		natTime.Start();
		while(natTime.Stop() < iTimeout)
		{
			// �������ȡ��
			if(bIsCanceled && *bIsCanceled==true)
			{
				fetcher.Cancel();
				break;
			}

			// ��ȡ�б����
			if(fetcher.IsCompleted())
			{
				break;
			}
		}

		int nSerNum = fetcher.GetServerCount();

		if(nSerNum > 0)
		{
			// ɾ��ԭ�ȵ��б�
			delete[] pResult->pServerInfoList;
			pResult->iServerInfoCount = 0;
			pResult->pServerInfoList = new NAT_SERVER_INFO[nSerNum];

			if(pResult->pServerInfoList)
			{
				pResult->iServerInfoCount = nSerNum;
				// ��ȡ�б�
				for (int i=0; i<nSerNum; i++)
				{
					fetcher.GetServerInfo(pResult->pServerInfoList+i, i);
				}
			}
		}
	}
	else
	{
		nRet = NET_DEV_CON_RET_INVALIDE_PARAM;
	}
	
	return nRet;
}


PUB_THREAD_RESULT PUB_THREAD_CALL DevConnectNAT::QueryDevThread( void *pParam )
{
	int							nRet = NET_DEV_CON_RET_ERROR;
	CNatDeviceFetcher*			pConnect;
	QueryDevThreadParam*		pConfig = (QueryDevThreadParam*)pParam;
	NAT_DEVICE_INFO				natDevInfo = {0};

	if(pConfig)
	{
		pConnect = CNatDeviceFetcher::NewDeviceFetcher(&(pConfig->config));
		if(pConnect)
		{
			nRet = pConnect->FetchSyn(pConfig->iTimeOut, &natDevInfo);
			pConnect->Destroy();
			// �ҵ��豸
			if(!nRet)
			{
				pConfig->pLock->Lock();
				// �����豸
				if(!NET_IS_INVALIDE_PORT(pConfig->pNetServerInfo->sPort))
				{
					pConfig->pNetServerInfo->sPort = pConfig->config.nServerPort;
					strcpy(pConfig->pNetServerInfo->szAddr, pConfig->config.pServerIp);
				}
				pConfig->pLock->UnLock();
			}
		}
	}
	return nRet;
}


int DevConnectNAT::QueryDev( NET_DEVICE_CONN_RESULT *pResult, bool *bIsCanceled, int iTimeout,  NAT_SERVER_INFO& natSerInfo )
{
	int nRet = NET_DEV_CON_RET_ERROR;

	if(pResult && pResult->pServerInfoList)
	{
		CPUB_Lock					lockInfo;
		int							nThread = 0;
		PUB_thread_t*				pThread = NULL;
		QueryDevThreadParam*		pConfig = NULL;
		size_t						i = 0;

		pThread = new PUB_thread_t[pResult->iServerInfoCount];
		pConfig = new QueryDevThreadParam[pResult->iServerInfoCount];
		natSerInfo.sPort = 0;

		if(pThread && pConfig)
		{
			for (i=0; i<pResult->iServerInfoCount; i++)
			{
				// �鿴�Ƿ��Ѿ��ҵ��豸
				lockInfo.Lock();
				if(natSerInfo.sPort>0)
				{
					lockInfo.UnLock();
					break;
				}
				lockInfo.UnLock();

				pConfig[i].pLock = &lockInfo;
				pConfig[i].pNetServerInfo = &natSerInfo;
				strcpy(pConfig[i].config.dwDeviceNo, pResult->szDeviceNo);
				strcpy(pConfig[i].config.pServerIp, pResult->pServerInfoList[i].szAddr);
				pConfig[i].config.nServerPort = pResult->pServerInfoList[i].sPort;
				pConfig[i].iTimeOut = iTimeout;

				pThread[i] = PUB_CreateThread(DevConnectNAT::QueryDevThread, pConfig+i, NULL);
			}

			// ֹͣ
			CNatDevTimer natTime;
			natTime.Start();
			while(natTime.Stop() < iTimeout)
			{
				lockInfo.Lock();
				if(NET_IS_INVALIDE_PORT(natSerInfo.sPort))
				{
					lockInfo.UnLock();
					nRet = NET_DEV_CON_RET_SUCCESS;
					break;
				}
				else if(bIsCanceled && *bIsCanceled==true)
				{
					nRet = NET_DEV_CON_RET_DEV_NOT_EXIST;
					lockInfo.UnLock();
					break;
				}
				lockInfo.UnLock();
			}

			// ֹͣ�����߳�
			for (size_t j=0; j<i; j++)
			{
				PUB_ExitThread(pThread+j, NULL);
			}
		}

		delete[] pThread;
		pThread = NULL;
		delete[] pConfig;
		pConfig = NULL;
	}

	return nRet;
}

int DevConnectNAT::ConnectNAT( const NAT_CLIENT_CONFIG& config, NatSocket& sock, int iTimeout )
{
	int nRet = NET_DEV_CON_RET_ERROR;
	// ���Խ���NAT����
	CNatConnectProc* pConnect = CNatConnectProc::NewConnectProc(&config);
	if(pConnect)
	{
		sock = pConnect->ConnectSyn(iTimeout);
		if(sock)
		{
			nRet = NET_DEV_CON_RET_SUCCESS;
		}
		pConnect->Destroy();
	}
	else
	{
		// ����ʧ�ܵ�ԭ��
		NAT_CLIENT_STATE state = pConnect->GetConnectState();
		if(state==NAT_CLI_STATE_P2P_DEVICE || state==NAT_CLI_STATE_RELAY_DEVICE )
		{
			nRet = NET_DEV_CON_RET_DEV_NOT_CONNECT;
		}
		else
		{
			nRet = NET_DEV_CON_RET_DEV_NOT_EXIST;
		}
	}
	return nRet;
}

DevConnectTCP::DevConnectTCP(const char* pDevIP, int nDevPort)
{
	strcpy(m_szDevIP, pDevIP);
	m_nDevPort = nDevPort;
}

int DevConnectTCP::Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)
{
	int	nRet = NET_DEV_CON_RET_ERROR;

	// ����TCP����
	CSWL_ConnectProcEx* pSock = CSWL_ConnectProcEx::NewConnectProc(m_szDevIP, m_nDevPort);
	if(pSock)
	{
		// ��������
		pResult->sock.swlSocket = pSock->ConnectSyn(iTimeout);
		if(pResult->sock.swlSocket != SWL_INVALID_SOCKET)
		{
			nRet = NET_DEV_CON_RET_SUCCESS;
			cout << "TCP���ӳɹ�" << endl;
		}
		pSock->Destroy();
	}

	return nRet;
}

int DevConnectLocal::Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)
{
	int		nRet = NET_DEV_CON_RET_ERROR;
	char	szDevIP[32] = {0};
	int		nDevPort = 0;

	// �������豸ɨ��
	nRet = LocalScanDev(pConfig->szDeviceNo, szDevIP, nDevPort, iTimeout, bIsCanceled);
	if(NET_DEV_CON_RET_SUCCESS == nRet)
	{
		// ����TCP����
		CSWL_ConnectProcEx* pSock = CSWL_ConnectProcEx::NewConnectProc(szDevIP, nDevPort);
		if(pSock)
		{
			// ��������
			pResult->sock.swlSocket = pSock->ConnectSyn(iTimeout);
			if(pResult->sock.swlSocket != SWL_INVALID_SOCKET)
			{
				nRet = NET_DEV_CON_RET_SUCCESS;
				cout << "���������ӳɹ�" << endl;
			}
			pSock->Destroy();
		}
	}
	else
	{
		cout << "������û��ɨ�赽�豸 : " << pConfig->szDeviceNo << endl;
	}
	return nRet;
}


int DevConnectLocal::LocalScanDev(const char* pDevSN, char* pDevIP, int& nDevPort, int iTimeout, bool *bIsCanceled )
{
	int nRet = NET_DEV_CON_RET_ERROR;

	//// ������ɨ���豸
	TVT_DISCOVER_DEVICE		devInfo;
	CNatDevTimer			natTime;
	int						nCount = 0;

	CDeviceSearcher& devSearcher = CDeviceSearcher::Instance();
	devSearcher.StartSearchThread();

	cout << "ɨ���豸��...\n";
	natTime.Start();
	while(natTime.Stop() < iTimeout)
	{
		if(devSearcher.GetDeviceInfo(pDevSN, devInfo))
		{
			strcpy(pDevIP, devInfo.ip);
			nDevPort = devInfo.port;
			nRet = NET_DEV_CON_RET_SUCCESS;
			break;
		}

		// �ⲿ����ȡ��
		if(bIsCanceled && *bIsCanceled==true)
		{
			break;
		}
	}

	if(0)
	{
		TVT_DEV_LIST li;
		TVT_DEV_LIST::iterator ite;
		devSearcher.GetDeviceList(li);
		ite = li.begin();
		while(ite != li.end())
		{
			cout << ite->sn << endl;
			ite++;
		}
	}

	devSearcher.StopSearchThread();

	return nRet;
}


int NetDeviceConnector::Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult)
{
	int	nRet = NET_DEV_CON_RET_ERROR;

	if(pConfig==NULL || pResult==NULL || strlen(pConfig->szDeviceNo)==0)
	{
		return NET_DEV_CON_RET_INVALIDE_PARAM;
	}
	else
	{
		strcpy(pResult->szDeviceNo, pConfig->szDeviceNo);
	}

	CONNECT_THREAD_PARAM config;
	CNatDevTimer		 natTimer;

	config.bIsCanceled = bIsCanceled;
	config.iTimeout = iTimeout;
	config.pConfig = pConfig;
	config.pResult = pResult;
	config.nError = nRet;
	config.bFinished = false;

	// ���������߳�
	PUB_thread_t threadConn = PUB_CreateThread(NetDeviceConnector::ConnectThread, &config, NULL);
	natTimer.Start();
	while(natTimer.Stop() < iTimeout)
	{
		if(config.bFinished)
		{
			break;
		}
	}
	PUB_ExitThread(&threadConn, NULL);

	return config.nError;
}


int NetDeviceConnector::GetDeviceInfo(const CManagerServer* pMS, const char* pDevSN, char* pDevIP, int& nDevPort )
{
	int nRet = NET_DEV_CON_RET_ERROR;
	CManagerServer* pServer = NULL;
	CManagerServer* pDeafultSer = NULL;

	if(pMS)
	{
		pServer = const_cast<CManagerServer*>(pMS);
	}
	else
	{
		pDeafultSer = new CAvosServer();
		pServer = pDeafultSer;
	}

	if(pServer)
	{
		string			strIP;
		unsigned short	nPort;
		bool			bRet = false;

		do
		{
			if(!pServer->Initial(pDevSN))
			{
				break;
			}

			if(!pServer->ReadInfo())
			{
				break;
			}

			if(!pServer->GetNetInfo(strIP, nPort))
			{
				break;
			}

			if(!NET_IS_INVALIDE_IP( strIP.c_str() ) || !NET_IS_INVALIDE_PORT(nPort))
			{
				cout << "�Ƿ����豸��Ϣ : [" << strIP << "\t" << nPort << "]" << endl;
				break;
			}

			pServer->Quit();

			strcpy(pDevIP, strIP.c_str());
			nDevPort = nPort;
			nRet = NET_DEV_CON_RET_SUCCESS;

			cout << "��ȡ���豸��Ϣ : [" << strIP << "\t" << nPort << "]" << endl;
		}while(0);

		if(!bRet)
		{
			nRet = NET_DEV_CON_RET_ERROR;
		}
	}

	delete pDeafultSer;
	pDeafultSer = NULL;
	return nRet;
}

PUB_THREAD_RESULT PUB_THREAD_CALL NetDeviceConnector::ConnectThread( void *pParam )
{
	int						nRet = NET_DEV_CON_RET_ERROR;
	DevConnect*				pDevConnect = NULL;
	CONNECT_THREAD_PARAM*	p = reinterpret_cast<CONNECT_THREAD_PARAM*>(pParam);

	if(p)
	{
		p->bFinished = false;
		pDevConnect = new DevConnectLocal();
		if(pDevConnect)
		{
			// ����Ĭ����������
			p->pResult->sockType = NET_TRANS_COMMON;

			// ���Ծ��������ӣ�ʧ���ٳ�������
			nRet = pDevConnect->Connect(p->pConfig, p->iTimeout, p->bIsCanceled, p->pResult);
			if(NET_DEV_CON_RET_SUCCESS != nRet)
			{
				delete pDevConnect;
				pDevConnect = NULL;

				char	szDevIP[64] = {0};
				int		nDevPort = 0;

				// �ӹ���������ϻ�ȡ�豸��TCP��Ϣ�����ʧ��ʹ��NAT���ӷ�ʽ
				// �����������������
				nRet = GetDeviceInfo(p->pConfig->pManagerServer, p->pConfig->szDeviceNo, szDevIP, nDevPort);
				if(NET_DEV_CON_RET_SUCCESS != nRet)
				{
					pDevConnect = new DevConnectNAT();
					p->pResult->sockType = NET_TRANS_NAT;
				}
				else
				{
					pDevConnect = new DevConnectTCP(szDevIP, nDevPort);
					p->pResult->sockType = NET_TRANS_COMMON;
				}

				nRet = pDevConnect->Connect(p->pConfig, p->iTimeout, p->bIsCanceled, p->pResult);
			}
		}

		delete pDevConnect;
		pDevConnect = NULL;

		p->nError = nRet;
		p->bFinished = true;
	}
	return nRet;
}
