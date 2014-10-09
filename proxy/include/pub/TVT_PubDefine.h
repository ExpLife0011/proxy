/***********************************************************************
** ����ͬΪ����Ƽ����޹�˾��������Ȩ����
** ����			: Ԭʯά
** ����         : 2012-03-22
** ����         : TVT_PubDefine.h
** �汾��		: 1.0
** ����			:
** �޸ļ�¼		:
***********************************************************************/
#ifndef __TVT_PUB_DEFINE_H__
#define __TVT_PUB_DEFINE_H__

#ifdef WIN32
#include <afxwin.h>
#include <io.h>
#include <winsock2.h>
#include "windows.h"
#include "time.h"
#include "winbase.h"
#include <ws2tcpip.h>
#else //////Linux����
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <pthread.h> 
#include <time.h> 
#include <signal.h>
#include <fcntl.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#endif

#include <assert.h> 
#include <iostream>
#include <fstream>
#include <string.h>
#include <list>
#include <map>

#include "TVT_Product.h"

#include "TVT_DevDefine.h"

using namespace std;

#pragma pack(4)
//////////////////////////////////////////////////////////////////////////

typedef enum _tvt_encryption_type_
{
    TVT_ENCRYPTION_NONE,
    TVT_ENCRYPTION_SSL,
    TVT_ENCRYPTION_TLS
}TVT_ENCRYPTION_TYPE;

typedef enum _tvt_date_mode_
{
	TVT_DATE_MODE_YYMMDD	= 0x01,		//��-��-��ģʽ
	TVT_DATE_MODE_MMDDYY	= 0x02,		//��-��-��ģʽ
	TVT_DATE_MODE_DDMMYY	= 0x03,		//��-��-��ģʽ
	TVT_DATE_MODE_Y_M_D	= 0x04,		//��/��/��ģʽ
	TVT_DATE_MODE_M_D_Y	= 0x05,		//��/��/��ģʽ
	TVT_DATE_MODE_D_M_Y	= 0x06,		//��/��/��ģʽ
	TVT_DATE_MODE_YMD		= 0x07,		//��.��.��ģʽ
	TVT_DATE_MODE_MDY		= 0x08,		//��.��.��ģʽ
	TVT_DATE_MODE_DMY		= 0x09		//��.��.��ģʽ
}TVT_DATE_MODE;

typedef enum _tvt_time_mode_
{
	TVT_TIME_MODE_12		= 0x01,		//12Сʱ��
	TVT_TIME_MODE_24		= 0x02,		//24Сʱ�� Ĭ��
}TVT_TIME_MODE;

//�¼�����
typedef enum _tvt_event_type_
{
	TVT_EVENT_MOTION		= 0x01,			//�ƶ����
    TVT_EVENT_PIR        = 0x02,          //�����Ӧ
    TVT_EVENT_SENSOR		= 0x03,			//����������
    TVT_EVENT_SHELTER		= 0x04,			//�ڵ�����
    TVT_EVENT_VLOSS      =  0x05,			//��Ƶ��ʧ
	TVT_EVENT_BEHAVIOR	= 0x06,		    //���ܱ���
}TVT_EVENT_TYPE;


typedef enum _tvt_alarm_type_
{
	TVT_ALARM_MOTION =0 ,		//�ƶ���ⱨ��
    TVT_ALARM_PIR,           //�����Ӧ����
    TVT_ALARM_SENSOR,			//����������
	TVT_ALARM_SHELTER,		//�ڵ�����
	TVT_ALARM_VLOSS,			//��Ƶ��ʧ
    TVT_ALARM_BEHAVIOR,	    //���ܱ���
}TVT_ALARM_TYPE;

typedef enum _tvt_generate_log_type_
{
	TVT_GENERATE_LOG_USER   = 0x01,
	TVT_GENERATE_LOG_CAMERA = 0x02,
	TVT_GENERATE_LOG_DEVICE = 0x03,
}TVT_GENERATE_LOG_TYPE; //������־����,��˭��������־

//��λ���ֽڶ�ӦcontentID����λ���ֽڶ�Ӧtype��
typedef enum _tvt_log_type_
{
	//������Alarm��
	TVT_LOG_ALARM			= 0x00010000,
	TVT_LOG_MOTION_START,				//�ƶ���⿪ʼ
	TVT_LOG_MOTION_END,				//�ƶ�������
    TVT_LOG_PIR_START,				//�����Ӧ������ʼ
    TVT_LOG_PIR_END,				    //�����Ӧ��������
	TVT_LOG_SHELTER_START,			//�ڵ�������ʼ
	TVT_LOG_SHELTER_END,				//�ڵ���������
	TVT_LOG_BEHAVIOR_START,			//���ܱ�����ʼ
	TVT_LOG_BEHAVIOR_END,				//���ܱ�������
	TVT_LOG_ALARM_IN_START,			//�������뿪ʼ
	TVT_LOG_ALARM_IN_END,				//�����������
}TVT_LOG_TYPE;

typedef  enum _tvt_log_ext_id_
{
	//������Alarm��
	TVT_LOG_ID_ALARM		= 0x00010000,   //TVT_LOG_ALARM
	TVT_LOG_ID_MOTION_START,		        //�ƶ���⿪ʼ			TVT_LOG_MOTION_START,			TVT_LOG_EXT_MOTION
	TVT_LOG_ID_MOTION_END,					//�ƶ�������			TVT_LOG_MOTION_END,				TVT_LOG_EXT_MOTION

    TVT_LOG_ID_SHELTER_START,				//�ڵ�������ʼ			TVT_LOG_SHELTER_START,			TVT_LOG_EXT_SHELTER
	TVT_LOG_ID_SHELTER_END,					//�ڵ���������			TVT_LOG_SHELTER_END,			TVT_LOG_EXT_SHELTER
	TVT_LOG_ID_BEHAVIOR_START,				//���ܱ�����ʼ			TVT_LOG_BEHAVIOR_START,			none
	TVT_LOG_ID_BEHAVIOR_END,				//���ܱ�������			TVT_LOG_BEHAVIOR_END,			none
	TVT_LOG_ID_ALARM_IN_START,				//�������뿪ʼ			TVT_LOG_ALARM_IN_START,			TVT_LOG_EXT_SENSOR
	TVT_LOG_ID_ALARM_IN_END,				//�����������			TVT_LOG_ALARM_IN_END,			TVT_LOG_EXT_SENSOR
	TVT_LOG_ID_ALARM_OUT_START,				//���������ʼ			TVT_LOG_ALARM_OUT_START,		TVT_LOG_EXT_ALARM_OUT
	TVT_LOG_ID_ALARM_OUT_END,				//�����������			TVT_LOG_ALARM_OUT_END,			none

}TVT_LOG_EXT_ID;

//ֻ�ܴ��㿪ʼ�Ų��������м丳ֵ��������
typedef enum _tvt_language_
{
	//����
	TVT_LANGUAGE_CHINESE_S=0,			//��������
	TVT_LANGUAGE_CHINESE_B,			//��������
	TVT_LANGUAGE_JAPANESE,			//�ձ�
	TVT_LANGUAGE_KOREA,				//����

	//������
	TVT_LANGUAGE_INDONESIA,			//ӡ��������
	TVT_LANGUAGE_THAI,				//̩��
	TVT_LANGUAGE_VIETNAMESE,		//Խ��

	//ŷ��
	TVT_LANGUAGE_ENGLISH_US,		//Ӣ�ģ�������
	TVT_LANGUAGE_ENGLISH_UK,		//Ӣ�ģ�Ӣ����
	TVT_LANGUAGE_PORTUGUESE,		//������
	TVT_LANGUAGE_GREECE,			//ϣ��
	TVT_LANGUAGE_SPANISH,			//������
	TVT_LANGUAGE_RUSSIAN,			//����	
	TVT_LANGUAGE_NORWAY,			//Ų��
	TVT_LANGUAGE_ITALY,				//�����
	TVT_LANGUAGE_CZECH,				//�ݿ�
	TVT_LANGUAGE_GERMAN,			//����
	TVT_LANGUAGE_DUTCH,				//����
	TVT_LANGUAGE_FINNISH,			//����
	TVT_LANGUAGE_SWEDISH,			//��ʿ
	TVT_LANGUAGE_DANISH,			//����
	TVT_LANGUAGE_FRENCH,			//����
	TVT_LANGUAGE_POLISH,			//����
	TVT_LANGUAGE_BULGARIAN,			//��������
	TVT_LANGUAGE_HUNGARY,			//������

	//�ж�
	TVT_LANGUAGE_HEBREW,			//ϣ����
	TVT_LANGUAGE_TURKEY,			//������
	TVT_LANGUAGE_PERSIAN,			//��˹��
	TVT_LANGUAGE_ARABIC,			//��������
	TVT_LANGUAGE_MAX_COUNT,
}TVT_LANGUAGE;


//���ֻ�ܶ���6��
typedef enum _tvt_default_color_type_
{
	TVT_DEFAULT_COLOR_STANDARD	= 0,
	TVT_DEFAULT_COLOR_INDOOR,
	TVT_DEFAULT_COLOR_LOW_LIGHT,
	TVT_DEFAULT_COLOR_OUTDOOR,
	TVT_DEFAULT_COLOR_REVS1,
	TVT_DEFAULT_COLOR_REVS2,

	TVT_DEFAULT_COLOR_AUTO		= 0x80000000,	//�Զ��л�
	TVT_DEFAULT_COLOR_CHANGE,					//��ҹת��
	TVT_DEFAULT_COLOR_DAY,						//����
	TVT_DEFAULT_COLOR_NIGHT,					//ҹ��
	TVT_DEFAULT_COLOR_OFF		= 0xFFFFFFFF,	//�ر��龰ģʽ
}TVT_DEFAULT_COLOR_TYPE;

typedef enum _tvt_nic_type_
{
	TVT_NIC_TYPE_10M_HALF_DUPIEX					= 0x0001,
	TVT_NIC_TYPE_10M_FULL_DUPIEX					= 0x0002,
	TVT_NIC_TYPE_100M_HALF_DUPIEX					= 0x0004,
	TVT_NIC_TYPE_100M_FULL_DUPIEX					= 0x0008,	
	TVT_NIC_TYPE_1000M_FULL_DUPIEX					= 0x0010,
	TVT_NIC_TYPE_10_100_1000M_AUTO_NEGOTIATION		= 0x0020,
	TVT_NIC_TYPE_WIFI								= 0x0040,
	TVT_NIC_TYPE_BLUETOOTH						= 0x0080
}TVT_NIC_TYPE;

//////////////////////////////////////////////////////////////////////////

typedef struct _tvt_device_param_
{
	/*
	Ӳ���ӿ�
	*/
	unsigned char	localAudioNum;			//������Ƶ������Ŀ
	unsigned char	localVideoInNum;		//������Ƶ������Ŀ
	unsigned char	lineInNum;				//����������Ŀ���������ʾʹ�õ�һͨ����Ƶ�Խ���
	unsigned char	lineOutNum;				//���������Ŀ

	unsigned char	localSensorInNum;		//���ش�������Ŀ
	unsigned char	localRelayOutNum;		//���ؼ̵��������Ŀ
	unsigned char	diskCtrlNum;			//���ش��̽ӿ���Ŀ
	unsigned char	eSataNum;				//����E-sata�ӿ���Ŀ

	unsigned char	networkPortNum;			//����ӿ���Ŀ
	unsigned char	rs485Mode;              //0-��˫����1ȫ˫����Ŀǰû����
	unsigned char	rs232Num;				//RS232�ӿ���Ŀ
	unsigned char	rs485Num;				//RS485�ӿ���Ŀ

	unsigned char	hdmiNum;				//HDMI�ӿ���Ŀ
	unsigned char	usbNum;					//USB�ӿ���Ŀ

	/*����ָ��*/
	unsigned char	netVideoInNum;  		//������Ƶ������Ŀ
	unsigned char	netSensorInNum;			//���紫������Ŀ
	unsigned char	netRelayOutNum;			//����̵��������Ŀ
	unsigned char	maxOnlineUserNum;		//֧�ֵ���������û���Ŀ��ֻ����
	
	/*
	�豸������Ϣ
	*/
	unsigned int	deviceID;									//�豸ID
	unsigned int	videoFormat;								//��Ƶ��ʽ
	unsigned int	deviceLanguage;								//ϵͳ����

	char			mac[8];										//MAC��ַ
	char			deviceName [TVT_MAX_TITLE_LEN+4];			//�豸����
	
	char			deviceSN[TVT_MAX_TITLE_LEN+4];				//�豸���к�
	char			firmwareVersion [TVT_MAX_TITLE_LEN+4];		//�̼��汾
	char			firmwareBuildDate [TVT_MAX_TITLE_LEN+4];	//�̼���������
	char			hardwareVersion [TVT_MAX_TITLE_LEN+4];		//Ӳ���汾
	char			kernelVersion [TVT_MAX_TITLE_LEN+4];		//�ں˰汾
	char			mcuVersion [TVT_MAX_TITLE_LEN+4];			//��Ƭ������汾

	unsigned char	 language[TVT_MAX_LANGUAGE_NUM];				//����֧��	
	char			productionDate[TVT_MAX_TITLE_LEN+4];		//��������
	char			manufacturer[TVT_MAX_TITLE_LEN+4];			//��������
}TVT_DEVICE_PARAM;


typedef struct _tvt_system_version_
{
	char			deviceSN[TVT_MAX_TITLE_LEN+4];				//�豸���к�
	char			firmwareVersion [TVT_MAX_TITLE_LEN+4];		//�̼��汾
	char			firmwareBuildDate [TVT_MAX_TITLE_LEN+4];	//�̼���������
	char			hardwareVersion [TVT_MAX_TITLE_LEN+4];		//Ӳ���汾
	char			kernelVersion [TVT_MAX_TITLE_LEN+4];		//�ں˰汾
	char			mcuVersion [TVT_MAX_TITLE_LEN+4];			//��Ƭ������汾
}TVT_SYSTEM_VERSION;

typedef struct _tvt_network_status_
{
	unsigned short		bSupportNTP;
	unsigned short		bEnableNTP;

	unsigned short		httpPort;
	unsigned short		dataPort;

	unsigned int		ethNum;
	
	struct  
	{
		unsigned short		recv;
		unsigned short		port;
		char				IP[40];					//�ಥ��ַ������IPV4��IPV6��
	}multiCast;

	struct  
	{
		char			MAC [8];					//�����ַ
		unsigned int	nicStatus;					//����״̬: 0,connect;1,disconnect.

		struct  
		{
			unsigned short	enable;					//�Ƿ�����
			unsigned short	getIPMode;				//��ȡ��ַ�ķ�ʽ��0-DHCP��1-��̬��ַ��2-PPPoE

			char			IP[16];					//�����ַ
			char			subnetMask[16];			//��������
			char			gateway[16];			//����

			char	preferredDNS[16];				//������������������ַ
			char	alternateDNS[16];				//������������������ַ
		}IPv4;

		struct  
		{
			unsigned short	enable;					//�Ƿ�����
			unsigned short	getIPMode;				//��ȡ��ַ�ķ�ʽ��0-DHCP��1-��̬��ַ��2-PPPoE

			unsigned int	subnetMask;
			char			IP[40];		
			char			gateway[40];

			char	preferredDNS[40];			//������������������ַ
			char	alternateDNS[40];			//������������������ַ
		}IPv6;

	}ethernet[TVT_MAX_ETH_NUM];

}TVT_NETWORK_STATUS;

#pragma pack()

//////////////////////////////////////////////////////////////////////////
// const unsigned int		TVT_ONE_DAY_SECOND_NUM		= 86400;
// const unsigned int		TVT_LOCAL_DEVICE_ID			= 0;
// const unsigned int		TVT_INVALID_NET_DEVICE_ID	= (~0x0);
// const unsigned int		TVT_LOCAL_CLIENT_ID			= TVT_LOCAL_DEVICE_ID;
// const unsigned int		TVT_INVALID_CLIENT_ID		= (~0x0);
// const unsigned short	TVT_HOLDTIME_ALWAYS			= (~0x0);
//////////////////////////////////////////////////////////////////////////
#endif //__TVT_PUB_DEFINE_H__

