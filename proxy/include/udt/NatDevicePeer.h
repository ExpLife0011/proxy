// NatDevicePeer.h: interface for the CNatDevicePeer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NAT_DEVICE_PEER_H_
#define _NAT_DEVICE_PEER_H_

#include "NatCommon.h"
#include <vector>
#include <string>
#include "NatListenProc.h"
#include "NatSocketBase.h"
#include "NatUdtTransport.h"
#include "NatTraversalDevicePeer.h"
#include "NatUdt.h"
#include "RelayObj.h"

class CNatDevicePeer;

class CNatUdtDeviceClient: public CNatSocketBase, CUdtNotifier
{
public:
    static const int UDT_SEND_WINDOW_SIZE_MAX = 170; // * CNatUdt::SEND_PACKET_BUF_SIZE = 92 KB
    static const int UDT_RECV_WINDOW_SIZE_MAX = 171; // * CNatUdt::RECV_PACKET_BUF_SIZE = 256 K

    enum CLIENT_COME_IN_MODE
    {
        MODE_DIRECT,    // ֱ�������ģʽ
        MODE_TRAVERSAL, // ��͸�����ģʽ
        MODE_MIXED      // ���ģʽ���п��ܿͻ����ǶԳ���NAT
    };
public:
    CNatUdtDeviceClient(CNatDevicePeer *devicePeer, unsigned long connectionId,
        unsigned long fromIP, unsigned short fromPort, CLIENT_COME_IN_MODE comeInMode);
    virtual ~CNatUdtDeviceClient();

    virtual void RecvPacket(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    bool IsMineDatagram(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    bool IsSymmetricClientDatagram(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    void Reset(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    CLIENT_COME_IN_MODE GetComeInMode();
public:
    virtual int Send(const void *pBuf, int iLen);

    virtual int Recv(void *pBuf, int iLen);
    /**
     * ��ȡ��ǰ���Է������ݵ��ֽڴ�С
     * @return ���û�д����򷵻ص�ǰ���Է������ݵ��ֽڴ�С��0��ʾ���ͻ��������ˣ����򣬳�����-1
     */
    virtual int GetSendAvailable();

    /**
     * ��ȡ���Խ������ݵ��ֽڴ�С����Ҫ����������������
     * @return ���û�д����򷵻ص�ǰ���Խ������ݵ��ֽڴ�С��0��ʾ�����ݣ����򣬳�����-1
     */
    virtual int GetRecvAvailable();

    virtual int SetAutoRecv(bool bAutoRecv);

    virtual int Close();

    virtual unsigned long GetRemoteIp();

    virtual unsigned short GetRemotePort();

	virtual unsigned long GetConnectionId();
public:
    virtual void OnConnect(CNatUdt *pUDT, int iErrorCode);

    virtual int OnRecv(CNatUdt *pUDT, const void* pBuf, int iLen);

    virtual int OnSendPacket(CNatUdt *pUDT, const void* pBuf, int iLen);
public:
    virtual NatRunResult Run();
private:
    void ChangeDisConnected();
private:
    NAT_UDT_CONFIG m_config;
    CNatDevicePeer *m_devicePeer;
    CNatUdt m_udt;
    bool m_isClosed;
    CPUB_Lock m_lock;
    CLIENT_COME_IN_MODE m_comeInMode;
};

// TODO: CNatRelayDeviceClient;


/**
 * �豸���
 */
class CNatDevicePeer: public CRelayNotifier
{
    friend class CNatUdtDeviceClient;
public:
    CNatDevicePeer();
    virtual ~CNatDevicePeer();

    /**
     * ��ʼ��
     * @param config �豸���á�
     * @return ����ɹ�������0�����򷵻�����ֵ��
     */    
	int Start(const NAT_DEVICE_CONFIG_EX *config);
    /**
     * ֹͣ��
     * @return ����ɹ�������0�����򷵻�����ֵ��
     */
    int Stop();
    /**
     * �Ƿ��ѿ�ʼ��
     * @return �����Ƿ��ѿ�ʼ��
     */
    bool IsStarted();


	 /*brief 
	 * ������NatServer������
	 */
	int ResetServer(const char *serverIp, unsigned short serverPort);

	/**
	 * �����豸����չ��Ϣ
	 * @param[in] extraInfo �豸��չ��Ϣ
	 */
	void SetDeviceExtraInfo(const char* extraInfo);

    /**
     * ����֪ͨ�ص���
     * ֻ֧�ֵ�֪ͨ�ص���
     * @param notifier ֪ͨ�ص�
     */
    void SetConnectNotifier( CNatSocketConnectNotifier* notifier);

    CNatSocketConnectNotifier* GetConnectNotifier();
    
	void RelayOnConnect(CNatSocketBase* pSockBase, int iErrorCode);


	NAT_DEVICE_SERVER_ERROR GetServerError();

	NAT_DEVICE_SERVER_STATE GetServerState();

    // TODO: Get informations 
public:
    ////////////////////////////////////////////////////////////////////////////////
	// interface for class CNatTraversalDevicePeer

	int  OnP2PClientConnect(unsigned long connectionId, unsigned long clientPeerIp,
        unsigned short clientPeerPort);

	void OnRelayClientConnect(unsigned long connectionId, unsigned long relayServerIp, 
        unsigned short relayServerPort);
private:
    CNatUdtDeviceClient* FindUdtClient(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    CNatUdtDeviceClient* FindUdtTraversalClient(const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    CNatUdtDeviceClient* FindUdtClient(unsigned long connectionId,unsigned long fromIP, unsigned short fromPort);
    CNatUdtDeviceClient* CreateUdtClient(unsigned long connectionId,unsigned long fromIP, 
        unsigned short fromPort, CNatUdtDeviceClient::CLIENT_COME_IN_MODE comeInMode);
	CRelayObj* FindRelayClient(unsigned long connectionID);
	CRelayObj* CreateRelayClient(unsigned long connectionID, unsigned long serverIp,unsigned short serverPort);

    void InternalStop();
    NatRunResult ClearClosedClients();
private:
    void NotifyOnConnect(CNatSocketBase* sockBase);
private:
    static PUB_THREAD_RESULT PUB_THREAD_CALL WorkThreadFunc(void *pParam);
    int RunWork();
private:
    /**
     * ������ӦCNatUdtTransport��֪ͨ�ص����������ݽ��зַ���
     */
    class CUdtTransportDispatch: public CNatUdtTransportNotifier
    {
    public:
        CUdtTransportDispatch();

        void Init(CNatDevicePeer* devicePeer);

        virtual void OnRecvDatagram(CNatUdtTransport *transport, const NAT_TRANSPORT_RECV_DATAGRAM* datagram);
    private:
        CNatDevicePeer* m_devicePeer;
    };

    friend class CUdtTransportDispatch;
private:
    bool m_started;
    char m_deviceNo[64];    /**< �豸�����к� */
    char m_serverIp[256];      /**< NAT������IP��ַ */
    unsigned short m_serverPort; /**< NAT�������˿ں� */
    CNatSocketConnectNotifier *m_connectNotifier;
    std::vector<CNatUdtDeviceClient*> m_udtClients;
	std::vector<CRelayObj*> m_RelayClients;
    CNatUdtTransport m_udtTransport;
    CUdtTransportDispatch m_udtTransportDispatch;
	CNatTraversalDevicePeer m_traversalDevicePeer;

    bool m_workThreadRunning;
    PUB_thread_t m_workThreadID;
    CPUB_Lock m_clientsLock;
    CPUB_Lock m_notifierLock;
};

#endif //_NAT_DEVICE_PEER_H_