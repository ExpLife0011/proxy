#ifndef _STREAM_BUFFER_MAN_H_
#define _STREAM_BUFFER_MAN_H_

#include "TVT_PubCom.h"
#include "StreamBuffer.h"
#include <list>


class CStreamBufferMan
{
	typedef struct _data_block_
	{	
		long deviceID;
		bool bLive;
		long channl;

		CStreamBuffer *pStream;
	}STREAM_BUFFER;

	typedef std::list<STREAM_BUFFER*> STREAM_BUFFER_LIST;

public:
	CStreamBufferMan();
	~CStreamBufferMan();

	bool Init(long bufSize = STREAM_BUF_SIZE,long bufUnitSize = STREAM_UNIT_SIZE);

	void Quit();

	bool InputData(long deviceID,long channl,bool bLive,bool bMainStream,char *pData,unsigned long dataLen);

	//��ȡ���������
	bool GetData(long deviceID,long channl,bool bLive,bool bMainStream,char **ppData,unsigned long *pDataLen);

	//�ͷ����ݿ�
	void RelData(long deviceID,long channl,bool bLive,bool bMainStream,char *pData,unsigned long dataLen);

	//Ϊ�豸��ӻ���
	bool AddDeviceBuf(long deviceID,long channl,bool bLive,bool bMainStream);

	//ɾ���豸ָ��ͨ����Buf
	void DelDeviceBuf(long deviceID,long channl,bool bLive,bool bMainStream);

	//ɾ���豸������Buf
	void DelDeviceBuf(long deviceID);

	//����mp4��ʹ��
	unsigned long NeedFeedBack(long deviceID,long channl,bool bLive);

private:
	STREAM_BUFFER * FindStreamBuffer(long deviceID,long channl,bool bLive);

private:
	STREAM_BUFFER_LIST m_streamBufList;
	CPUB_Lock m_streamBufLock;
	
	long m_streamBufSize;
	long m_bufUnitSize;
};
#endif