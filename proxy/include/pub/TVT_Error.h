/***********************************************************************
** ����ͬΪ����Ƽ����޹�˾��������Ȩ����
** ����			: Ԭʯά
** ����         : 2012-04-23
** ����         : TVT_Error.h
** �汾��		: 1.0
** ����			:
** �޸ļ�¼		:
***********************************************************************/
#ifndef __TVT_ERROR_H__
#define __TVT_ERROR_H__

//////////////////////////////////////////////////////////////////////////
typedef enum _tvt_msg_ack_
{
	TVT_MSG_ACK_SUCC,								//�ɹ�


	TVT_MSG_ACK_PROTOCOL_NOT_MATCH		= 0x0300,	//Э�鲻ƥ��
	TVT_MSG_ACK_DEVICE_UNINITIAL,					//�豸δ��ʼ��
	TVT_MSG_ACK_CONNECT_OVERLOAD,					//�������ӳ���
	TVT_MSG_ACK_SYSTEM_BUSY,						//ϵͳæ
	TVT_MSG_ACK_NO_CAPACITY,						//ϵͳ��������


	TVT_MSG_ACK_ID_PW_WRONG 		     = 0x0400,	//�û��������������
	TVT_MSG_ACK_USER_BLOCK,							//�û�������:�ڰ�����,MAC��ַ,���߳�
	TVT_MSG_ACK_USER_MAX,							//�ﵽ��������û�����
	TVT_MSG_ACK_HIDE_CHANNEL,						//ͨ��������:�����ͨ�����޷�����,ȫ����������ͨ������
	TVT_MSG_ACK_NO_AUTHORITY,						//�޲���Ȩ��:�����ͨ�����޷�����,�������ֱ���������
	TVT_MSG_ACK_NO_FULL_AUTHORITY,					//ֻ�в���Ȩ��:�����ͨ��,ֻҪ��һ�������Ͽ��Բ���
	TVT_MSG_ACK_NO_FUNC,							//��֧�ָù���
	TVT_MSG_ACK_GET_CONFIG_FAIL,					//��ȡ����ʧ��
	TVT_MSG_ACK_SET_CONFIG_FAIL,					//��������ʧ��
	TVT_MSG_ACK_GET_DATA_FAIL,						//��ȡ����ʧ��
	TVT_MSG_ACK_NO_CHG_TIME_FOR_DISK,               //���ڸ�ʽ������,�����޸�ʱ��.���Ժ�����
	TVT_MSG_ACK_NO_CHG_TIME_FOR_READ,               //���ڶ�����,�����޸�ʱ��.���Ժ�����
	TVT_MSG_ACK_NO_CHG_TIME_FOR_TALK,               //���ڶԽ�,�����޸�ʱ��.���Ժ�����


	TVT_MSG_ACK_TIME_OUT                 = 0x0500,	//����ȴ���ʱ
	TVT_MSG_ACK_CONNECT_SERVER_FAIL,				//���ӷ���������
	TVT_MSG_ACK_SEND_FAIL,							//send���������ʧ��
	TVT_MSG_ACK_DDNS_REGISTER_FAIL,					//DDNSע��ʧ��
	TVT_MSG_ACK_DDNS_HOSTDOMAIN_ERR,				//DDNS�û������������������
	TVT_MSG_ACK_TALK_LINE_BUSY,	                    //�Խ�ռ��
	TVT_MSG_ACK_DEVICE_DOWN,						//����δ����,δ������,����������
	TVT_MSG_ACK_DHCP,						        //DHCP������
	TVT_MSG_ACK_DHCP6,						        //ipv6��DHCP������
	TVT_MSG_ACK_IP_CONFLICT,						//��̬���õ�IP��ͻ
	TVT_MSG_ACK_NETWORK_PARAM_FORMAT_ERR,			//���������ʽ����
	TVT_MSG_ACK_NETWORK_UNREACHABLE,				//���粻�ɴ�
	TVT_MSG_ACK_CANNT_PREVIEW,						//����Ԥ����ԭ����������������Ȩ�޵ȣ�

	TVT_MSG_ACK_PTZ_WRITE_COM_FAIL       = 0x0600,	//д����ʧ��
	TVT_MSG_ACK_PTZ_OPERATE_FAIL,					//������̨ʧ��

	TVT_MSG_ACK_GET_DISK_FAIL            = 0x0700,  //��ȡ������Ϣʧ��
	TVT_MSG_ACK_NETDISK_NO_SUPPORT,                 //�û�����֧���������
	TVT_MSG_ACK_NETDISK_CONNECT_LOST,				//������̲������Ӷ�ʧ
	TVT_MSG_ACK_NETDISK_PASSWD_ERR,					//������̲����û����������
	TVT_MSG_ACK_NETDISK_RELOGIN_ERR,				//������̲����ظ���¼
	TVT_MSG_ACK_NETDISK_ADDINDEX_ERR,				//�û���֧�ֵ�������̸�������
	TVT_MSG_ACK_DISK_FORMATING,					    //���ڸ�ʽ�����̡�
	TVT_MSG_ACK_DISK_FORMAT_FAIL,                   //��ʽ��ʧ��	
	TVT_MSG_ACK_CHANGEDISK_PROPERTY_ERR,            //�޸Ĵ�������ʧ�ܡ�
	TVT_MSG_ACK_CHANGE_GROUP_ERR,                   //�������ԭ���޸�Ⱥ��Ҫ��ѭ�����Ҫ������Ҫ��һ��Ⱥ�飻Ϊÿ�����̶�������Ⱥ�飻Ϊÿ��ͨ����������Ⱥ�顣
	TVT_MSG_ACK_GROUP_HAVE_BE_CHANGE,               //Ⱥ���б仯�����ȱ���Ⱥ�顣
	TVT_MSG_ACK_DISK_SPACE_ERR,                     //��������Ҫ����10G��
	TVT_MSG_ACK_INVALID_PATH,                       //��Ч·��
	TVT_MSG_ACK_NO_ENOUGH_SPACE,                    //���㹻���̿ռ�
	TVT_MSG_ACK_DISK_READING,                       //�������ڱ��ݻ�ط���
	TVT_MSG_ACK_REMOVEDISK_WRITE_ERR,				//���ƶ�����(U��)��������,�����
	TVT_MSG_ACK_UPGRADE_NO_SPACE,                   //�����õ�U��ʣ������Ҫ����500M����U��֧�ֿɶ�д
	TVT_MSG_ACK_UPGRADE_FILE_ERR,                   //��Ч���ļ�
	TVT_MSG_ACK_UPGRADE_FAIL,						//����ʧ��

	TVT_MSG_ACK_CREATE_THREAD_FAIL			= 0x0800,
	TVT_MSG_ACK_ALLOC_MEM_FAIL,
	

	TVT_MSG_ACK_PARAM_ERROR              = 0x1000,	//��������
	TVT_MSG_ACK_UNKNOW_ERROR,						//δ֪����


}TVT_MSG_ACK;
#endif //__TVT_ERROR_H__
