// NetDeviceConnector.h: interface for the CNetDeviceConnector class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __NET_DEVICE_CONNECTOR__
#define __NET_DEVICE_CONNECTOR__

#include "SWL_TypeDefine.h"
#include "NatPublicTypes.h"
#include "ManagerServer.h"
#include <string>
#include <vector>

 /* ������ */
#define NET_DEV_CON_RET_SUCCESS					0		// �ɹ�
#define NET_DEV_CON_RET_ERROR					1		// һ�����
#define NET_DEV_CON_RET_DEV_NOT_EXIST			2		// NAT���Ҳ����豸��Ϣ
#define NET_DEV_CON_RET_DEV_NOT_CONNECT			3		// NAT���Ӳ����豸
#define NET_DEV_CON_RET_INVALIDE_PARAM			4		// ��������

typedef struct __net_device_conn_cache__
{
	NAT_SERVER_INFO sServerInfo;						// �豸ע��Ĵ�͸��������Ϣ
	unsigned long	iLastConnectTime;					// �ϴ����ӵ�ʱ�䣬unit: second
}NET_DEVICE_CONN_CACHE;

typedef struct __net_device_conn_config__
{
	char					szDeviceNo[64];				// �豸���к�
	char					szServerAddr[128];			// ��͸��������ַ
	unsigned short			sServerPort;				// ��͸�������˿ں�
	CManagerServer			*pManagerServer;			// ����������������ΪNULL����ʾ��֧�ֹ��������
	int						iServerInfoCount;			// ��͸�����������棩���������Ϊ0�������pServerInfoList
	NAT_SERVER_INFO			*pServerInfoList;			// ��͸�������б�ָ�룬���ΪNULL����ʾ������
	NET_DEVICE_CONN_CACHE	*pDeviceConnCache;			// �豸���ӻ��棬���ΪNULL����ʾ������

}NET_DEVICE_CONN_CONFIG;


typedef struct __nat_device_conn_result__
{
	char					szDeviceNo[64];				// �豸���к�
	NET_TRANS_TYPE			sockType;					// ����Socket������
	TVT_SOCKET				sock;						// ����Socket
	int						iServerInfoCount;			// ��͸�����������棩���������Ϊ0�������pServerInfoList
	NAT_SERVER_INFO			*pServerInfoList;			// ��͸�������б�ָ�룬���ΪNULL����ʾ������
	NET_DEVICE_CONN_CACHE	*pDeviceConnCache;			// �豸���ӻ��棬���ΪNULL����ʾ������

}NET_DEVICE_CONN_RESULT;


// �����࣬����������豸������	
// 
class NetDeviceConnector
{
public:
	NetDeviceConnector(){}
	~NetDeviceConnector(){}
	
	/** 
	 * �����豸�����к�,������豸���ӳɹ������ظ����ӵ���������ͬʱ�ɵ���������ָ�����ӵ�����
	 * @param[in] pConfig ��������
	 * @param[in] iTimeout ���ӳ�ʱʱ�䣬��λ��ms
	 * @param[in] bIsCanceled �Ƿ�ȡ�����ӹ��̣������߿���ͨ����ָ���ж�ִ�й��̣�ΪNULL��ʾ������
	 * @param[in] pResult ���ӽ����ָ�룬�ɵ����ߴ���ָ�룬�ڲ��������
	 * @return �ɹ�����0��ʧ�ܷ�������ֵ
	 */
	int Connect(const NET_DEVICE_CONN_CONFIG* pConfig, int iTimeout, bool *bIsCanceled, NET_DEVICE_CONN_RESULT *pResult);

private:
	typedef struct _connect_thread_param_
	{
		const NET_DEVICE_CONN_CONFIG*	pConfig;
		int								iTimeout;
		bool*							bIsCanceled;
		NET_DEVICE_CONN_RESULT*			pResult;
		int								nError;
		bool							bFinished;
	}CONNECT_THREAD_PARAM;

	static PUB_THREAD_RESULT PUB_THREAD_CALL ConnectThread(void *pParam);

private:
	static int	GetDeviceInfo(const CManagerServer* pMS, const char* pDevSN, char* pDevIP, int& nDevPort);
};

#endif//__NET_DEVICE_CONNECTOR__
