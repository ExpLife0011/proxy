// NatDebug.h: interface for the Debug.
//
//////////////////////////////////////////////////////////////////////

#ifndef _NAT_DEBUG_H_
#define _NAT_DEBUG_H_

/**
 * ÿ��ʵ��(.cpp)�ļ����������ø�NatDebug.h���������һ�����ã�#include)��
 */



#ifdef _DEBUG
#ifdef __ENVIRONMENT_WIN32__
#ifdef DEBUG_NEW
 #define new DEBUG_NEW
 
#endif//DEBUG_NEW
#endif//__ENVIRONMENT_WIN32__
#endif//_DEBUG

#endif//_NAT_DEBUG_H_