// NatSocket.cpp: implementation of the NatSocket.
//
//////////////////////////////////////////////////////////////////////
#include "NatCommon.h"
#include "NatSocket.h"
#include "NatClientPeerManager.h"
#include "NatSockManger.h"
#include "NatDebug.h"

static unsigned long s_nat_Init = 0;
CPUB_Lock				g_natSockLock;


const char* NAT_GetClientErrorText(NAT_CLIENT_ERROR error)
{
    static const int ERROR_COUNT = 8;
    static const char* ERROR_NAME_LIST[ERROR_COUNT] = {
        "OK",                   // NAT_CLI_OK		= 0,               // �ɹ�
        "Unknown Error",        // NAT_CLI_ERR_UNKNOWN,                // δ֪����
        "Timeout Error",        // NAT_CLI_ERR_TIMEOUT,               // �ͻ��˳�ʱ
        "Device is Offline",    // NAT_CLI_ERR_DEVICE_OFFLINE,	       // �豸������
        "Server is Overload",   // NAT_CLI_ERR_SERVER_OVERLOAD,       // ���������ع��أ��޷����ṩ����
        "Device is no relay",   // NAT_CLI_ERR_DEVICE_NO_RELAY,       // �豸��֧���м�ͨ�Ź���
        "System Error",         // NAT_CLI_ERR_SYS,                   // ϵͳ����
        "Network Error",        // NAT_CLI_ERR_NETWORK                // �������
    };
    if (error >= 0 && error < 8)
    {
        return ERROR_NAME_LIST[error];
    }
    return ERROR_NAME_LIST[NAT_CLI_ERR_UNKNOWN];
}


int NAT_Init()
{
    int ret = 0;
    CNatScopeLock lock(&g_natSockLock);
    if(s_nat_Init == 0)
    {
        ++s_nat_Init;
//        SWL_Init(); // ����֧�ֶ�ε���
    }
    else if (s_nat_Init > 0)
    {
        ++s_nat_Init;
        return 0;
    }
    else
    {
        ret = -1;
        assert(false);
    }
    return ret;
}

int NAT_Quit()
{
    int ret = 0;
    CNatScopeLock lock(&g_natSockLock);

    --s_nat_Init;
    if (s_nat_Init == 0)
    {
        if(CNatClientPeerManager::IsInstanceInited())
        {
            CNatClientPeerManager::DestroyInstance();
        }
//        SWL_Quit(); // ����֧�ֶ�ε���
    }
    else if (s_nat_Init < 0)
    {
        ret = -1;
        assert(false);
    }

    return ret;
}


int NAT_CloseSocket(NatSocket sock)
{
    return CNatSockManager::GetSocketManager()->CloseSocket(sock);
}

int NAT_GetSocketInfo(NatSocket sock, NAT_SOCKET_INFO &info)
{
    return CNatSockManager::GetSocketManager()->GetSocketInfo(sock, info);
}

int NAT_Send(NatSocket sock, const void *pBuf, int iLen)
{
    return CNatSockManager::GetSocketManager()->Send(sock, pBuf, iLen);
}

int NAT_Recv(NatSocket sock, void *pBuf, int iLen)
{
    return CNatSockManager::GetSocketManager()->Recv(sock, pBuf, iLen);
}

int NAT_GetSendAvailable(NatSocket sock)
{
    return CNatSockManager::GetSocketManager()->GetSendAvailable(sock);
}

int NAT_GetRecvAvailable(NatSocket sock)
{
    return CNatSockManager::GetSocketManager()->GetRecvAvailable(sock);
}

inline int NAT_Select(const NatSocketSet *readSet, const NatSocketSet *writeSet, 
               NatSocketSet *readSetAvalible, NatSocketSet *writeSetAvalible)
{
    int ret = 0;
    NatSocketSet::const_iterator it;
    if (readSet != NULL)
    {
        assert(readSetAvalible != NULL);
        for (it = readSet->begin(); it!= readSet->end(); it++)
        {
            if (NAT_GetRecvAvailable(*it) != 0)
            {
                readSetAvalible->insert(*it);
            }
        }
        ret += (int)readSetAvalible->size();
    }

    if (writeSet != NULL)
    {
        assert(writeSetAvalible != NULL);
		for (it = writeSet->begin(); it!= writeSet->end(); it++)
        {
            if (NAT_GetSendAvailable(*it) != 0)
            {
                writeSetAvalible->insert(*it);
            }
        }
        ret += (int)writeSetAvalible->size();
    }

    return ret;
}

int NAT_Select(NatSocketSet *readSet, NatSocketSet *writeSet, int iTimeout)
{
    NatSocketSet::iterator it;
    NatSocketSet readSetAvalible;
    NatSocketSet writeSetAvalible;
    int ret = 0;
    if (iTimeout > 0)
    {
        int startTime = Nat_GetTickCount();
        do 
        {
            ret = NAT_Select(readSet, writeSet, &readSetAvalible, &writeSetAvalible);
			if(0 == ret)
			{
				PUB_Sleep(1);
			}

        } while (ret == 0 && !PUB_IsTimeOutEx(startTime, iTimeout, Nat_GetTickCount()));
    }
    else
    {
        ret = NAT_Select(readSet, writeSet, &readSetAvalible, &writeSetAvalible);
    }

    if (readSet != NULL)
    {
        *readSet = readSetAvalible;
    }
    if (writeSet != NULL)
    {
        *writeSet = writeSetAvalible;
    }
    assert(ret == (int)(readSetAvalible.size() + writeSetAvalible.size()));
    return ret;
    
}


bool NAT_IsSocketInSet(NatSocket sock, NatSocketSet *socketSet)
{
    if (socketSet != NULL)
    {
        return socketSet->find(sock) != socketSet->end();
    }
    return false;
}

void NAT_EraseSocketFromSet(NatSocket sock, NatSocketSet *socketSet)
{
    if (socketSet != NULL)
    {
        socketSet->erase(sock);
    }
}

void NAT_InsertSocketToSet(NatSocket sock, NatSocketSet *socketSet)
{
    if (socketSet != NULL)
    {
        socketSet->insert(sock);
    }
}