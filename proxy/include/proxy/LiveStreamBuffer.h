#ifndef _LIVE_STREAM_BUFFER_H_
#define _LIVE_STREAM_BUFFER_H_
#include "StreamBuffer.h"
#include "TSMuxer.h"
#include "aacEncode.h"

class CLiveStreamBuffer : public CStreamBuffer
{
	typedef struct _live_stream_buf_
	{
		char *pBuf;
		long bufSize;
		long unitSize;
		STREAM_BLOCK_LIST *pEmptyList;
		STREAM_BLOCK_LIST *pFilledList;
		STREAM_BLOCK_LIST *pReleaseList;
		CTSMuxer *pTSMuxer;
		long programID;
		long videoID;
		long audioID;
		char *pData;
		int dataLen;
	}LIVE_STREAM_BUF;

public:
	CLiveStreamBuffer();
	virtual ~CLiveStreamBuffer();

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
	bool InputStreamBuffer(char *pData,unsigned long dataLen,bool bMainStream);
	
	void DelLiveStream(LIVE_STREAM_BUF *pLiveStream);
private:
	LIVE_STREAM_BUF *m_pMainStreamBuf;
	LIVE_STREAM_BUF *m_pSubStreamBuf;

	CAacEncoder		m_encode;
};

#endif
