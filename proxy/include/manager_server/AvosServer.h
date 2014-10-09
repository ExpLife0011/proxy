
/***********************************************************
�����ṩ�豸��ͻ�����AVOS����ƽ̨��ȡ�豸��Ϣ�ľ���ʵ�֡�
************************************************************/

#ifndef _AVOS_SERVER_H_
#define _AVOS_SERVER_H_

#include "ManagerServer.h"

//const char AVOS_ACTION_URL[] = "https://pilive.avosapps.com/action";
//const char AVOS_ACTION_URL[] = "http://dev.pilive.avosapps.com/action";
const char AVOS_ACTION_URL[] = "http://192.168.2.160:3000/action";

class CAvosServer : public CManagerServer
{
public:
	CAvosServer(void);
	~CAvosServer(void);

	static CAvosServer *Instance();

	virtual bool SaveInfo();
	virtual bool ReadInfo();

	virtual int CheckAuth(const std::string &account, const std::string &password);
	virtual int BindOwner(const std::string &strAccount, const std::string &password);

private:
	char			m_authCode[28];

};

#endif
