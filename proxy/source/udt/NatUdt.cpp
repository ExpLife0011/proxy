// NatDevicePeer.cpp: implements for the CNatUdt class.
//
//////////////////////////////////////////////////////////////////////


#ifdef TEST_PROFILE
#include "Profile.h"
#endif

#include "NatCommon.h"
#include "NatUdt.h"
#include <assert.h>

#include "NatDebug.h"
bool g_bool=0;


bool CNatUdt::IsDatagramValid(const NAT_TRANSPORT_RECV_DATAGRAM* datagram)
{
	return datagram != NULL && datagram->dataSize >= sizeof(UDP_WRAPPER_HEAD) && datagram->dataSize <= MAX_WRAPPER_DATA_BUFFSIZE;
}

byte CNatUdt::GetDatagramCategory( const NAT_TRANSPORT_RECV_DATAGRAM* datagram )
{
    assert(datagram != NULL);
	if (IsDatagramValid(datagram))
	{
		return (reinterpret_cast<const UDP_WRAPPER_HEAD *>(datagram->data))->category;
	}
	return NAT_UDT_CATEGORY_INVALID;
}

uint32 CNatUdt::GetDatagramConnectionId(const NAT_TRANSPORT_RECV_DATAGRAM* datagram)
{
    assert(datagram != NULL);
	if (IsDatagramValid(datagram))
	{
		return (reinterpret_cast<const UDP_WRAPPER_HEAD *>(datagram->data))->connectionID;
	}
	return 0;
}

bool CNatUdt::IsDatagramSynCmd(const NAT_TRANSPORT_RECV_DATAGRAM* datagram)
{
    assert(datagram != NULL);

	if (IsDatagramValid(datagram))
	{
		return (reinterpret_cast<const UDP_WRAPPER_HEAD *>(datagram->data))->cmdId == UDP_DATA_TYPE_SYN;
	}
	return false;
}

CNatUdt::CNatUdt()
:
m_pSendDataManager(NULL),
m_pOwnedSendDataManager(NULL),
m_pRecvDataManager(NULL),
m_pOwnedRecvDataManager(NULL),
m_curTickCount(0)
{
	m_bStarted = false;
	m_UDP_TIMEOUT = UDP_TIMEOUT;
	m_UDP_MAX_SEND_INTERVAL = UDT_HEARTBEAT_INTERVAL;
}


CNatUdt::~CNatUdt()
{

}



uint32 CNatUdt::GetConnectionID()
{
	return m_RecordedInfo.initSendSeq;;
}

bool CNatUdt::Start(const NAT_UDT_CONFIG* config)
{
	if (m_bStarted)
	{
		return true;
	}
	m_bStarted = true;
	m_curTickCount = Nat_GetTickCount();

	m_Category = config->category;
	m_RemoteInfo.ip = config->remoteIp;
	m_RemoteInfo.port = config->remotePort;
	struct in_addr tmp_Addr;
	tmp_Addr.s_addr = config->remoteIp;
	memset(m_RemoteInfo.Ip, 0, sizeof(m_RemoteInfo.Ip));
	strcpy(m_RemoteInfo.Ip, inet_ntoa(tmp_Addr));
	m_pUdtNotifier = NULL;

    assert(m_pSendDataManager == NULL && m_pOwnedSendDataManager == NULL);
	if (NULL == config->pSendHeapDataManeger)
	{
		m_pOwnedSendDataManager = new CNatHeapDataManager(SEND_PACKET_BUF_SIZE, config->maxSendWindowSize);
		if (NULL == m_pOwnedSendDataManager)
		{
			NAT_LOG_WARN("CNatUdt create send data manager failed!\n");
			return false;
		}
		m_pSendDataManager = m_pOwnedSendDataManager;
	}
	else
	{
		m_pSendDataManager = config->pSendHeapDataManeger;
    }


    assert(m_pRecvDataManager == NULL && m_pOwnedRecvDataManager == NULL);
    if (NULL == config->pRecvHeapDataManeger)
    {
        m_pOwnedRecvDataManager = new CNatHeapDataManager(RECV_PACKET_BUF_SIZE, config->maxRecvWindowSize);
        if (NULL == m_pOwnedRecvDataManager)
        {
            NAT_LOG_ERROR("CNatUdt Create recv data manager failed!\n");
            return false;
        }
        m_pRecvDataManager = m_pOwnedRecvDataManager;
    }
    else
    {
        m_pRecvDataManager = config->pRecvHeapDataManeger;
    }
    m_dwMaxRecvReliableBuffCount = config->maxRecvWindowSize;

	m_RSendInfo.remainHeapPacketNum = MAX_SEND_BUFF_PACKET_COUNT;
	m_RSendInfo.remainedBuffSize = MAX_SEND_RELIABLE_BUFFSIZE;

	memset(&m_RecordedInfo, 0, sizeof(UDT_RECORDED_INFO));
	memset(&m_Statistic, 0, sizeof(UDT_STATISTIC));
	memset(&m_CtrlAlg, 0, sizeof(UDT_CTRL_ALG));

	m_LastAckTime = 0;
	m_bIsAutoRecv = false;

	//RTT RTO������صĳ�ʼ��
	m_sa = MIN_RTT_TIME << 3;
	m_sv = 0;
	m_dwRTT = MIN_RTT_TIME;
	m_dwRTO = 2 * m_dwRTT;	//���ֵֻӰ���һ�εĳ�ʱʱ��
	m_CtrlAlg.sendMode = SWL_UDP_SENDMODE_SLOW_STARTUP;

	m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NONE;

	m_iUsed = 0;
	m_iCwnd = 1;
	m_iCrowdedNumerator = 0;
	m_iSsthresh = 0x7fffffff;

	m_LastResendTime = 0;
	m_LastRemoveUnAckPackTime = 0;
	//����֮ǰ�ĳ�ʼ��
	m_RecordedInfo.initSendSeq = config->connectionID;
	m_RecordedInfo.initPeerSendSeq = 0;
	m_RecordedInfo.lastRecvTick = m_curTickCount;
	m_RecordedInfo.lastSendTick = 0;
	m_RecordedInfo.lastSendIndex = m_RecordedInfo.initSendSeq - 1;
	m_RecordedInfo.maxPeerRecvIndex = m_RecordedInfo.initSendSeq - 1;
	m_RecordedInfo.state = SWL_UDP_STATE_UNCONNECTED;
	m_RSendInfo.nextSeq = m_RecordedInfo.initSendSeq + 1;
	m_RRecvInfo.expectedSeq = 0;
	m_RRecvInfo.lastOnRecvPackPos = 0;
	m_RRecvInfo.maxPeerAck = 0;
	m_RRecvInfo.minSeq = 0;

	NAT_LOG_DEBUG("[category: %d Peer Ip:%s  port:%d  ConnectionID:%lu]  CNatUdt::Start\n", m_Category, m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
	return true;
}

void CNatUdt::Stop()
{
	if (!m_bStarted)
	{
		return ;
	}
	m_bStarted = false;

	//TODO ����RST�����������ϲ��ChangeDisconnected������RST������ȥ�������
	Disconnect();

#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	if (m_pOwnedSendDataManager != NULL)
	{
		delete m_pOwnedSendDataManager;
        m_pOwnedSendDataManager = NULL;
    }
    m_pSendDataManager = NULL;
	m_RSendInfo.listReliableSendData.clear();
	m_RSendInfo.listUnackedData.clear();
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif


#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif


	NatUdtRecvDataList::iterator it;
	for (it = m_RRecvInfo.recvDataList.begin(); it!=m_RRecvInfo.recvDataList.end(); ++it)
	{
		DestroyRecvData((*it).second);
	}
	m_RRecvInfo.recvDataList.clear();

    if (m_pOwnedRecvDataManager != NULL)
    {
        delete m_pOwnedRecvDataManager;
        m_pOwnedRecvDataManager = NULL;
    }
    m_pRecvDataManager = NULL;
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif

#ifdef TEST_PROFILE
	PrintProfileResult();
	NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  Real Resend Times %lu\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID(),  m_Statistic.reSendPacketNum);
#endif
	NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  NatStop\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
}

NatRunResult CNatUdt:: Run()
{
#ifdef TEST_PROFILE
static FunctionProfile __funcProfile(__FUNCTION__);
ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	if (!m_bStarted)
	{
		return RUN_NONE;
	}

	m_curTickCount = Nat_GetTickCount();
	NatRunResult ret = RUN_NONE;
	int	iRet = 0;
	bool bSendAck = false;
	do 
	{
		if(-1 == CheckConnect(m_curTickCount))
		{		
			ret = RUN_DO_FAILED;
			break;
		}

#ifndef UDT_WITHOUT_LOCK
		m_RSendInfoLock.Lock();
#endif
		if (((int)m_RSendInfo.listUnackedData.size() > m_iUsed) || PUB_UNSIGNED_LONG_GT(m_curTickCount - 5, m_LastResendTime))
		{
			m_LastResendTime = m_curTickCount;
			do 
			{
				iRet = ResendData(m_curTickCount);
			} while(iRet == 1);
		}
#ifndef UDT_WITHOUT_LOCK
		m_RSendInfoLock.UnLock();
#endif
		if ((-4 == iRet) || (-6 == iRet))
		{
			//��������⼸���������Ͳ��÷�����������
			//����ɶ��������������ȥ����ȷ������

			//ret = 
		}
		else if ((-2 == iRet) || (-5 == iRet))
		{
			//��������⼸���������Ͳ��÷�����������
			//Ҳ���÷�ȷ����
			
			//ret = 
			break;
		}
		else if (-1 == iRet)
		{
			ret = RUN_DO_FAILED;
			break;
		}
		else if(0 == iRet)
		{
			//û�������ش��ͷ����µ�����
			do 
			{
				iRet = SendReliableData(m_curTickCount);
				if (-1 == iRet)
				{
					ret = RUN_DO_FAILED;
					break;
				}
				else if(1 == iRet)
				{
					
				}
				else
				{

				}
				ret = RUN_NONE;
			} while(iRet == 1);
		}

		//������� ���� ȷ��ʱ�䳬ʱ����ظ�ȷ��	
		if(PUB_IsTimeOutEx(m_LastAckTime, SEND_ACK_INTERVAL, m_curTickCount))
		{
			//���SEND_ACK_INTERVAL����û�з��͹�һ��ack�������Ƿ�����Ҫack
			//�����ݣ��������Ҫack����ظ�ack

			//�����Ƿ�����Ҫack�����ݶ����¸�dwAckTimer��ֵ�����ζ����¸�ֵ
			//��Ϊ���
			//��1��û��������Ҫack����ô�ٹ�SEND_ACK_INTERVAL���ټ���Ƿ���Ҫack��
			//��2�������������Ҫack������m_RecordedInfo.ackStatus��״̬��ΪSWL_UDP_ACK_STATUS_NONE���ͻᷢ��ack			
			m_LastAckTime = m_curTickCount;

			//�ж��Ƿ�����Ҫack������
			if (SWL_UDP_ACK_STATUS_NONE != m_RecordedInfo.ackStatus)
			{				
				bSendAck = true;
			}
		}
		else
		{
			if((SWL_UDP_ACK_STATUS_NOW == m_RecordedInfo.ackStatus) 
				|| (SWL_UDP_ACK_STATUS_DISORDER == m_RecordedInfo.ackStatus))
			{
				//����ack�ˣ��´�����SEND_ACK_INTERVAL����û��ack�ټ���Ƿ�����Ҫack������
				m_LastAckTime = m_curTickCount;
				bSendAck = true;
			}
		}

		//�ظ�ȷ��
		if (bSendAck)
		{
			iRet = SendAckData(m_curTickCount);
			if(iRet == -1)
			{				
				ret = RUN_DO_FAILED;
			}
			else if(iRet == -2)
			{
				ret = RUN_NONE;
			}
			else
			{
				ret = RUN_DO_MORE;
			}
		}

	} while(0);
	
	if (ret == RUN_DO_FAILED)
	{
		NAT_LOG_DEBUG("CNatUdt Run failed!\n");
		Disconnect();
	}

	return ret; 
}

void CNatUdt::SetNotifier(CUdtNotifier *pUDTInterface)
{
	m_pUdtNotifier = pUDTInterface;
}

int CNatUdt::RecvPacket(const UDP_WRAPPER_DATA *packet, int packetSize)
{
	if (!m_bStarted)
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  OnRecvPacke is invalid(udt not start)\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		return -1;
	}

	if (!packet)
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  CNatUdt::OnRecvPacke\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		return -1;
	}

#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	assert(packetSize <= MAX_WRAPPER_DATA_BUFFSIZE);

	if (m_RecordedInfo.state != SWL_UDP_STATE_CONNECTED && packet->head.cmdId != UDP_DATA_TYPE_SYN)
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  cmdId err:%d\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID(), packet->head.cmdId);
		return 0;
	}

	assert(packet->head.connectionID == GetConnectionID());

	int ret = 0;
	uint32 currTick = m_curTickCount;
	switch(packet->head.cmdId)
	{
	case UDP_DATA_TYPE_SYN:
		ret = HandleRecvConnect(packet, packetSize, currTick);
		break;

	case UDP_DATA_TYPE_RELIABLE:
		m_Statistic.totalRecvPacketNum++;
		m_Statistic.totalRecvBytes += packetSize;
		ret = HandleRecvReliableData(packet, packetSize, currTick);
		m_RecordedInfo.lastRecvTick = currTick;
		break;

	case UDP_DATA_TYPE_ACK:
		ret = HandleRecvAckData(packet, packetSize, currTick);
		m_RecordedInfo.lastRecvTick = currTick;
		break;

	case UDP_DATA_TYPE_RST:
		HandleRecvRST(packet, packetSize, currTick);
		ret = -1;
		break;
	default:
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  cmdId err:%d\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID(), packet->head.cmdId);
		ret = -1;
		break;
	}

	if (ret == -1)
	{		
		Disconnect();
	}

	return ret;
}

int CNatUdt::Send(const void *pData, int iLen)
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	UDP_SEND_PACKET_DATA	*pPacket = NULL;
	int totalLen = iLen;

	if (m_RecordedInfo.state != SWL_UDP_STATE_CONNECTED)
	{
		return -1;
	}
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	if(m_RSendInfo.remainedBuffSize < iLen || m_RSendInfo.remainHeapPacketNum < 1)
	{
#ifndef UDT_WITHOUT_LOCK
		m_RSendInfoLock.UnLock();
#endif
		return 0;
	}
	
	while(iLen > 0)
	{
		pPacket = static_cast<UDP_SEND_PACKET_DATA*>(m_pSendDataManager->GetMemory());
		if (!pPacket)
		{
            //NAT_LOG_DEBUG("NatUdt Warning: Send Data failed! Send Heap Data is used up!\n");
			break;
		}
		m_RSendInfo.remainHeapPacketNum--;
		pPacket->wrapperData.head.seqNum = m_RSendInfo.nextSeq++;
#ifndef UDT_WITHOUT_LOCK
		m_RSendInfoLock.UnLock();
#endif

		pPacket->wrapperData.head.category = m_Category;
		pPacket->wrapperData.head.cmdId = UDP_DATA_TYPE_RELIABLE;
		pPacket->wrapperData.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;		
		

		pPacket->lastSendTick = 0;
		pPacket->dataStatus = SWL_UDP_DATA_STATUS_NOSEND;	
		pPacket->iLen = (iLen > static_cast<int32>(WRAPPER_VALIDATA_LEN)) ? WRAPPER_VALIDATA_LEN : iLen;
		iLen -= pPacket->iLen;
		memcpy(&(pPacket->wrapperData.wrapper_dat), pData, pPacket->iLen);
		pData = (const char*)pData + pPacket->iLen;		
#ifndef UDT_WITHOUT_LOCK
		m_RSendInfoLock.Lock();
#endif
		m_RSendInfo.listReliableSendData.push_back(pPacket);	
	}

	m_RSendInfo.remainedBuffSize -= (totalLen - iLen);
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif
	return totalLen - iLen;
}

int CNatUdt::CheckConnect( uint32 currentTick )
{
	if (PUB_IsTimeOutEx(m_RecordedInfo.lastRecvTick, m_UDP_TIMEOUT, currentTick))
	{	
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  UDP_TIMEOUT\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());        
		return -1;
	}

	if(SWL_UDP_STATE_CONNECTED != m_RecordedInfo.state)
	{		
		if ((0 == m_RecordedInfo.lastSendTick) 
			|| PUB_IsTimeOutEx(m_RecordedInfo.lastSendTick, UDP_CONNECT_INTERVAL, currentTick))
		{
			UDP_SYN_PACKET synPacket;
			synPacket.head.category = m_Category;
			synPacket.head.cmdId = UDP_DATA_TYPE_SYN;
			synPacket.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;
			synPacket.head.seqNum = m_RecordedInfo.initSendSeq;
			synPacket.head.connectionID = GetConnectionID();
			synPacket.data.protocolTag = NAT_UDT_PROTOCOL_TAG;
			//synPacket.data.version

			if(SWL_UDP_STATE_RECVED == m_RecordedInfo.state)
			{
				synPacket.head.ackNum = m_RecordedInfo.initPeerSendSeq;
			}
			else if(SWL_UDP_STATE_UNCONNECTED == m_RecordedInfo.state)
			{
				synPacket.head.ackNum = 0;
			}
			else
			{
				assert(false);
				return -1;
			}
			//�������һ�η���ʱ��
			m_RecordedInfo.lastSendTick = currentTick;
			OnNotifySendPacket(&synPacket, sizeof(UDP_SYN_PACKET));
		}
	}
	else
	{
		if (PUB_IsTimeOutEx(m_RecordedInfo.lastSendTick, m_UDP_MAX_SEND_INTERVAL, currentTick))
		{
			SendAckData(currentTick);		
		}	
	}

	return 0;
}

int CNatUdt::SendAckData( uint32 currentTick )
{
	int ret = -1;

	//m_SendTmpData.head.seqNum = m_RSendInfo.nextSeq - 1;	//liujiang ���ֵ�Է�û����
	m_SendTmpData.head.category = m_Category;
	m_SendTmpData.head.cmdId = UDP_DATA_TYPE_ACK;
	m_SendTmpData.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;
	m_SendTmpData.head.sendIndex = m_RecordedInfo.lastSendIndex;
	m_SendTmpData.head.recvIndex = m_RecordedInfo.maxRecvIndex;
	m_SendTmpData.head.ackNum = m_RRecvInfo.expectedSeq - 1;
	m_SendTmpData.head.connectionID = GetConnectionID();		

	std::list<uint32>::iterator it;			
	uint32 i = 0;	
	//�����ֻ�ظ�һ��
	for (i = 0, it = m_listDisorderlyPacket.begin(); 
		it != m_listDisorderlyPacket.end() && (i < MAX_ACK_COUNT);
		it = m_listDisorderlyPacket.begin(), ++i)
	{		
		m_SendTmpData.wrapper_dat.acks[i].ackSeq = (*it);		
		m_listDisorderlyPacket.pop_front();
	}

	int iLen = sizeof(UDP_WRAPPER_HEAD) + sizeof(uint32) * i;


	ret = OnNotifySendPacket(&m_SendTmpData, iLen);
	if (0 == ret)
	{
		//�������һ�η���ʱ��
		m_RecordedInfo.lastSendTick = currentTick;
		m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NONE;
	}
	else if(-1 == ret)
	{

	}
	else//socket blocked
	{
		ret = -2;
	}
	

	return ret;
}

int CNatUdt::ReleaseReliableData( uint32 seq, uint32 index, uint32 currentTick )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	UDP_SEND_PACKET_DATA	*pPacket = NULL;
	std::list<UDP_SEND_PACKET_DATA*>::iterator it;
	bool	bCompareIndex = false;
	bool	bDeleteData = false;
	uint32	lastIndex = 0;

	
	//���¶Է��յ����ط��ͳ�������ʶ
	if (PUB_UNSIGNED_LONG_GT(index, m_RecordedInfo.maxPeerRecvIndex))
	{		
		lastIndex = m_RecordedInfo.maxPeerRecvIndex;
		m_RecordedInfo.maxPeerRecvIndex = index;		
		bCompareIndex = true;

		if(m_CtrlAlg.bResendRestrict)
		{
			if(PUB_UNSIGNED_LONG_GEQ(index, m_CtrlAlg.nextResendIndex))
			{
				m_CtrlAlg.bResendRestrict = false;
			}
		}

		//ֻҪ֪���Է����յ������Ͱ��ش�������0
		m_CtrlAlg.resendTime = 0;	

		//���� RTT �� RTO
#ifdef	RTT_RESOLUTION_1
		//RTT����һ��
		if (m_CtrlAlg.bCalculateRtt)
		{
			if (PUB_UNSIGNED_LONG_GEQ(m_RecordedInfo.maxPeerRecvIndex, m_CtrlAlg.rttHelp.sendIndex))
			{
				m_CtrlAlg.bCalculateRtt = false;
				CalculateRTO(currentTick, m_CtrlAlg.rttHelp.sendTick);
			}
		}
#else
		//RTT��������
		uint32 dwStartTick = 0;
		uint32 sendIndex = 0;		
		std::list<UDP_RTT_HELP>::iterator it;
		//��3516�İ����ϣ�list��size������list������н��Խ�࣬Ч��Խ��,��ǰ������ѭ���ж�size��С
		//���ڸĵ���
		for(it = m_listRttHelp.begin(); it != m_listRttHelp.end(); it = m_listRttHelp.begin())
		{
			sendIndex = (*it).sendIndex;
			int iDis = static_cast<int>(sendIndex - m_dwIndexPeerRecv);
			if (0 > iDis)
			{
				m_listRttHelp.pop_front();
			}
			else if(0 == iDis)
			{
				dwStartTick = (m_listRttHelp.front()).sendTick;
				m_listRttHelp.pop_front();
				break;
			}
			else
			{
				assert(false);
			}			
		}
		CalculateRTO(currentTick, dwStartTick);
#endif			
	}

	//���¶Է�ȷ�ϵ�������ݣ�ɾ������δȷ�ϵķ�������
	if(PUB_UNSIGNED_LONG_GT(seq, m_RRecvInfo.maxPeerAck))
	{
		//��ȷ���µ�����

		bDeleteData = true;
		m_RRecvInfo.maxPeerAck = seq;

		//���µ�˳�����ݱ�ȷ���ˣ����һ�����ش��Ĵ�������
		m_CtrlAlg.firstResend = 0;

		//�ش���������¼�����ش��ı����������л�״̬
		if (SWL_UDP_SENDMODE_TIMEOUT_RESEND == m_CtrlAlg.sendMode)
		{			
			m_CtrlAlg.bResendRestrict = true;
			m_CtrlAlg.nextResendIndex = m_RecordedInfo.lastSendIndex;
			ChangeMode(SWL_UDP_SENDMODE_SLOW_STARTUP);
		}
		else if(SWL_UDP_SENDMODE_FAST_RESEND == m_CtrlAlg.sendMode)
		{			
			m_CtrlAlg.bResendRestrict = true;
			m_CtrlAlg.nextResendIndex = m_RecordedInfo.lastSendIndex;
			ChangeMode(SWL_UDP_SENDMODE_CROWDED_AVOID);
		}
	}
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	for (it = m_RSendInfo.listUnackedData.begin(); it != m_RSendInfo.listUnackedData.end(); ) 
	{	
		pPacket = *it;

		//�ҵ�ÿ���ظ���Ӧ�ķ������ݣ��ͷ���Դ
		if (bDeleteData)
		{
			if(PUB_UNSIGNED_LONG_GEQ(seq, pPacket->wrapperData.head.seqNum))
			{				
				//�����SWL_UDP_DATA_STATUS_BEYOND��SWL_UDP_DATA_STATUS_TIMEOUT�ģ�����ǰ�����û�������
				if (pPacket->dataStatus == SWL_UDP_DATA_STATUS_SENDED)
				{										
					IncreaseCwnd(1, false);
				}

				m_RSendInfo.remainedBuffSize += pPacket->iLen;					
				m_pSendDataManager->ReleaseMemory(pPacket);
				m_RSendInfo.remainHeapPacketNum++;
				it = m_RSendInfo.listUnackedData.erase(it);	//bug ��������ҪЧ�ʵ͵ĵط�			
				continue;
			}
			else
			{
				//��������ҵ������ݵ�����Ѿ��Ƚ��յĸ�����ô����Ŀ϶���������
				bDeleteData = false;
			}
		}

		if (bCompareIndex)
		{
			//���ڵĲ����жϣ������ɾ��   ��ǰ��û�д�����ı�ʶ
			if (PUB_UNSIGNED_LONG_GT(index, pPacket->lastSendIndex))
			{					
				assert(pPacket->lastSendIndex != index);

				//[dwLastIndex + 1, dwIndexPeerRecv) �������ı�ʶ����Ҫ�����
				//dwIndexPeerRecv�İ��ᱻɾ�������ﲻ������������ˣ����ھͲ�������
				//�������˵İ����������Ӵ��ڴ�С

				//û������SWL_UDP_DATA_STATUS_TIMEOUT��SWL_UDP_DATA_STATUS_BEYOND
				//����ı�ʶ�Է����յ��ˣ�����SWL_UDP_DATA_STATUS_BEYOND�������жϿ����ش�
				if (SWL_UDP_DATA_STATUS_SENDED == pPacket->dataStatus)
				{
					pPacket->dataStatus |= SWL_UDP_DATA_STATUS_BEYOND;
					IncreaseCwnd(1, true);
				}					
			}
			else
			{
				// >=dwIndexPeerRecv 
				// > dwIndexPeerRecv �Ĳ������ݶԷ���û���յ�����ʱ�Ȳ�����
				// = �İ�Ҫô��ǰ���һ��ɾ���Ĵ�����ɾ���ˣ�Ҫô�������������ʱ���ɾ��

				//fixed by cc  ��ΪlistUnackedData�е�packet����lastSendIndex�ǵ�����
				bCompareIndex = false;
			}			

  			if (!bDeleteData && !bCompareIndex)
  			{
  				break;
  			}
			
		}		
		++it;
	}
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif
	return 0;
}

uint32 CNatUdt::CalculateRTO( uint32 currentTick, uint32 startTick )
{
/* diff between this shouldn't exceed 32K since this are tcp timer ticks
	and a round-trip shouldn't be that long... */ 
	uint32 dwLastRTO = m_dwRTO;		 
	m_dwRTT = currentTick - startTick;
	short max = static_cast<short>(m_dwRTT);
	max = MIN_RTT_TIME > max ? MIN_RTT_TIME : max;
	short m = max;
		
	/* This is taken directly from VJs original code in his paper */
	m = m - (m_sa >> 3);
	m_sa += m;
	if (m < 0)
	{
		m = -m;
	}
	m = m - (m_sv >> 2);
	m_sv += m;
	m_dwRTO = static_cast<uint32>((m_sa >> 3) + m_sv);

#ifdef __ENVIRONMENT_LINUX__
	if(2 == g_bool)
	{
		NAT_LOG_DEBUG("dwRTT = %lu dwRTO = %lu m_sa = %hu m_sv = %hu\n", m_dwRTT, m_dwRTO,m_sa,m_sv);
	}
#endif


/*modified by cc  ���漸�����ж�[jacobson 1988/1990]���޸ģ��ᵼ�����������
**��Ȼ����RTO�ܿ��ٷ�Ӧ��RTT��2�������ǻ�Ӱ����һ��RTO�ļ��㣬
**�����һ��RTO�Ƚϴ��Ǽ��������һ��RTOֵ���ܱ���һ�ε�RTTֵ��С,�������ش�����
	//�����޸�����ʵ��rtt�Ѿ���С��������ǰrtt�Ƚϴ��ʱ�򣬵���rto�Ƚϴ�
	//��ʱ����rto�仯̫���������������⣬
	//��������RTO������RTT������
	if (m_dwRTO > static_cast<uint32>(max << 1))
	{
		m_dwRTO = (max << 1);
	}	

	//��������RTO�������ϴ�RTO������
	if(m_dwRTO > (dwLastRTO<< 1))
	{
		m_dwRTO = (dwLastRTO<< 1);
	}
*/

//	if((m_dwRTO<200) || (m_dwRTO>400))
//	{
//		printf("dwRTT = %lu dwRTO = %lu m_sa = %hu m_sv = %hu\n", m_dwRTT, m_dwRTO,m_sa,m_sv);
//	}
	assert(m_dwRTO != 0);
	return m_dwRTO;
}

int CNatUdt::ChangeMode( byte ucMode )
{
	//printf("curMode :%d    Changeto :%d   Cwnd :%d   SSthresh :%d\n", m_CtrlAlg.sendMode, ucMode, m_iCwnd, m_iSsthresh);

	if (SWL_UDP_SENDMODE_TIMEOUT_RESEND == ucMode)
	{		
		if(SWL_UDP_SENDMODE_FAST_RESEND == m_CtrlAlg.sendMode)//���ڿ��ٻָ�ʱ��Ҫ�л���������
		{				
			//m_iSsthresh ���䣬��Ϊ������ٻָ���ʱ���Ѿ����һ����
			m_iCwnd = (m_iSsthresh << 2) / 5;
		}
		else if((SWL_UDP_SENDMODE_CROWDED_AVOID == m_CtrlAlg.sendMode) || (SWL_UDP_SENDMODE_SLOW_STARTUP == m_CtrlAlg.sendMode))
		{			
			m_iSsthresh = m_iCwnd;			
			m_iCwnd = (m_iCwnd << 2) / 5;	// 4/5
		}
		else if(SWL_UDP_SENDMODE_TIMEOUT_RESEND == m_CtrlAlg.sendMode)
		{			
			m_iSsthresh = m_iCwnd;			
			m_iCwnd = (m_iCwnd << 2) / 5;	// 4/5						
		}

		if (m_iSsthresh < MIN_SSTHRESH)
		{
			m_iSsthresh = MIN_SSTHRESH;
		}

		if(m_iCwnd < MIN_CWND_WIN)
		{
			m_iCwnd = MIN_CWND_WIN;
		}

		m_CtrlAlg.sendMode = SWL_UDP_SENDMODE_TIMEOUT_RESEND;
	}
	//ת��������״̬
	else if (SWL_UDP_SENDMODE_SLOW_STARTUP == ucMode)
	{
		//Ŀǰֻ��һ�ַ���ģʽ�����л�����
		assert(SWL_UDP_SENDMODE_TIMEOUT_RESEND == m_CtrlAlg.sendMode);
		m_CtrlAlg.sendMode = SWL_UDP_SENDMODE_SLOW_STARTUP;
	}
	//ת��ӵ������״̬
	else if(SWL_UDP_SENDMODE_CROWDED_AVOID == ucMode)
	{
		if (SWL_UDP_SENDMODE_SLOW_STARTUP == m_CtrlAlg.sendMode)
		{
			//ʲô������
		}		
		else if(SWL_UDP_SENDMODE_FAST_RESEND == m_CtrlAlg.sendMode)
		{			
			//m_iSsthresh ���䣬��Ϊ������ٻָ���ʱ���Ѿ����һ����
			m_iCwnd = m_iSsthresh;
		}
		else
		{
			//�����������ӵ�������״̬�������ֵ��ý���ӵ������״̬��
			//��ô�߼��Ͽ϶�������
			//����ǳ�ʱ�ش�Ҳ�������л���ӵ������״̬
			assert(false);
		}
		m_iCrowdedNumerator = 0;
		m_CtrlAlg.sendMode = SWL_UDP_SENDMODE_CROWDED_AVOID;		
	}
	//ת�����ٻָ�״̬
	else if(SWL_UDP_SENDMODE_FAST_RESEND == ucMode)
	{
		assert(m_CtrlAlg.sendMode != SWL_UDP_SENDMODE_FAST_RESEND);

		//��һ�л��������Ϊ��device�˶Ͽ����Ӻ�NATServer�˻�û�Ͽ���NATServer�˷����İ�ûӦ��ģʽ�л���SWL_UDP_SENDMODE_TIMEOUT_RESEND
		//����ʱdevice�����������ӣ�device�����ݵ�SendIndex������GetTickCount��������ģ��϶����֮ǰ�Ĵ��⵼��unAckList�е�packet�����Ϊ��SWL_UDP_DATA_STATUS_BEYOND,
		//����RecvPacket�������ӻ�û����ʱ��packet���ԣ�������ָö�����
//		assert(m_CtrlAlg.sendMode != SWL_UDP_SENDMODE_TIMEOUT_RESEND);

		m_iSsthresh = ((m_iCwnd << 3) - m_iCwnd) >> 3;		// 7/8
		if (m_iSsthresh < MIN_SSTHRESH)
		{
			m_iSsthresh = MIN_SSTHRESH;
		}		
		m_CtrlAlg.sendMode = SWL_UDP_SENDMODE_FAST_RESEND;
	}
	else
	{
		assert(false);
	}
	return 0;
}

int CNatUdt::IncreaseCwnd( int iCount /*= 1*/, bool bResend /*= false*/ )
{
	m_iUsed -= iCount;
	assert(m_iUsed >= 0);

	if (!bResend)
	{
		for (int i = 0; i < iCount; ++i)
		{
			//ӵ������
			if(SWL_UDP_SENDMODE_CROWDED_AVOID == m_CtrlAlg.sendMode)
			{
				//ÿ��Ҫ�յ� ӵ���������� �����Ÿ�ӵ�����ڼ�1
				++m_iCrowdedNumerator;
				if (m_iCrowdedNumerator >= m_iCwnd)
				{
					++m_iCwnd;
					m_iCrowdedNumerator = 0;
				}		
			}
			//��ʱ�ش�
			else if(SWL_UDP_SENDMODE_TIMEOUT_RESEND == m_CtrlAlg.sendMode) 
			{
				//��ʱ�ش�ʱ��ÿ�յ�һ������ֻ���ٷ�һ������������������һ��
				//ÿ��һ������������
			}
			//���ٻָ�
			else if(SWL_UDP_SENDMODE_FAST_RESEND == m_CtrlAlg.sendMode)
			{
				//���ٻָ�ʱ��ÿ�յ�һ������ֻ���ٷ�һ������������������һ��
				//ÿ��һ������������
			}
			else//������
			{				
				++m_iCwnd;
				if (m_iCwnd > m_iSsthresh)
				{
					ChangeMode(SWL_UDP_SENDMODE_CROWDED_AVOID);
				}								
			}		
		}	
	}

	return 0;
}

int CNatUdt::AdjustRecvPacket()
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif
	NatUdtRecvDataList::iterator it;
	for(it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.expectedSeq);
		it != m_RRecvInfo.recvDataList.end();
		it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.expectedSeq))
	{		
		++(m_RRecvInfo.expectedSeq);
	}
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif
	//ɾ��û������İ����
	std::list<uint32>::iterator itex;	
	for (itex = m_listDisorderlyPacket.begin(); itex != m_listDisorderlyPacket.end();)
	{	
		//�������е��ڵİ�
		assert( (*itex) != m_RRecvInfo.expectedSeq);
		if (PUB_UNSIGNED_LONG_LT((*itex), m_RRecvInfo.expectedSeq))
		{
			itex = m_listDisorderlyPacket.erase(itex);
		}
		else
		{
			++itex;
		}
	}

	return 0;
}


int CNatUdt::OnNotifySendPacket( const void* pBuf, int iLen )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	int ret = -1;
	if (m_pUdtNotifier)
	{
		ret = m_pUdtNotifier->OnSendPacket(this, pBuf, iLen);
	}
	else
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  UdtNotifier Err\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
	}

	if (0 == ret)
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  UdtNotifier Blocked\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		ret = 1;
	}
	else if(iLen == ret)
	{
		ret = 0;
	}
	else
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  sock err\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		ret = -1;
	}

	return ret;
}

int CNatUdt::OnNotifyRecvPacket( const void* pBuf, int iLen )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	int ret = -1;
	if (m_pUdtNotifier)
	{
		ret = m_pUdtNotifier->OnRecv(this, pBuf, iLen);
	}

	return ret;
}

void CNatUdt::OnNotifyConnect( int iErrorCode )
{
	NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  OnNotifyConnect : %d\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID(), iErrorCode);
	if (m_pUdtNotifier)
	{
		m_pUdtNotifier->OnConnect(this, iErrorCode);
	}
}

int CNatUdt::HandleRecvReliableData(const UDP_WRAPPER_DATA *pWrapperData, int iLen, uint32 currentTick )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	/********************************** ���ݶԷ���ȷ�ϣ���ʶ������ش��� **************************************************/
	ReleaseReliableData(pWrapperData->head.ackNum, pWrapperData->head.recvIndex, currentTick);

	/********************************** ���ݶԷ��ķ�����Ž�����ش��� ****************************************************/
	// �����յ��Է����ݵ�����ʶ
	if (PUB_UNSIGNED_LONG_GT(pWrapperData->head.sendIndex, m_RecordedInfo.maxRecvIndex))
	{
		m_RecordedInfo.maxRecvIndex = pWrapperData->head.sendIndex;

		// ��ʶ����ҲҪ����ȷ�ϣ�������������ϸ���жϣ��򵥵Ĵ���һ��
		// û���жϱ�ʶ����ĸ��ֿ�����������磺��ʶ��󣬵���û���յ������ݣ��п����ǶԷ��ش�������յ���������
		if (SWL_UDP_ACK_STATUS_NONE == m_RecordedInfo.ackStatus)
		{
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_DELAY;
		}
		else if(SWL_UDP_ACK_STATUS_DELAY == m_RecordedInfo.ackStatus)//�չ������µ����ݣ�����Ҫ��������ȷ��
		{
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NOW;
		}
		else
		{
			// �������� SWL_UDP_ACK_STATUS_NOW�������Ѿ��Ǹ��߼���� SWL_UDP_ACK_STATUS_DISORDER ���Ͳ��������� 
		}
	}

	//ע�⣬����iDisSeq�������з��ŵ����֣����ҷ��������������Ҳ����������
    int	iDisSeq = pWrapperData->head.seqNum - m_RRecvInfo.expectedSeq;
    int iBufDisSeq = pWrapperData->head.seqNum - m_RRecvInfo.minSeq;
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif
    if(iBufDisSeq > (static_cast<int>(m_dwMaxRecvReliableBuffCount) - 1))	// ������������С�İ�
    {
        // NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  buffer over flow\n", m_RemoteInfo.Ip, m_RemoteInfo.port, m_RecordedInfo.initPeerSendSeq);
        // NAT_LOG_DEBUG("map size:%lu\n", m_RRecvInfo.mapReliableRecvData.size());
        // BUG����������������һ�·��ͣ���Ҫ����������������Ժ�汾���������շ�˫���ֿ��������ȡ���������
        // ���˶��䶼Ҫ��������������ӵ��������α仯
        // ����������յ�������֮ǰ�����ܻ�������û�б�Ӧ�ó������ߣ����Ӧ�ó����ϲ��գ���ô���ܾͻ��ڴ�ľ�
        //		assert(false);
#ifndef UDT_WITHOUT_LOCK
        m_RecvLock.UnLock();
#endif
        return 0;
    }
	//������������ڵȵİ�
	else if (0 == iDisSeq)
	{
		//������ǰ�յ�����������ӷ�����-1����ҲҪ���������־����Ϊ���ܶԷ�û���յ�ȷ�ϣ����ط�	
		if (SWL_UDP_ACK_STATUS_NONE == m_RecordedInfo.ackStatus)
        {
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_DELAY;
		}
		else if(SWL_UDP_ACK_STATUS_DELAY == m_RecordedInfo.ackStatus) //�չ������µ����ݣ�����Ҫ��������ȷ��
		{
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NOW;
		}
		else
		{
			//��������SWL_UDP_ACK_STATUS_NOW�������Ѿ��Ǹ��߼����SWL_UDP_ACK_STATUS_DISORDER���Ͳ���������
		}

		// ��������ŵİ���ǰ�Ѿ��յ��ˣ��ͻ����ʧ��        
        if(!InsertRecvData(pWrapperData->head.seqNum, pWrapperData, iLen))
		{		
#ifndef UDT_WITHOUT_LOCK
			m_RecvLock.UnLock();
#endif
			return -1;
		}
	}
	else if(iDisSeq < 0)//��ʱ��
	{
		//printf("UDT recv overtime packet\n");

		//�����ͻظ�ȷ�ϵļ���ͺ���
		if (SWL_UDP_ACK_STATUS_DISORDER != m_RecordedInfo.ackStatus)
		{
			//�յ���ʱ���������ǶԷ�û���յ�ȷ�ϣ���������ط�ȷ��
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NOW;
		}
#ifndef UDT_WITHOUT_LOCK
		m_RecvLock.UnLock();
#endif
		return 0;
	}
	else //����� �����else�͵���if(iDisSeq > 0 && iDisSeq <= m_dwMaxRecvReliableBuffCount - (m_RRecvInfo.expectedSeq - m_RRecvInfo.minSeq) + 1)
	{
		//������ǰ�յ�����������ӷ�����-1����ҲҪ���������־����Ϊ���ܶԷ�û���յ�ȷ�ϣ����ط�		
		m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_DISORDER;

		//��ǰ��ɾ�����ظ���������ţ���������������ŷ�����ǰ�棬���ڲ�ȥ�أ�ֱ�ӷ���������棬ֻ����ɿ��ܶ෢ack
		m_listDisorderlyPacket.push_back(pWrapperData->head.seqNum);

        //��������ŵİ���ǰ�Ѿ��յ��ˣ��ͻ����ʧ��
        InsertRecvData(pWrapperData->head.seqNum, pWrapperData, iLen);

#ifndef UDT_WITHOUT_LOCK
		m_RecvLock.UnLock();
#endif
		return 0;
	}

	//�ߵ������ˣ��϶����յ�˳��İ���

	//�����������յ�����һ��������ţ�ɾ��û������İ����
	++m_RRecvInfo.expectedSeq;
	uint32 dwNextExpected = m_RRecvInfo.expectedSeq;

	AdjustRecvPacket();

	//����յ������ݻ��ѻ����е�����������������һ��ȷ���ˣ�����Ҫ��������ACK
	if (PUB_UNSIGNED_LONG_GT(m_RRecvInfo.expectedSeq, dwNextExpected))
	{
		if (SWL_UDP_ACK_STATUS_DISORDER != m_RecordedInfo.ackStatus)
		{
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NOW;
		}
	}


	if (m_bIsAutoRecv)
	{
		AutoNotifyRecvPacket();
	}
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif
	return 0;
}

int CNatUdt::HandleRecvAckData( const UDP_WRAPPER_DATA *pData, int iLen, uint32 currentTick )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	UDP_SEND_PACKET_DATA	*pPacket = NULL;
	std::list<UDP_SEND_PACKET_DATA*>::iterator it;

	assert( 0 == ((iLen - sizeof(UDP_WRAPPER_HEAD)) % sizeof(uint32)));

	ReleaseReliableData(pData->head.ackNum, pData->head.recvIndex, currentTick);

	int i = 0;
	int iFoundCount = 0;

#ifdef __ENVIRONMENT_LINUX__
	if (g_bool == 2)
	{
		int iLenex = iLen;
		//printf("dwRecvSeq = %lu, dwRecvIndex = %lu, currentTick = %lu\n",pData->head.dwRecvSeq, pData->head.dwRecvIndex, //currentTick);
		//for (i = 0, iLenex -= sizeof(UDP_WRAPPER_HEAD); iLenex > 0; iLenex -= sizeof(UDP_ACK), ++i)
		//{
		//	printf("%lu ", pData->wrapper_dat.acks[i].ackSeq);
		//}
		//printf("i = %d iLen = %d\n",i,iLen);
	}
#endif
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	//bug �����ѭ����erase���ᵼ��Ч�ʵͣ���Ҫ��erase
	for (i = 0, iLen -= sizeof(UDP_WRAPPER_HEAD); iLen > 0; iLen -= sizeof(UDP_ACK), ++i)
	{			
		//�ҵ�ÿ���ظ���Ӧ�ķ������ݣ��ͷ���Դ

		//����unacked����
		for(it = m_RSendInfo.listUnackedData.begin(); it != m_RSendInfo.listUnackedData.end();)
		{			
			pPacket = *it;				

			int iDisSeq = pData->wrapper_dat.acks[i].ackSeq - pPacket->wrapperData.head.seqNum;
			if (0 == iDisSeq)
			{	
				if (pPacket->dataStatus == SWL_UDP_DATA_STATUS_SENDED)
				{					
					++iFoundCount;
				}				

				m_RSendInfo.remainedBuffSize += pPacket->iLen;
				m_pSendDataManager->ReleaseMemory(pPacket);
				m_RSendInfo.remainHeapPacketNum++;
				it = m_RSendInfo.listUnackedData.erase(it);
				break;
			}
			else if(0 > iDisSeq)
			{		
				break;
			}
			else //(0 < iDisSeq)
			{				
				//�����Ƚ���һ����������
				++it;
			}				
		}	
	}
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif
	//֮���԰����󴰿ڷ��ں��棬����ΪҪ���ж��Ƿ��л������ٻָ�
	if (0 != iFoundCount)
	{
		IncreaseCwnd(iFoundCount, false);
	}

	return 0;
}

int CNatUdt::HandleRecvConnect( const UDP_WRAPPER_DATA *pData, int iLen, uint32 currentTick )
{
	//printf("HandleRecvConnect ConnectionID %lu  sendSeq%lu\n", m_RecordedInfo.initPeerSendSeq, pData->head.sendSeqNum);

	if ( iLen < sizeof(UDP_SYN_PACKET) 
		|| pData->wrapper_dat.syn.protocolTag != NAT_UDT_PROTOCOL_TAG/* || pData->wrapper_dat.syn.version*/)
	{
		NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: wrong protocol syn!\n", \
			m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		return 0;
	}

	int ret = 0;
	bool bSendAck = false;

	switch(m_RecordedInfo.state)
	{
	case SWL_UDP_STATE_UNCONNECTED:
		if (0 == pData->head.ackNum)
		{
			NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: [1] recv dest peer 1st syn!\n", \
				m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());	

			m_RecordedInfo.state = SWL_UDP_STATE_RECVED;
			bSendAck = true;
		}
		else if(m_RecordedInfo.initSendSeq == pData->head.ackNum)
		{
			NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: [2] dest peer has recv syn, but do not known if this peer have recv syn!\n", \
				m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());

			m_RecordedInfo.state = SWL_UDP_STATE_CONNECTED;
			OnNotifyConnect(0);
			bSendAck = true;
		}
		else
		{
			//����
			NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: useless syn!\n", \
				m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		}
		break;
	case SWL_UDP_STATE_RECVED:
		if (m_RecordedInfo.initSendSeq + 1 == pData->head.ackNum)
		{
			NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: [4] dest peer has been in connected state!\n", \
				m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());

			m_RecordedInfo.state = SWL_UDP_STATE_CONNECTED;
			OnNotifyConnect(0);
		}
		else if(0 == pData->head.ackNum)
        {
            NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: recv syn[2] again, just reply!\n", \
                m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
			bSendAck = true;
		}
		else if(m_RecordedInfo.initSendSeq == pData->head.ackNum)
		{
			//˫��򿪵��������ʱ�Է��յ��ҷ��ĵ�һ�����ְ��ˣ�Ҳ��Ϊ���ӽ���
            NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: [3] dest peer has recv syn, and known this peer have recv syn!\n", \
                m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
			m_RecordedInfo.state = SWL_UDP_STATE_CONNECTED;
			OnNotifyConnect(0);
			bSendAck = true;
		}
		else
		{
			//�����SYN  ����
			NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: recv wrong syn ack!\n", \
				m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
		}
		break;
	case SWL_UDP_STATE_CONNECTED:
		assert(m_RecordedInfo.initPeerSendSeq != 0);
		if (m_RecordedInfo.initSendSeq == pData->head.ackNum)
		{
            //���������ֶԷ�û�յ����ط�
            NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: this peer has connected, just reply syn!\n", \
                m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
			bSendAck = true;
		}
		else if(pData->head.ackNum == 0)
		{
			//assert(false);//�����Ѿ��������ˣ���һ��SYN���ŵ���
			//�Է���ʱ���쳣����ͼ����
			if (m_RecordedInfo.initPeerSendSeq == pData->head.seqNum)
			{
				//ͬһ��peer�������ģ�Ӧ�ò��������������
				NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: useless syn!\n", \
					m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
			}
			else
			{
				//��ͬpeer
				NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu] Connect Syn: differnt syn\n", \
					m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());
			}
		}
		else if (m_RecordedInfo.initSendSeq + 1 == pData->head.ackNum)
		{
			//�����Ѿ�����������
		}
		break;
	default:
		assert(false);
		break;
	}

	if (bSendAck)
	{
		m_RecordedInfo.initPeerSendSeq = pData->head.seqNum;
		m_RRecvInfo.maxPeerAck = m_RecordedInfo.initSendSeq;	
		m_RRecvInfo.expectedSeq = pData->head.seqNum + 1;
		m_RRecvInfo.minSeq = m_RRecvInfo.expectedSeq;

		m_RecordedInfo.maxRecvIndex = pData->head.sendIndex;

		UDP_SYN_PACKET synPacket;
		synPacket.head.category = m_Category;
		synPacket.head.cmdId = UDP_DATA_TYPE_SYN;
		synPacket.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;

		//SYN���͵����ݣ�����������û����
		synPacket.head.sendIndex = m_RecordedInfo.lastSendIndex;
		synPacket.head.recvIndex = m_RecordedInfo.maxRecvIndex;

		synPacket.head.seqNum = m_RecordedInfo.initSendSeq;
		synPacket.head.connectionID = GetConnectionID();
		synPacket.data.protocolTag = NAT_UDT_PROTOCOL_TAG;
		//synPacket.data.version

		if (SWL_UDP_STATE_CONNECTED == m_RecordedInfo.state)
		{
			synPacket.head.ackNum = pData->head.seqNum + 1;
		}
		else if(SWL_UDP_STATE_RECVED == m_RecordedInfo.state)
		{
			synPacket.head.ackNum = pData->head.seqNum;
		}	

		//�������һ�η���ʱ��
		m_RecordedInfo.lastSendTick = currentTick;

		//��¼����������ĵ���������
		//m_dwMaxRecvReliableBuffCount = (MAX_RECV_RELIABLE_BUFFSIZE / pData->head.maxPacketLen) + 1;

		OnNotifySendPacket(&synPacket, sizeof(UDP_SYN_PACKET));
	}

	return ret;	
}

int CNatUdt::HandleRecvRST( const UDP_WRAPPER_DATA *pData, int iLen, uint32 currentTick )
{
	NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  HandleRecvRST\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());

	m_RecordedInfo.state = SWL_UDP_STATE_UNCONNECTED;

	return 0;
}

int CNatUdt::ResendData( uint32 currentTick )
{
	if (SWL_UDP_STATE_CONNECTED != m_RecordedInfo.state)
	{
		return -5;
	}

#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif


	UDP_SEND_PACKET_DATA *pPacket = NULL;
	std::list<UDP_SEND_PACKET_DATA*>::iterator it = m_RSendInfo.listUnackedData.begin();
	uint32 dwResendTime = m_dwRTO << m_CtrlAlg.resendTime;//ָ���˱�
	dwResendTime = dwResendTime > MAX_RESEND_TIME ? MAX_RESEND_TIME : dwResendTime;

	//ÿ�ν�������ͷ��β������Ч�ʱȽϵ�	
	//bug ��δ����Ƿ���Կ�����������ʵ�ַ�ʽ���棬��m_RSendInfo.listUnackedData�������ݱȽ϶��ʱ��
	//Ч�ʻ�Ƚϵ�
	for(; it != m_RSendInfo.listUnackedData.end(); ++it)
	{
		pPacket = *it;						
		if (SWL_UDP_DATA_STATUS_SENDED == pPacket->dataStatus)
		{
			if (PUB_IsTimeOutEx(pPacket->lastSendTick, dwResendTime, currentTick))
			{
				IncreaseCwnd(1, true);
				pPacket->dataStatus |= SWL_UDP_DATA_STATUS_TIMEOUT;
			}
		}		
	}


	if (m_iUsed >= m_iCwnd)
	{
		return -6;
	}

	bool bFirst = true;
	//bug ѭ����SendPacketData���п��ܽ���Ч�ʣ���ϸ�о�һ����ô�����������Ч��
	//����ѭ���ܶ�Σ��ط������ĳ������ʱ��ѭ����Ч�ʱȽϵ�
	for(it = m_RSendInfo.listUnackedData.begin(); it != m_RSendInfo.listUnackedData.end(); ++it, bFirst = false)
	{
		//�ش���һ�κ�û��index���£����2�ο�ʼÿ��ֻ�ش���һ����
		//������������ڲ���������
		if (!bFirst)
		{
			if(m_CtrlAlg.resendTime > 1)
			{
				return -6;
			}
		}
		pPacket = *it;
		if ((SWL_UDP_DATA_STATUS_TIMEOUT & pPacket->dataStatus)
			|| (SWL_UDP_DATA_STATUS_BEYOND & pPacket->dataStatus))
		{
			byte dataStatus = pPacket->dataStatus;
#ifdef	RTT_RESOLUTION_1
			//RTT����һ��
			//��ΪRTT����һ���緽����׼�����ĳ�����������ڼ���RTT�����Ƿ������ش���
			//���������ǰʱ���������¼���RTT

			//modified by cc  Ӧ��ΪpPacket->lastSendIndex
			if (m_CtrlAlg.bCalculateRtt && m_CtrlAlg.rttHelp.sendIndex == pPacket->lastSendIndex)
			{
				//������Ϊfalse����Ҫ��SendPacketData�����¸���rtthelp�ķ���ʱ��
				m_CtrlAlg.bCalculateRtt = false;
			}
#else
#endif
			m_Statistic.reSendPacketNum++;
			m_Statistic.reSendBytes += pPacket->iLen+sizeof(UDP_WRAPPER_HEAD);

			int iRet = SendPacketData(pPacket, currentTick);
			if(0 == iRet)
			{	
				if (bFirst)
				{
					do 
					{
						//�ش�����ʱ��û�г���m_dwNextResendIndex�򲻻��ٴ��л�ĳ���ش�״̬
						if(m_CtrlAlg.bResendRestrict && PUB_UNSIGNED_LONG_LEQ(pPacket->lastSendIndex, m_CtrlAlg.nextResendIndex))
						{
							break;
						}

						//�ٴν��ش�
						if (0 != m_CtrlAlg.firstResend)
						{
							ChangeMode(SWL_UDP_SENDMODE_TIMEOUT_RESEND);
						}
						else	//�״��ش�
						{
							//��ʱ�ش�
							if (SWL_UDP_DATA_STATUS_TIMEOUT & dataStatus)
							{
								ChangeMode(SWL_UDP_SENDMODE_TIMEOUT_RESEND);
							}
							//�����ش�
							else if(SWL_UDP_DATA_STATUS_BEYOND & dataStatus)
							{
								ChangeMode(SWL_UDP_SENDMODE_FAST_RESEND);
							}
							else
							{
								assert(false);
							}
						}

						//�ش�����+1

						++m_CtrlAlg.resendTime;
						++m_CtrlAlg.firstResend;
					} while(false);				
				}			

				++m_iUsed;
				return 1;
			}
			else if(-1 == iRet)
			{		
				return -1;
			}
			else //if(1 == iRet) socket EWOULDBLOCK
			{
				return -2;
			}
		}
	}

	return 0;
}

int CNatUdt::SendPacketData( UDP_SEND_PACKET_DATA *pPacket, uint32 currentTick )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	int iLen = 0;

	pPacket->wrapperData.head.category = m_Category;
	pPacket->wrapperData.head.cmdId = UDP_DATA_TYPE_RELIABLE;
	pPacket->wrapperData.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;
	pPacket->wrapperData.head.sendIndex = m_RecordedInfo.lastSendIndex + 1;
	pPacket->wrapperData.head.recvIndex = m_RecordedInfo.maxRecvIndex;
	pPacket->wrapperData.head.seqNum = pPacket->wrapperData.head.seqNum;
	pPacket->wrapperData.head.ackNum = m_RRecvInfo.expectedSeq - 1;
	pPacket->wrapperData.head.connectionID = GetConnectionID();

	if (0 != pPacket->iLen)
	{
		iLen = sizeof(UDP_WRAPPER_HEAD) + pPacket->iLen;			
	}
	else
	{
		iLen = sizeof(UDP_WRAPPER_HEAD);
	}

	int iRet = OnNotifySendPacket(&(pPacket->wrapperData), iLen);

	if(0 == iRet)
	{
		++m_RecordedInfo.lastSendIndex;

		//�������һ�η���ʱ��
		m_RecordedInfo.lastSendTick = currentTick;

		//�Ӵ������԰�DELAY��NOW���
		if((SWL_UDP_ACK_STATUS_DELAY == m_RecordedInfo.ackStatus)
			||(SWL_UDP_ACK_STATUS_NOW == m_RecordedInfo.ackStatus))
		{
			m_RecordedInfo.ackStatus = SWL_UDP_ACK_STATUS_NONE;
		}

		//������ݷ��͹�
		pPacket->dataStatus = SWL_UDP_DATA_STATUS_SENDED;

		pPacket->lastSendIndex = m_RecordedInfo.lastSendIndex;

		//�޸ļ�ʱʱ��
		pPacket->lastSendTick = currentTick;
#ifdef	RTT_RESOLUTION_1
		//RTT����һ��
		if (!m_CtrlAlg.bCalculateRtt) 
		{
			m_CtrlAlg.rttHelp.sendIndex = m_RecordedInfo.lastSendIndex;
			m_CtrlAlg.rttHelp.sendTick = currentTick;
			m_CtrlAlg.bCalculateRtt = true;
		}
#else
		//RTT��������
		UDP_RTT_HELP udpRttHelp;
		udpRttHelp.sendIndex = m_dwIndexSendLast;
		udpRttHelp.sendTick = currentTick;
		m_listRttHelp.push_back(udpRttHelp);
#endif
	}
	return iRet;
}

int CNatUdt::SendReliableData( uint32 currentTick )
{
	if (SWL_UDP_STATE_CONNECTED != m_RecordedInfo.state)
	{
		return -5;
	}

	//��������
	if (m_iUsed >= m_iCwnd)			
	{
		return -6;
	}

#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	int ret = 0;
	UDP_SEND_PACKET_DATA *pPacket = NULL;
	do  
	{
		if (m_RSendInfo.listReliableSendData.empty())
		{
			break;
		}
		
		pPacket = m_RSendInfo.listReliableSendData.front();
		assert(pPacket);

		//�Ѿ��� MAX_SEND_RELIABLE_WINDOW ����û��ȷ���ˣ�����ʱ�Ȳ���������
        // -- ���Ż�������û�з��ʹ��ڴ�С�Ķ�̬�������˴��ж�����ʱ���ԡ�
// 		if(PUB_UNSIGNED_LONG_GEQ(pPacket->wrapperData.head.seqNum, m_RRecvInfo.maxPeerAck + MAX_SEND_RELIABLE_WINDOW))
// 		{
// 			ret = -3;
// 			break;
// 		}

		int iRet = SendPacketData(pPacket, currentTick);
		if(0 == iRet)
		{
			//�ɹ�������һ�����ݣ����ͳ�ȥ���������ϵ�������+1
			++m_iUsed;
			ret = 1;
			
			m_Statistic.totalSendPacketNum++;
			m_Statistic.totalSendBytes += pPacket->iLen+sizeof(UDP_WRAPPER_HEAD);

			//�����ݻ�������
			m_RSendInfo.listUnackedData.push_back(pPacket);
			m_RSendInfo.listReliableSendData.pop_front();
			break;
		}
		else if(-1 == iRet)
		{
			ret = -1;		
			break;
		}
		else//OnSendPacket�Ƿ�Ҫ����һ�������������������������
		{				
			ret = -2;
			break;
		}
	}while(0);
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif
	return ret;
}

//�����������ⲿ���õĽӿڶ�����һ���߳���
//����RST����û�н���RST��ACK������֤�Է�һ�����յ�RST
void CNatUdt::Disconnect()
{

	if(m_RecordedInfo.state != SWL_UDP_STATE_CONNECTED)
    {
        m_RecordedInfo.state = SWL_UDP_STATE_UNCONNECTED;
		return;
	}
	m_RecordedInfo.state = SWL_UDP_STATE_UNCONNECTED;

	NAT_LOG_DEBUG("[Peer Ip:%s  port:%d  ConnectionID:%lu]  Send Disconnect\n", m_RemoteInfo.Ip, m_RemoteInfo.port, GetConnectionID());

	UDP_SYN_PACKET synPacket;
	synPacket.head.category = m_Category;
	synPacket.head.cmdId = UDP_DATA_TYPE_RST;
	synPacket.head.maxPacketLen = MAX_WRAPPER_DATA_LEN;
	synPacket.head.sendIndex = m_RecordedInfo.lastSendIndex;
	synPacket.head.recvIndex = m_RecordedInfo.maxRecvIndex;
	synPacket.head.seqNum = 0;//û��
	synPacket.head.ackNum = 0;//û��
	synPacket.head.connectionID = GetConnectionID();
	synPacket.data.protocolTag = NAT_UDT_PROTOCOL_TAG;
	//synPacket.data.version

	OnNotifySendPacket(&synPacket, sizeof(UDP_WRAPPER_HEAD));
}

void CNatUdt::SetAutoRecv( bool IsCallBack )
{
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif
	m_bIsAutoRecv = IsCallBack;
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif
}

int CNatUdt::Recv( void *pData, int iLen )
{
#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	if (m_RecordedInfo.state != SWL_UDP_STATE_CONNECTED)
	{
		return -1;
	}

	int ret = 0, dataLen = 0;
	UDP_WRAPPER_DATA *pWrapperData = NULL;
	NatUdtRecvDataList::iterator it;
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif
	do
	{
		if (m_bIsAutoRecv)
		{
			ret = -1;
			break;
		}

		it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.minSeq);
		while (it != m_RRecvInfo.recvDataList.end() && iLen > 0)
		{
			pWrapperData = &(*it).second->pPacket;
			dataLen = (*it).second->PacketSize() - m_RRecvInfo.lastOnRecvPackPos;

			assert(dataLen > 0);

			if (dataLen <= iLen)
			{
				memcpy((char *)pData+ret, pWrapperData->wrapper_dat.data + m_RRecvInfo.lastOnRecvPackPos, dataLen);
				ret += dataLen;
				iLen -= dataLen;
				m_RRecvInfo.lastOnRecvPackPos = 0;
			}
			else
			{
				memcpy((char*)pData+ret, pWrapperData->wrapper_dat.data + m_RRecvInfo.lastOnRecvPackPos, iLen);
				ret += iLen;
				m_RRecvInfo.lastOnRecvPackPos += iLen;
				break;
			}
			
			//ȡ��һ����
            DestroyRecvData((*it).second);
			m_RRecvInfo.recvDataList.erase(it);
			++(m_RRecvInfo.minSeq);
			it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.minSeq);
		}

	}while(0);
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif
	return ret;
}

void CNatUdt::AutoNotifyRecvPacket()
{
	//���ݻص���Ӧ�ò�
	int	dataLen = 0, heapDataLen = 0;
	int ret = 0;
	UDP_WRAPPER_DATA *pWrapperData = NULL;
	NatUdtRecvDataList::iterator it;
	it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.minSeq);
	while(it != m_RRecvInfo.recvDataList.end())
	{
		pWrapperData = &(*it).second->pPacket;
		dataLen = (*it).second->PacketSize() - m_RRecvInfo.lastOnRecvPackPos;
		assert(dataLen > 0);

		ret = OnNotifyRecvPacket(pWrapperData->wrapper_dat.data + m_RRecvInfo.lastOnRecvPackPos, dataLen);

		if (ret == dataLen)
		{
			m_RRecvInfo.lastOnRecvPackPos = 0;
			DestroyRecvData((*it).second);
			m_RRecvInfo.recvDataList.erase(it);

			++(m_RRecvInfo.minSeq);
			it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.minSeq);
		}
		else if(ret > 0)
		{
			m_RRecvInfo.lastOnRecvPackPos += ret;
			break;
		}
		else//ret<=0
		{
			break;
		}
	}
}

int CNatUdt::GetSendAvailable()
{
	int ret = 0;
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.Lock();
#endif
	ret = m_RSendInfo.remainedBuffSize;
#ifndef UDT_WITHOUT_LOCK
	m_RSendInfoLock.UnLock();
#endif
	return ret;
}

int CNatUdt::GetRecvAvailable()
{

#ifdef TEST_PROFILE
	static FunctionProfile __funcProfile(__FUNCTION__);
	ProfilerStackFrame __funcFrame(&__funcProfile);
#endif

	int ret = 0;
	NatUdtRecvDataList::iterator it;
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.Lock();
#endif
	it = m_RRecvInfo.recvDataList.find(m_RRecvInfo.minSeq);
	if (it != m_RRecvInfo.recvDataList.end())
	{
		ret = (*it).second->PacketSize() - m_RRecvInfo.lastOnRecvPackPos;
	}
#ifndef UDT_WITHOUT_LOCK
	m_RecvLock.UnLock();
#endif

	return ret;
}

bool CNatUdt::IsMineDatagram(const NAT_TRANSPORT_RECV_DATAGRAM* datagram)
{
	if (!IsDatagramValid(datagram))
	{
		return false;
	}

    const UDP_WRAPPER_HEAD *packetHeader = reinterpret_cast<const UDP_WRAPPER_HEAD *>(datagram->data);
    return packetHeader->category == this->m_Category
        && packetHeader->connectionID == GetConnectionID()
        && datagram->fromIp == m_RemoteInfo.ip 
        && datagram->fromPort== m_RemoteInfo.port;
}


int CNatUdt::NotifyRecvDatagram(const NAT_TRANSPORT_RECV_DATAGRAM* datagram)
{
	if (!IsDatagramValid(datagram))
	{
		return -1;
	}
	return RecvPacket((const UDP_WRAPPER_DATA *)datagram->data, datagram->dataSize);
}

bool CNatUdt::InsertRecvData( uint32 seqNum, const UDP_WRAPPER_DATA* packet, int dataSize )
{
    NatUdtRecvDataList::iterator it = m_RRecvInfo.recvDataList.find(seqNum);
    if(it != m_RRecvInfo.recvDataList.end())
    {		
        NAT_LOG_DEBUG("Udt recv packet repeat!\n");
        return false;
    }

    NAT_UDT_RECV_DATA* recvData = (NAT_UDT_RECV_DATA*)m_pRecvDataManager->GetMemory();
    memcpy(&recvData->pPacket, packet, dataSize);
    recvData->iPacketLen = dataSize;
    m_RRecvInfo.recvDataList.insert(NatUdtRecvDataList::value_type(seqNum, recvData));
    return true;

}