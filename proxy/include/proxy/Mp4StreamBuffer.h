#ifndef _MP4_STREAM_BUFFER_H_
#define _MP4_STREAM_BUFFER_H_
#include "StreamBuffer.h"

class CMp4StreamBuffer : public CStreamBuffer
{
public:
	CMp4StreamBuffer();
	virtual ~CMp4StreamBuffer();

	//Ϊ�����������ɾ������;
	virtual bool AddBuffer(long bufSize,long bufUnitSize,bool bMainStream);
	virtual bool DelBuffer(bool bMainStream);

	//�ж�ʱ��ɾ�������л���
	virtual bool bDelAllBuffer();

	//�����ݷ��뻺��
	virtual bool InputData(bool bMainStream,char *pData,unsigned long dataLen);

	//��ȡ���������
	virtual bool GetData(bool bMainStream,char **ppData,unsigned long *pDataLen);

	//�ͷ����ݿ�
	virtual void RelData(bool bMainStream,char *pData,unsigned long dataLen);

	//����mp4��ʹ��
	virtual unsigned long NeedFeedBack();
private:
	char *m_pBuf;
	long m_bufSize;
	long m_unitSize;

	STREAM_BLOCK_LIST m_emptyList;
	STREAM_BLOCK_LIST m_filledList;
	STREAM_BLOCK_LIST m_releaseList;

	unsigned long m_recvBytes;
	unsigned long m_feedBackBytes;
};

#endif