#ifndef _STREAM_BUFEER_H_
#define _STREAM_BUFEER_H_

#include <list>

typedef struct _stream_block_
{
	char *pBuf;
	long dataLen;
}STREAM_BLOCK;

typedef std::list<STREAM_BLOCK *> STREAM_BLOCK_LIST;

const int STREAM_BUF_SIZE  = 8*1024*1024;
const int STREAM_UNIT_SIZE = 8*1024;


class CStreamBuffer
{
public:
	CStreamBuffer(){}
	virtual ~CStreamBuffer(){}

	//Ϊ�����������ɾ������;
	virtual bool AddBuffer(long bufSize,long bufUnitSize,bool bMainStream) = 0;
	virtual bool DelBuffer(bool bMainStream) = 0;
	
	//�ж�ʱ��ɾ�������л���
	virtual bool bDelAllBuffer() = 0;

	//�����ݷ��뻺��
	virtual bool InputData(bool bMainStream,char *pData,unsigned long dataLen) = 0;

	//��ȡ���������
	virtual bool GetData(bool bMainStream,char **ppData,unsigned long *pDataLen) = 0;

	//�ͷ����ݿ�
	virtual void RelData(bool bMainStream,char *pData,unsigned long dataLen) = 0;

	//����mp4��ʹ��
	virtual unsigned long NeedFeedBack() = 0;
};

#endif