
/********************************************************************************************
*		����Э�̣�Encrypt Agreement��Э��
*
*		1��Э�����ݰ����Ƿ�Ҫ���ܡ�������Կ���͡�ÿһ�������Ƿ�������
*		2������Э��Э��͸�������ϲ����ݣ�ͬʱ�����ϲ��������Ĵ��估���Ĵ���
*
********************************************************************************************/

#ifndef EA_PROTOCOL_H_
#define EA_PROTOCOL_H_

const unsigned int EA_HEAD_FLAG = '1111';

typedef enum _ea_frame_type
{
	EA_FRAME_PLAIN,				//����
	EA_FRAME_AES,				//AES��������

	EA_FRAME_AGREEMENT	= 255,	//Э������
}EA_FRAME_TYPE;

typedef enum _ea_agreement_cmd_
{
	EA_CMD_SET_ENCRYPT_MODE,
	EA_CMD_PUBLIC_KEY,
	EA_CMD_USER_KEY,
	EA_CMD_AGREEMENT_SUCC,
	EA_CMD_AGREEMENT_FAIL,
}EA_AGREEMENT_CMD;

typedef struct _ea_frame_head_
{
	unsigned int		headFlag;		//EA_HEAD_FLAG
	unsigned int		frameLength;	//֡����
}EA_FRAME_HEAD;

typedef struct _ea_frame_info_
{
	EA_FRAME_HEAD		frameHead;
	unsigned char		frameType;		//EA_FRAME_TYPE
	unsigned char		alignPadding;	//Ϊ���������ӵ��ֽ���
	unsigned char		resv[2];
}EA_FRAME_INFO;

typedef struct _ea_cmd_head_
{
	unsigned int		cmdId;		//EA_AGREEMENT_CMD
	unsigned int		paramLen;	//��������
}EA_CMD_HEAD;


/************************************************   Э������˵��    **********************************************/

//EA_CMD_SET_ENCRYPT_MODE
//�ͻ������������������ָ������Ự�費��Ҫ���ܡ������Ҫ���ɹ��ظ�EA_CMD_PUBLIC_KEY��ʧ�ܻظ�EA_CMD_AGREEMENT_FAIL���������Ҫ����ظ�EA_CMD_AGREEMENT_SUCC
//���͵��������Ϊ��EA_FRAME_INFO + EA_CMD_HEAD + int(����ģʽ��ȡֵΪEA_FRAME_TYPE)

//EA_CMD_PUBLIC_KEY
//�������˶Կͻ��˵Ļ�Ӧ�����ͻ���Ҫ����ܴ���ʱ��Ӧ������Ϊrsa���ܵĹ�Կ
//���͵��������Ϊ��EA_FRAME_INFO + EA_CMD_HEAD + unsigned char [EA_CMD_HEAD.paramLen]

//EA_CMD_USER_KEY
//�ͻ��������������Ҫ�������ݼ�����Կ�������������ݼ�����Կ�ù�Կ����
//�������������Ϊ��EA_FRAME_INFO + EA_CMD_HEAD + unsigned char [EA_CMD_HEAD.paramLen]

//EA_CMD_AGREEMENT_SUCC
//�������˶Կͻ��˵Ļ�Ӧ����ʾЭ�̳ɹ�����Э�̵ļ��ܼ���Կ�������ݴ���
//���͵��������Ϊ��EA_FRAME_INFO + EA_CMD_HEAD

//EA_CMD_AGREEMENT_FAIL
//�������˶Կͻ��˵Ļ�Ӧ����ʾЭ��ʧ�ܣ���ʱҪô����Э�̣�Ҫô�����Ĵ���
//���͵��������Ϊ��EA_FRAME_INFO + EA_CMD_HEAD

#endif

