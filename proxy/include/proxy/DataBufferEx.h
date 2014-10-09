/***********************************************************************
** Copyright (C) Tongwei Video Technology Co.,Ltd. All rights reserved.
** Author       : YSW
** Date         : 2012-03-14
** Name         : DataBufferEx.h
** Version      : 1.0
** Description  :
** Modify Record:
1:
***********************************************************************/
#ifndef __DATA_BUFFER_EX_H__
#define __DATA_BUFFER_EX_H__
#include "TVT_PubCom.h"

const unsigned int DEFAULT_PAGE_LENGTH			= 4*1024;
const unsigned int DEFAULT_BUFFER_LENGTH_EX		= 3*512*1024;

#ifndef WIN32
const unsigned int		MAX_BUFFER_NUM			= 64;
#endif

class CDataBufferEx
{
public:
	CDataBufferEx(unsigned int bufLength = DEFAULT_BUFFER_LENGTH_EX);
	~CDataBufferEx();

	int GetBuffer(unsigned char **ppBuf, unsigned int & bufLength);
	void UsedBuffer(unsigned char *pBuf, unsigned int bufLength);
	void ReleaseBuf(unsigned char *pBuf, unsigned int bufLength);

#ifndef WIN32
	static void PushBuffer(unsigned char *pBuf, unsigned int index, int bufLength);
#endif
protected:
private:
	unsigned char	*m_pBuffer;
	unsigned int	m_bufLength;

	unsigned int	m_pageNum;

	//lh_modify 2012-12-14
	unsigned int	m_freeHeadPage;	//���пռ��ͷ����Ҳ����GetBuffer��λ��
	unsigned int	m_freeTailPage;	//���пռ��β����Ҳ���Ǵ��ͷŵĿ�ʼλ��
	unsigned int	m_bufTailPage;	//��GetBufferʱ������BUFFER�Ľ�β�����ˣ���ѵ�ǰ��m_freeHeadPageָ������ֵ
	unsigned int	m_freePageNum;
	bool			m_bBusy;		//GetBuffer�������û�е���UsedBuffer��Ϊtrue

#ifndef WIN32
	unsigned int			m_bufIndex;

	typedef struct _buffer_
	{
		unsigned long	length;
		unsigned char	*pBuffer;
		CDataBufferEx	*pThis;
	}BUFFER;

	static BUFFER			s_buffer[MAX_BUFFER_NUM];
	static CPUB_Lock		s_lockBuff;
#endif
};
#endif //__DATA_BUFFER_EX_H__
