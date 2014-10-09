// NatListenProc.cpp: implementation of the CNatListenProc class.
//
//////////////////////////////////////////////////////////////////////

#include "NatCommon.h"
#include <string.h>

#ifdef   __ENVIRONMENT_LINUX__
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#endif

#include "NatListenProc.h"
#include "NatDevicePeer.h"
#include "NatSocketBase.h"
#include "NatXml.h"

#include "NatDebug.h"

#ifndef __NAT_DEVICE_CONFIG_EX__

struct CNatListenProc::DEV_EXTRA_INFO
{
	NAT_DEVICE_EXTRA_INFO info;
};

#endif//__NAT_DEVICE_CONFIG_EX__

class CNatListenProc::COnConnectNotifier: public CNatSocketConnectNotifier
{
public:
    COnConnectNotifier(CNatListenProc *listenProc)
        :m_listenProc(listenProc)
    {
    }
    virtual void OnConnect(NatSocket sock, int iErrorCode)
    {
        assert(sock);
        printf("CNatListenProc::COnConnectNotifier recv callback OnConnect\n");

        if (sock != NAT_SOCKET_INVALID)
        {
            NAT_SOCKET_INFO info;
            if(NAT_GetSocketInfo(sock, info) == 0)
            {
                m_listenProc->OnConnect(sock, info.remoteIp, info.remotePort);
            }
            else
            {
                NAT_CloseSocket(sock);
            }
        }
    };

private:
    CNatListenProc *m_listenProc;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNatListenProc::CNatListenProc(NAT_ACCEPT_LINK_CALLBACK pCallback, void* pParam)
{
	assert(NULL != pCallback);
	m_pAcceptCallback = pCallback;
	m_pParam = pParam;
	m_isStarted = false;
    m_onConnectNotifier = new COnConnectNotifier(this);
	m_pNatDevicePeer	= NULL;

	PUB_InitLock(&m_listenLock);
}
CNatListenProc::~CNatListenProc()
{
    if(m_pNatDevicePeer)
    {
        delete m_pNatDevicePeer;
        m_pNatDevicePeer = NULL;
    }
	PUB_DestroyLock(&m_listenLock);

	delete m_onConnectNotifier;
}

void CNatListenProc::OnConnect(NatSocket sock, unsigned long iIP, unsigned short nPort)
{
#ifdef NAT_TVT_DVR_4_0
    m_clientSockInfo.tvtSock.natSock = sock;
    in_addr ipAddr;
    ipAddr.s_addr = iIP;
    strcpy(m_clientSockInfo.szIP, inet_ntoa(ipAddr));
	NET_TRANS_TYPE serverType = NET_TRANS_NAT;
#else
    m_clientSockInfo.pTcpSock = (SWL_socket_t)sock;
    m_clientSockInfo.dwIP   = iIP;
    m_clientSockInfo.nPort = nPort;
    SERVER_TYPE serverType = SERVER_NAT;
#endif//NAT_TVT_DVR_4_0

	//���ö�����ʱע��Ļص�����^
	int ret = m_pAcceptCallback(&m_clientSockInfo, m_pParam, &serverType);
	if (-1 == ret)
	{
		NAT_LOG_DEBUG("CNatListenProc OnConnect not accept the new NatSocket\n");
		NAT_CloseSocket(sock);
	}
}
 
//����socket,����ʼ�����˿�,����accept�̣߳�accept�߳̽�����ʱ��socket������
//return value: 0 �ɹ�  1 ����socketʧ�� 2 ��ȡ���������߻�ȡ����IP��ַʧ��
//				3 ��socketʧ��  4 ����ʧ��  5 ���������߳�ʧ��
//				6 �Ѿ���ʼ�˼���
int CNatListenProc::StartListen(const NAT_DEVICE_CONFIG* pConfig)
{
	//������StartListen�ķ��ؽ��
	int iRet = 0;
	CPUB_LockAction   FunctionLock(&m_listenLock); //������������

	//����Ѿ���ʼ�˼�������ֱ�ӷ���
	if (m_isStarted)
	{
		return 0;
	}

	m_pNatDevicePeer = new CNatDevicePeer();
    if (NULL == m_pNatDevicePeer)
    {
        Clear();
        return -1;
    }

	const NAT_DEVICE_CONFIG_EX *configEx = NULL;
#ifdef __NAT_DEVICE_CONFIG_EX__ 
	// for ��չ��
	configEx = pConfig;
#else
	// for ��׼��
	m_extraInfo = new CNatListenProc::DEV_EXTRA_INFO;
	if (NULL == m_extraInfo)
	{
		Clear();
		return -1;
	}
	m_extraInfo->info.deviceType = pConfig->deviceType;
	strcpy(m_extraInfo->info.deviceVersion, pConfig->deviceVersion);
	m_extraInfo->info.deviceWebPort = pConfig->deviceWebPort;
	m_extraInfo->info.deviceDataPort = pConfig->deviceDataPort;

	NAT_DEVICE_CONFIG_EX deviceConfigEx;
	memset(&deviceConfigEx, 0, sizeof(deviceConfigEx));
	strcpy(deviceConfigEx.deviceNo, pConfig->deviceNo);
	strcpy(deviceConfigEx.serverIp, pConfig->serverIp);
	deviceConfigEx.serverPort = pConfig->serverPort;
	deviceConfigEx.localPort = pConfig->localPort;
	deviceConfigEx.localIp = pConfig->localIp;
	deviceConfigEx.refuseRelay = pConfig->refuseRelay;
	CNatTraversalXmlPacker::PackDeviceExtraInfo(&m_extraInfo->info, deviceConfigEx.caExtraInfo, sizeof(deviceConfigEx.caExtraInfo));

	configEx = & deviceConfigEx;
#endif//__NAT_DEVICE_CONFIG_EX__

    m_pNatDevicePeer->SetConnectNotifier(m_onConnectNotifier);
	iRet = m_pNatDevicePeer->Start(configEx);
    if (iRet < 0)
    {
        Clear();
        return -1;
    }
    NAT_LOG_INFO("CNatListenProc StartListen ok!\n");
    m_isStarted = true;
	return iRet;
}

//ֹͣ�����˿�,����socket,socket�����ٷ�����accept�̵߳Ľ���
//return value: 0 �ɹ�
//				1 �Ѿ�ֹͣ
int CNatListenProc::StopListen()
{
    if (m_isStarted)
    {
        NAT_LOG_INFO("CNatListenProc StopListen!\n");
        Clear();
        m_isStarted = false;
    }
	return 0;
}

bool CNatListenProc::IsStarted()
{
    return m_isStarted;
}

void CNatListenProc::ResetServer( const char *serverIp, unsigned short serverPort )
{
    if (m_isStarted)
    {
        NAT_LOG_DEBUG("CNatListenProc ResetServer(%s:%d)\n", serverIp, serverPort);
        m_pNatDevicePeer->ResetServer(serverIp, serverPort);
    }
}
#ifdef __NAT_DEVICE_CONFIG_EX__   

void CNatListenProc::SetDeviceExtraInfo(const char* extraInfo)
{
	if (m_isStarted)
	{
		m_pNatDevicePeer->SetDeviceExtraInfo(extraInfo);
	}
}

#else

void CNatListenProc::SetDeviceWebPort( unsigned short deviceWebPort )
{
    if (m_isStarted)
    {
		m_extraInfo->info.deviceWebPort = deviceWebPort;
		UpdateExtraInfo();
    }
}

void CNatListenProc::SetDeviceDataPort( unsigned short deviceDataPort )
{
    if (m_isStarted)
	{
		m_extraInfo->info.deviceDataPort = deviceDataPort;
		UpdateExtraInfo();
    }
}

#endif//__NAT_DEVICE_CONFIG_EX__

NAT_DEVICE_SERVER_STATE CNatListenProc::GetServerState()
{
    if (m_isStarted)
    {
        return m_pNatDevicePeer->GetServerState();
    }
    return NAT_STATE_NONE;
}

NAT_DEVICE_SERVER_ERROR CNatListenProc::GetServerError()
{
    if (m_isStarted && GetServerState() == NAT_STATE_DISCONNECTED)
    {
        return m_pNatDevicePeer->GetServerError();
    }
    return NAT_ERR_OK;
}

void CNatListenProc::Clear()
{
    if (m_pNatDevicePeer != NULL)
    {
        m_pNatDevicePeer->Stop();
        delete m_pNatDevicePeer;
        m_pNatDevicePeer = NULL;
    }
}

#ifndef __NAT_DEVICE_CONFIG_EX__   
void CNatListenProc::UpdateExtraInfo()
{
	char infoText[512] = {0};
	CNatTraversalXmlPacker::PackDeviceExtraInfo(&m_extraInfo->info, infoText, sizeof(infoText));
	m_pNatDevicePeer->SetDeviceExtraInfo(infoText);
}
#endif //__NAT_DEVICE_CONFIG_EX__
