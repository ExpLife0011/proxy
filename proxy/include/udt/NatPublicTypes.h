// NatPublicTypes.h: interface for the public types of nat.
//
//////////////////////////////////////////////////////////////////////
#ifndef __NAT_PUBLIC_TYPES_H__
#define __NAT_PUBLIC_TYPES_H__

#include "NatUserDefine.h"

#ifdef NAT_TVT_DVR_4_0
#include "TVT_PubCom.h"
#else
#include "PUB_common.h"
#endif//NAT_TVT_DVR_4_0

#include "SWL_Public.h"
/**
 * ��͸��������Ϣ����Ҫ��Ϊ��͸�������б���
 */
typedef struct __nat_server_info__
{
    char                szAddr[64];     // ��������ַ
    unsigned short      sPort;          // ��͸�˿ں�
    unsigned short      sTestPort;      // ��͸���Զ˿ں� 
}NAT_SERVER_INFO;


#endif//__NAT_PUBLIC_TYPES_H__