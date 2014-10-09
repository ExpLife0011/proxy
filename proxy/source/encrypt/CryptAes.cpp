
#include "TVT_PubDefine.h"
#include "CryptAes.h"
#include "openssl/aes.h"
#include "openssl/rand.h"
#include "TVT_PubCom.h"

CCryptAes::CCryptAes(void)
{
	m_keyBits = 128;
}

CCryptAes::~CCryptAes(void)
{
}

void CCryptAes::GenerateKey(unsigned char *pKey, int keyLen)
{
	assert(NULL != pKey);
	assert(keyLen >= 16);

	char seed[4];
	TVT_DATE_TIME dateTime = GetCurrTime();
	memcpy(seed, &dateTime.microsecond, 3);
	unsigned long tickCount = PUB_GetTickCount();
	memcpy(seed+3, &tickCount, 1);
	RAND_seed(seed, 4);

	int ret = RAND_bytes(pKey, 16);

	if(ret != 1)
	{
		assert(false);
	}
}

//������Կ���ӽ��ܶ��������Կ��userKeyΪ��Կ��bitsΪ��Կλ��
void CCryptAes::SetCryptKey(const unsigned char *userKey, const int bits)
{
	memcpy(m_userKey, userKey, bits/8);
	m_keyBits = bits;
}

//��������pData, ���ݳ���Ϊlength
bool CCryptAes::Encrypt(const unsigned char *pPlainData, int length, unsigned char *pEncryptData)
{
	assert(pPlainData != NULL);
	assert(pEncryptData != NULL);

	AES_KEY aes;
	unsigned char iv[AES_BLOCK_SIZE];
	memset(iv, 0, sizeof(iv));

	if (AES_set_encrypt_key(m_userKey, m_keyBits, &aes) < 0)
	{
		return false;
	}

	AES_cbc_encrypt(pPlainData, pEncryptData, length, &aes, iv, AES_ENCRYPT);

	return true;
}

//��������pData, ���ݳ���Ϊlength
bool CCryptAes::Decrypt(const unsigned char *pEncryptData, int length, unsigned char *pPlainData)
{
	assert(pPlainData != NULL);
	assert(pEncryptData != NULL);

	AES_KEY aes;
	unsigned char iv[AES_BLOCK_SIZE];
	memset(iv, 0, sizeof(iv));

	if (AES_set_decrypt_key(m_userKey, m_keyBits, &aes) < 0)
	{
		return false;
	}

	AES_cbc_encrypt(pEncryptData, pPlainData, length, &aes, iv, AES_DECRYPT);

	return true;
}
