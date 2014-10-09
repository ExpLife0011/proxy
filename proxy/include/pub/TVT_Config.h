/***********************************************************************
** Copyright (C) Tongwei Video Technology Co.,Ltd. All rights reserved.
** Author       : YSW
** Date         : 2012-03-23
** Name         : TVT_Config.h
** Version      : 1.0
** Description  :	����DVR/DVS��ص�һЩ��������
** Modify Record:
1:����
***********************************************************************/
#ifndef __TVT_CONFIG_H__
#define __TVT_CONFIG_H__
#include "TVT_PubDefine.h"
#include "TVT_TimeZone.h"

#pragma pack(4)

typedef enum _tvt_dd_config_item_id_
{
	TVT_CONFIG_ITEM_GUI				= 0x0000,
	TVT_CONFIG_ITEM_GUI_CONFIG,				//TVT_GUI_CONFIG
	TVT_CONFIG_ITEM_STATUS_BK,				//TVT_SYSTEM_STATUS_BACKUP
	TVT_CONFIG_ITEM_GUI_END,

	TVT_CONFIG_ITEM_SYSTEM_BASE		=0x0100,
	TVT_CONFIG_ITEM_VIDEO_OUT,				//TVT_VIDEO_OUT_CONFIG
	TVT_CONFIG_ITEM_DEVICE_CONFIG,			//TVT_DEVICE_CONFIG
	TVT_CONFIG_ITEM_DATE_TIME,				//TVT_DATE_TIME_CONFIG
	TVT_CONFIG_ITEM_SYSTEM_OTHER,			//TVT_BASIC_OTHER
	TVT_CONFIG_ITEM_ALLOW_DENY_IP,			//TVT_ALLOW_DENY_IP
	TVT_CONFIG_ITEM_DISK_CONFIG,			//TVT_DISK_CONFIG
	TVT_CONFIG_ITEM_SYSTEM_END,

	TVT_CONFIG_ITEM_LIVE_BASE		= 0x0200,
	TVT_CONFIG_ITEM_LIVE_CHNN,				//TVT_LIVE_CHNN_CONFIG
	TVT_CONFIG_ITEM_LIVE_DISPLAY,			//TVT_LIVE_DISPLAY_CONFIG
	TVT_CONFIG_ITEM_LIVE_END,



	//
	TVT_CONFIG_ITEM_END
}TVT_CONFIG_ITEM_ID;


typedef struct _tvt_work_mode_info_
{
    bool   bEnableAlarm;  //��������  0:�رգ�1����
    bool   bEnablePush;   //���Ϳ���  0:�رգ�1����
    bool   bEnableAlarmrecord;  //����¼�񿪹�  0:�رգ�1����
}TVT_WORK_MODE_INFO;

typedef struct _tvt_work_mode_
{
    int  workMode; // 0:�ڼ�ģʽ 1:���ģʽ
    TVT_WORK_MODE_INFO  atHomeMode;      //�ڼ�ģʽ
    TVT_WORK_MODE_INFO  leaveHomeMode;   //���ģʽ
}TVT_WORK_MODE;

typedef  struct  _tvt_user_info_
{
    char masterName[64];       //��������
    char masterPassword[64];   //��������
}TVT_USER_INFO; 

typedef struct _tvt_privacy_protection_
{
    int   enable;   //��˽���� 0:������1:�ر�
}TVT_PRIVACY_PROTECTION;

typedef  struct  _tvt_wireless_info_
{
    char  wifiSsid[64];        //wifi SSID
    char  wifipassword[132];   //wifi ����
}TVT_WIRELESS_INFO;

typedef struct  _tvt_network_ip_
{
	char			MAC[8];					    //�����ַ��ֻ����
	unsigned short  nicTypeMask;                //֧�ֵ������������루ֻ������Ӧ�ṹ��TVT_NIC_TYPE
	unsigned short  supportIPV6;                //�Ƿ�֧��IPV6(ֻ��)
	
	unsigned short  nicType;                    //��������
	unsigned short  enable;                     //�����Ƿ����á�0-�����ã�1-����,

	unsigned int 	nicStatus;					//����״̬: 0,connect;1,disconnect.	
	
	//WiFi�������ڣ�TVT_NIC_TYPE_WIFI == nicType��ʱ��Ч
	struct  
	{
		unsigned int	channel;				//�ŵ�
		unsigned int	authMode;				//��֤ģʽ��TVT_WIFI_AUTH_MODE
		unsigned int	encrypType;				//���ܷ�ʽ��TVT_WIFI_ENCRYPTION_TYPE
		unsigned int	defaultKeyID;			//������

		unsigned int	rate;					//Ƶ��
		unsigned int	mode;					//ģʽ

		char			SSID[36];				//SSID(strlen() <= 32)
		char			key[132];				//�������루strlen() <= 128��
	}wifi;

	struct  
	{
		unsigned short	recv;
		unsigned short	useDHCP;				//�Ƿ�ʹ�ö�̬�����ַ

		char			IP[16];					//�����ַ
		char			subnetMask[16];			//��������
		char			gateway[16];			//����

		char	preferredDNS[16];				//������������������ַ
		char	alternateDNS[16];				//������������������ַ
	}IPv4;

	/*IPV6��IP��ַ���ܹ�16���ֽڣ�������ʾ��ʽΪX:X:X:X:X:X:X:X��
	X��16���Ʊ�ʾ��ÿ��Xռ4λ16���ƣ�Ҳ���������ֽڣ��ܹ�8��X������ʾ16���ֽڵ�IP��ַ
	IPV6���������룬�ó�������ʾ��ȡֵΪ1��127������ȡ64����IP��ַ��ǰ64λΪ�����
	IPV6�����ء���IPV6��IP��ַ���ֽ�������ʾ��ʽһ��*/
	struct  
	{
		unsigned short	recv;
		unsigned short	useDHCP;				//�Ƿ�ʹ�ö�̬�����ַ

		unsigned int	subnetMask;
		char			IP[40];		
		char			gateway[40];

		char	preferredDNS[40];				//������������������ַ
		char	alternateDNS[40];				//������������������ַ
	}IPv6;

	struct  
	{
		unsigned short	supportPPPoE;			//�Ƿ�֧��PPPoE���ܣ�ֻ����
		unsigned short	usePPPoE;				//�Ƿ�ʹ��PPPoE
		char account[TVT_MAX_ACCOUNT_LEN+4];	//PPPoE���ʺ�
		char password[TVT_MAX_PASSWORD_LEN+4];	//PPPoE������
	}PPPoE;
}TVT_NETWORK_IP;

typedef struct _tvt_network_ip_config_
{
	unsigned char	recv;
	unsigned char	bLock;				//�Ƿ�����

	unsigned char	bSupportNAT;		//�Ƿ�֧�����紩͸��ֻ����
	unsigned char	ethNum;				//��Ч��������(ֻ��)
	unsigned char	bSupportPush;		//�Ƿ�֧���ƶ�(ֻ����

	unsigned char	bEnableCellular;	//�Ƿ����ƶ��������ݣ�3G/4G��
	unsigned char	bEnableNAT;			//�������紩͸
	unsigned char	bEnablePush;		//�Ƿ�������

	TVT_NETWORK_IP	network[TVT_MAX_ETH_NUM];
}TVT_NETWORK_IP_CONFIG;

#pragma pack()
#endif //__TVT_CONFIG_H__
