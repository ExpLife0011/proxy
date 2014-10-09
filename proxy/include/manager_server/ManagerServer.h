
#ifndef _MANAGER_SERVER_H_
#define _MANAGER_SERVER_H_

#include <string>
#include <map>

/***********************************************************
�����ṩ�豸��ͻ��������ƽ̨��ȡ�豸��Ϣ�Ľӿڡ�
����ƽ̨�Ķ����ԣ�����˶Բ�ͬƽ̨�в�ͬ�ľ����ȡ��ʽ��
����������ṩͳһ�Ĵ�ȡ�ӿڣ�����ƽ̨�Ĵ�ȡ��Ҫ�������Ļ����Ͼ��廯��
************************************************************/
typedef std::map<std::string, std::string> MS_INFO_MAP;
class CManagerServer
{
public:
	CManagerServer();
	virtual ~CManagerServer();

	//��ʼ���������豸�����к�
	virtual bool Initial(std::string strDeviceSN);
	virtual void Quit();

	//�����豸��������ַ���˿���Ϣ������ֻ���ݴ��ڱ��أ���û�д浽���������
	virtual bool SetNetInfo(const std::string &strIP, const unsigned short &nPort);
	//�����豸��������Ϣ��ÿ����Ϣ����Ϣ������Ϣֵ��ɣ���Ϣֵ�޶�Ϊ�ַ���
	virtual bool SetInfo(const std::string &strInfoName, const std::string &strInfoValue);

	//��ȡ�豸��������ַ���˿���Ϣ�����ﲻֱ�Ӵӷ�����ȡ�����Ǵ������Ķ�����ȡ
	virtual bool GetNetInfo(std::string &strIP, unsigned short &nPort);
	//��ȡָ�����Ƶ��豸��Ϣ
	virtual bool GetInfo(const std::string &strInfoName, std::string &strInfoValue);

	//���浽������
	virtual bool SaveInfo() = 0;
	//�ӷ�������ȡ
	virtual bool ReadInfo() = 0;

	//////////////////////////////////////////////////////////////////////////
	//����Ϊ������������û��豸������صĽӿ�

	//���ָ���û��Ը��豸�ķ���Ȩ�ޣ�����-1��ʾû��Ȩ�ޣ�0��ʾ����ԱȨ�ޣ�����0��ʾ���޵�Ȩ��
	virtual int CheckAuth(const std::string &strAccount, const std::string &password){return -1;}
	virtual int BindOwner(const std::string &strAccount, const std::string &password){return -1;}

protected:
	std::string		m_strDeviceSN;
	std::string		m_strIP;
	unsigned short	m_nPort;
	MS_INFO_MAP		m_msInfo;
};

#endif