#ifndef __TVT_DEV_DEFINE_H__
#define __TVT_DEV_DEFINE_H__


#ifdef WIN32
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

#pragma pack(4)

typedef enum _tvt_video_format_
{
    TVT_VIDEO_FORMAT_NTSC		= 0x01,
    TVT_VIDEO_FORMAT_PAL		= 0x02,
}TVT_VIDEO_FORMAT;

typedef enum _tvt_encode_type_
{
    TVT_ENCODE_TYPE_CBR,
    TVT_ENCODE_TYPE_VBR
}TVT_ENCODE_TYPE;

typedef enum   __tvt_encode_format__type__
{
   TVT_VIDEO_ENCODE_TYPE_H264  = 0,
   TVT_VIDEO_ENCODE_TYPE_MPEG4 = 1,
   TVT_AUDIO_ENCODE_TYPE_RAW   = 2,
   TVT_AUDIO_ENCODE_TYPE_G711A = 3,
   TVT_AUDIO_ENCODE_TYPE_G711U = 4,
   TVT_AUDIO_ENCODE_TYPE_ADPCM = 5,
   TVT_AUDIO_ENCODE_TYPE_G726  = 6,
   TVT_AUDIO_ENCODE_TYPE_AAC   = 7,
   TVT_AUDIO_ENCODE_TYPE_LPCM  = 8,
   TVT_PIC_ENCODE_TYPE_JPEG    = 9,
}TVT_ENCODE_FORMAT_TYPE;

typedef enum _tvt_stream_type_
{
    TVT_STREAM_TYPE_NONE		    = 0x00,

    TVT_STREAM_TYPE_VIDEO_1		= 0x01,			//��Ƶ1��������Ӧ��������
    TVT_STREAM_TYPE_VIDEO_2		= 0x02,			//��Ƶ2��������Ӧ��������
    TVT_STREAM_TYPE_VIDEO_3		= 0x03,			//��Ƶ3���������ã�
    TVT_STREAM_TYPE_AUDIO		    = 0x04,			//��Ƶ
    TVT_STREAM_TYPE_PICTURE		= 0x05,			//ͼƬ
    TVT_STREAM_TYPE_TEXT		    = 0x06,			//�ı�����
    TVT_FRAME_TYPE_BEGIN,
    TVT_FRAME_TYPE_END
}TVT_STREAM_TYPE;


//��λ���棬��಻�ܳ���32����
typedef enum _tvt_video_size_
{
    TVT_VIDEO_SIZE_QCIF			= 0x0001,	//(NTSC 176 X 120),(PAL 176 X 144)
    TVT_VIDEO_SIZE_CIF			= 0x0002,	//(NTSC 352X 240),(PAL 352 X 288)
    TVT_VIDEO_SIZE_2CIF			= 0x0004,	//(NTSC 704 X 240),(PAL 704 X 288)
    TVT_VIDEO_SIZE_4CIF			= 0x0008,	//(NTSC 704 X 480),(PAL 704 X 576)

    TVT_VIDEO_SIZE_QVGA			= 0x0010,	//(320 X 240)
    TVT_VIDEO_SIZE_VGA			= 0x0020,	//(640 X 480)
    TVT_VIDEO_SIZE_960H			= 0x0040,	//(NTSC 960 X 480),(PAL 960 X 576)
    TVT_VIDEO_SIZE_SVGA			= 0x0080,	//(800 X 600)

    TVT_VIDEO_SIZE_XGA			= 0x0100,	//(1024 X 768)
    TVT_VIDEO_SIZE_XVGA			= 0x0200,	//(1280 X 960)
    TVT_VIDEO_SIZE_SXGA			= 0x0400,	//(1280 X 1024)
    TVT_VIDEO_SIZE_UXGA			= 0x0800,	//(1600 X 1200)

    TVT_VIDEO_SIZE_720P			= 0x1000,	//(1280 X 720)
    TVT_VIDEO_SIZE_1080P		= 0x2000,	//(1920 X 1080)
    TVT_VIDEO_SIZE_QXGA			= 0x4000,	//(2048 X 1536)
    TVT_VIDEO_SIZE_4K			= 0x8000,	//(3840 x 2160)
}TVT_VIDEO_SIZE;


/*��ö�ٵ�ֵ������һ��unsigned short�����ϣ��������ֻ�ܶ���16��*/
typedef enum _tvt_white_balance_
{
    TVT_WHITE_BALANCE_AUTO    = 0x0001,		//�Զ�
    TVT_WHITE_BALANCE_MANUAL  = 0x0002,		//�ֶ�
    TVT_WHITE_BALANCE_LAMP    = 0x0004,		//�׳��
    TVT_WHITE_BALANCE_OUTDOOR = 0x0008,		//����
    TVT_WHITE_BALANCE_INDOOR  = 0x0010,		//����
    TVT_WHITE_BALANCE_CLOUDY  = 0x0020,		//����
    TVT_WHITE_BALANCE_SUNNY   = 0x0040,		//����
}TVT_WHITE_BALANCE;

typedef enum _tvt_net_stream_mode_
{
    TVT_NET_STREAM_SYSTEM,		//ϵͳĬ��
    TVT_NET_STREAM_SHARPNESS,	//������
    TVT_NET_STREAM_FLUENCY,		//������
}TVT_NET_STREAM_MODE;

typedef enum  _tvt_osd_pos_
{
    TVT_OSD_LEFT_UP,            //����
    TVT_OSD_LEFT_DWON,          //����
    TVT_OSD_RIGHT_UP,           //����
    TVT_OSD_RIGHT_DOWN,         //����
}TVT_OSD_POS;

typedef struct _tvt_block_
{
    unsigned char	*pBuffer;			//���Ӧ���ڴ��ַ*/
    unsigned int	bufLength;			//��ǰ��ĳ���*/

    unsigned int	channel;			 //���Ӧ��ͨ����*/
    unsigned int	streamType;			//��Ӧ��������������*/
    unsigned int	dataLength;			//��ŵ����ݳ���*/
}TVT_BLOCK;

typedef struct _tvt_local_time_
{
    short	year;			//��
    short	month;			//��
    short	mday;			//ĳ�µڼ���

    short	hour;			//ʱ
    short	minute;			//��
    short	second;			//��

    int	microsecond;	//΢��
}TVT_LOCAL_TIME;

typedef struct _tvt_date_time_
{
    int	seconds;		//������
    int	microsecond;	//΢��

#ifdef   __cplusplus
    _tvt_date_time_();
    _tvt_date_time_ & operator=(tm & time);
    _tvt_date_time_ & operator=(const timeval & time);
    _tvt_date_time_ & operator=(const _tvt_local_time_ & time);
    tm TM() const;
    timeval TimeVal() const;
    _tvt_local_time_ LocalTime() const;

    _tvt_date_time_ & operator+=(int usec);
    _tvt_date_time_ & operator-=(int usec);

    long long Dec(const _tvt_date_time_ & time);
    int operator-(const _tvt_date_time_ & time);

    bool operator!=(const _tvt_date_time_ & time) const;
    bool operator==(const _tvt_date_time_ & time) const;
    bool operator<(const _tvt_date_time_ & time) const;
    bool operator<=(const _tvt_date_time_ & time) const;
    bool operator>(const _tvt_date_time_ & time) const;
    bool operator>=(const _tvt_date_time_ & time) const;
#endif  //__cplusplus
}TVT_DATE_TIME;

typedef struct _tvt_data_frame_
{
    union
    {
        unsigned int   position;	//�ڻ�����е�λ��
        unsigned char   *pData;		//������ָ�루TVT_DATA_FRAME + DATA��
    };
    unsigned int   length;			//��������С��sizeof(TVT_DATA_FRAME) + frame.length��

    unsigned int	deviceID;		//�豸���
    unsigned int   cameraID;		//��������

    unsigned int	freameID;		//֡���
    unsigned int	streamID;		//�����

    struct  
    {
        unsigned short		width;		//���
        unsigned short		height;		//�߶�

        unsigned short		bKeyFrame;	//�Ƿ�Ϊ�ؼ�֡
        unsigned short		encodeType;	//��������

        unsigned int		length;		//���ݳ���
        unsigned int		streamType;	//�����ͣ���TVT_STREAM_TYPE

        unsigned int		channel;	//ͨ�����

        TVT_LOCAL_TIME		localTime;	//��ʽ��ʱ��
        TVT_DATE_TIME		time;		//��׼ʱ��
    }frame;

    unsigned int	checkBits;      //У��λ

#ifdef __cplusplus
    bool operator!=(const _tvt_data_frame_ & frame) const;
    bool operator==(const _tvt_data_frame_ & frame) const;
#endif //__cplusplus	
}TVT_DATA_FRAME;


#endif