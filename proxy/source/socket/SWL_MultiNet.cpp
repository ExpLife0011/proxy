// SWL_MultiNetComm.cpp: implementation of the CSWL_MultiNet class.
//
//////////////////////////////////////////////////////////////////////
#include "SWL_MultiNet.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSWL_MultiNet::CSWL_MultiNet()
{

}

CSWL_MultiNet::~CSWL_MultiNet()
{
}

bool CSWL_MultiNet::Init()
{
 	return true;
}

void CSWL_MultiNet::Destroy()
{
}
/******************************************************************************
*
*
*
******************************************************************************/
int CSWL_MultiNet::AddComm(long deviceID, TVT_SOCKET sock)
{
	return 0;
}

/******************************************************************************
*
*
*
******************************************************************************/
int CSWL_MultiNet::RemoveComm(long deviceID)
{
	return 0;
}

int CSWL_MultiNet::RemoveAllConnect()
{
	return 0;
}

/******************************************************************************
*
*	����ͬ���������ݵķ�ʽ��ͨ���˽ӿ�ֱ�ӵ���socket���ͽӿڷ�������
*
*	����ֵ��
*	���ͳɹ�������ʵ�ʷ��ͳɹ����ֽ���
	����ʧ�ܣ�����-1
*
*	������bBlock: �������Ϊtrue�����˽ӿ�Ϊ����ģʽ�����ȴ����ݷ�����ϻ�ʱΪֹ
*
******************************************************************************/
int CSWL_MultiNet::SendData(long deviceID, const void *pBuf, int iLen, bool bBlock)
{
	return -1;
}

