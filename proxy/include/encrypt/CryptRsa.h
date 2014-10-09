
/************************************************************************
***		RSA�ǶԳƼ����㷨

***		RSA�����ص㣺
*		1���㷨�Ƚϼ򵥣���ͬ�Ȱ�ȫ�����£���DES���ܵ��㷨Ҫ��1000��
*		2���㷨����˼·��
			��������p��q 
			ȡn=p*q 
			ȡt=(p-1)*(q-1) 
			ȡ�κ�һ����e,Ҫ������eȡd*e%t==1 
			�������յõ��������� n d e 
			
			��������ȡnΪ��Կ��dΪ˽Կ��e����ȡһ��

***		�������Ӧ����㷺��CBC����ģʽ��SSLҲ�õ����ּ���ģʽ
*************************************************************************/
#ifndef _CRYPT_RSA_H_
#define _CRYPT_RSA_H_

#include "openssl/rsa.h"

class CCryptRsa
{
public:
	CCryptRsa(void);
	~CCryptRsa(void);

	//bServerΪtrue��ʾ����Կ�����ɷ�����ʱ��Ҫ�����ֶ�ΪkeyBits,keyBitsȡֵҪ����1024����
	//bServerΪfalse��ʾҪ�������ݷ�����Ҫ����keyBits��publickey����������
	bool Initial(bool bServer, int keyBits = 1024, const unsigned char *pPublicKey = NULL);

	//������Կ���ɷ�����Ҫ�ѹ�Կȡ���������Է�,������ڴ��С��Ҫ��С��keyBits/8���ֽ�
	bool GetPublicKey(unsigned char *pPublicKey, int keyLen);

	//���ܺ�����ݳ���
	int GetEncryptSize();

	//��Կ���ܣ�dataLen <= GetEncryptSize() - 11
	//����ֵΪ���ܺ�ĳ��ȣ�-1��ʾ����ʧ��
	int PublicEncryt(unsigned char *pPlainData, int dataLen, unsigned char *pEncryptData);

	//˽Կ����,dataLen == GetEncryptSize()
	//����ֵΪ���ܺ�����ĳ��ȣ�-1��ʾ����ʧ��
	int PrivateDecrypt(unsigned char *pEncryptData, int dataLen, unsigned char *pPlainData);

	void Quit();

private:
	bool			m_bServer;
	int				m_keyBits;
	RSA				*m_pRsa;
	int				m_rsaSize;
};

#endif
