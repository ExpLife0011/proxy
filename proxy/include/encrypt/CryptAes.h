
/************************************************************************
***		AES�ԳƼ����㷨

***		AES�����ص㣺
*		1����ȫ�ȼ�����DES
*		2��������ܣ�Ҳ��Ϊ�����
*		3����5�ּ���ģʽ��ECB��CBC��CFB��OFB��CTR

***		�������Ӧ����㷺��CBC����ģʽ��SSLҲ�õ����ּ���ģʽ
*************************************************************************/

#ifndef _CRYPT_AES_H_
#define _CRYPT_AES_H_

class CCryptAes
{
public:
	CCryptAes(void);
	~CCryptAes(void);

	//�������128λ��Կ
	static void GenerateKey(unsigned char *pKey, int keyLen);

	//������Կ���ӽ��ܶ��������Կ��userKeyΪ��Կ��bitsΪ��Կλ��
	void SetCryptKey(const unsigned char *userKey, const int bits);

	//��������pPlainData, lengthΪ���ĵĳ���,��Ҫ����16������;
	//���ܺ������pEncryptData���ȱ���Ϊ16�ֽڵ�������
	bool Encrypt(const unsigned char *pPlainData, int length, unsigned char *pEncryptData);

	//��������pEncryptData, lengthΪ���ܺ�����ĵĳ���,��Ҫ�󳤶���16��������
	bool Decrypt(const unsigned char *pEncryptData, int length, unsigned char *pPlainData);

private:
	unsigned char			m_userKey[32];					//��� 32 * 8 = 256 bit
	int						m_keyBits;						//��Կλ��
};

#endif
