#ifndef  __AAC_ENCODER__
#define  __AAC_ENCODER__
#include "faac.h"
#include "TVT_DevDefine.h"
#include "G711Codec.h"

class  CAacEncoder
{
public:
    CAacEncoder();
    ~CAacEncoder();

    bool Init();
    void Quit();

    bool AudioFrameToAac(TVT_DATA_FRAME *pAudInFrame, TVT_DATA_FRAME **ppAudOutFrame);

private:
	unsigned long           m_maxOutputBytes;	//aac�������������ֽ���
	unsigned long           m_inputSamples;		//aac�������������������

	unsigned char*          m_PcmOutBuf;		//����ת������
    unsigned long           m_pcmDataLen;		//�����е�������

    faacEncHandle           m_encodeHandler;	//aac���������
	unsigned long			m_encodeUnitLen;	//aac������С
	unsigned char*          m_pOutBuff;			//g711�����������
    unsigned char*          m_pAacOutBuff;		//aac�����������

	TVT_DATE_TIME			m_dateTime;		//���浱ǰ֡��ʱ��
	bool					m_bGetTime;		//ʱ���Ѿ���ʼ����

	long long				m_totalSamples; //��ǰ�Ѿ���ȡ����Ƶ������
	CG711Codec				m_g711decode;	//����g711;

};
#endif