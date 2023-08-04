#define _CRT_SECURE_NO_WARNINGS
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/pem.h>
#include <string>
#include <memory>
#include <openssl/rand.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <openssl/bio.h>
#include "time.h"
#define RED_t "\033[31m"
#define YELLOW_t "\033[33m"
#define GREEN_t "\033[32m"
#define WRITE "\033[0m"

#define errorL(msg) \
	std::cout << RED_t <<"Error:["<< __FILE__  << ":"<< __LINE__ << "]:"<< msg << WRITE <<std::endl;
#define debugL(msg) \
	std::cout << YELLOW_t <<"debug:["<< __FILE__ << ":"<< __LINE__ << "]:"<< msg << WRITE << std::endl;
#define infoL(msg) \
	std::cout << GREEN_t <<"infor:["<< __FILE__ << ":" << __LINE__ << "]:"<< msg << WRITE << std::endl;

#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

std::string GetErrorStr() {
    unsigned long er = 0;

    char erbuf[512] = { 0 };

    size_t erlen = 512;

    er = ERR_get_error();
    ERR_error_string_n(er, erbuf, erlen);
    return std::string(erbuf, erlen);
}
class sm2PrivateKey;

struct EVP_CUNSTOM {
    EVP_PKEY* pkey = NULL;
    ~EVP_CUNSTOM() {
        if (pkey != NULL) {
            EVP_PKEY_free(pkey);
        }
    }
};

class sm2PublicKey {
public:
    sm2PublicKey() = default;

    sm2PublicKey(const sm2PublicKey& other) {
        m_pkey = other.m_pkey;
    }

    sm2PublicKey(const std::string& pub_str);

    sm2PublicKey(const unsigned char* pub_str, size_t len);

    std::string Encrypt(const std::string& message, std::string& error);

    bool SignatureVerification(const std::string& signature, const std::string& message, std::string& error);


    std::string GetPublicString();

    std::string GetPublicStringBase64();
private:

    std::shared_ptr<EVP_CUNSTOM> m_pkey = nullptr;//使用shared_ptr 防止拷贝构造的时候造成内存泄漏和意外释放
};


class sm2PrivateKey
{
public:

    sm2PrivateKey();

    //sm2PrivateKey(const std::string& priv_str);

    sm2PublicKey CreatePublic();

    std::string Decrypt(const std::string& encoded, std::string& error);

    std::string Signature(const std::string& message, std::string& error);

    std::string GetPrivateString();

private:

    std::shared_ptr<EVP_CUNSTOM>  M_PKEY = nullptr;
};

/*生成密钥对*/
sm2PrivateKey::sm2PrivateKey() {
    EVP_PKEY* ret = NULL;

    EVP_PKEY_CTX* pkctx = NULL;

    pkctx = EVP_PKEY_CTX_new_id(EVP_PKEY_SM2, NULL);//创建sm2 上下文

    if (pkctx == NULL) {
        errorL("EVP_PKEY_CTX_new_id");
        return;
    }
    int retV = 1;
    retV = EVP_PKEY_keygen_init(pkctx);//初始化sm2 上下文

    if (retV <= 0) {
        errorL("EVP_PKEY_keygen_init:" << GetErrorStr());
        EVP_PKEY_CTX_free(pkctx);
        return;
    }

    retV = EVP_PKEY_keygen(pkctx, &ret);//生成密钥对
    if (retV <= 0) {
        errorL("EVP_PKEY_keygen:" << GetErrorStr());
        EVP_PKEY_CTX_free(pkctx);
        return;
    }
    EVP_CUNSTOM* cst = new EVP_CUNSTOM{ ret };
    M_PKEY = std::shared_ptr<EVP_CUNSTOM>(cst);
    EVP_PKEY_CTX_free(pkctx);
}

/*导出公钥和导入公钥*/
sm2PublicKey sm2PrivateKey::CreatePublic() {
    unsigned char* buffer = nullptr;
    int retV = i2d_PUBKEY(M_PKEY.get()->pkey, &buffer);//导出
    if (retV <= 0) {
        errorL("i2d_PUBKEY:" << GetErrorStr());
        return sm2PublicKey{};
    }
    //buffer 里的是公钥二进制
    sm2PublicKey pub(buffer, retV);
    //OPENSSL_free(buffer);
    return pub;
}

sm2PublicKey::sm2PublicKey(const unsigned char* pub_str, size_t len) {
    EVP_PKEY* pkey_t = NULL;
    //pkey_t=d2i_PublicKey(EVP_PKEY_SM2,NULL, &pub_str, len);
    pkey_t = d2i_PUBKEY(NULL, &pub_str, len);//导入
    std::string error;
    if (pkey_t == NULL) {
        error = GetErrorStr();
        errorL(error);
        return;
    }
    EVP_CUNSTOM* cst = new EVP_CUNSTOM{ pkey_t };
    m_pkey = std::shared_ptr<EVP_CUNSTOM>(cst);
}

/*公钥加密*/
std::string sm2PublicKey::Encrypt(const std::string& message, std::string& error) {
    EVP_PKEY_CTX* pkctx = NULL;

    if (!(pkctx = EVP_PKEY_CTX_new(m_pkey.get()->pkey, NULL))) {
        error = GetErrorStr();
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }

    int retV = EVP_PKEY_encrypt_init(pkctx);
    if (retV <= 0) {
        error = GetErrorStr();
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }

    size_t outbuflen = 0;
    retV = EVP_PKEY_encrypt(pkctx, NULL, &outbuflen,
        (const unsigned char*)message.c_str(), message.size());

    unsigned char* outbuf = new unsigned char[outbuflen];

    retV = EVP_PKEY_encrypt(pkctx, outbuf, &outbuflen,
        (const unsigned char*)message.c_str(), message.size());

    if (retV <= 0) {
        error = GetErrorStr();
        delete[] outbuf;
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }

    std::string encodedstr = std::string((const char*)outbuf, outbuflen);

    delete[] outbuf;

    EVP_PKEY_CTX_free(pkctx);

    return encodedstr;
}

/*私钥解密*/
std::string sm2PrivateKey::Decrypt(const std::string& encoded, std::string& error) {
    std::string decodedstr;
    EVP_PKEY_CTX* pkctx = NULL;
    unsigned char* outbuf = NULL;
    size_t outlen = 0;

    int retV = 1;
    if (!(pkctx = EVP_PKEY_CTX_new(M_PKEY.get()->pkey, NULL))) {//创建EVP 上下文
        error = GetErrorStr();
        errorL("EVP_PKEY_CTX_new:" << error);
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }
    retV = EVP_PKEY_decrypt_init(pkctx);// 解密初始化
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_PKEY_decrypt_init:" << error);
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }
    retV = EVP_PKEY_decrypt(pkctx, NULL, &outlen,
        (const unsigned char*)encoded.c_str(), encoded.size());//解密
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_PKEY_encrypt_init:" << error);
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }

    if (outlen == 0) {
        errorL("EVP_PKEY_decrypt:" << error);
        EVP_PKEY_CTX_free(pkctx);
        return "";
    }

    outbuf = new unsigned char[outlen];

    retV = EVP_PKEY_decrypt(pkctx, outbuf, &outlen,
        (const unsigned char*)encoded.c_str(), encoded.size());//解密
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_PKEY_encrypt_init:" << error);
        EVP_PKEY_CTX_free(pkctx);
        delete[] outbuf;
        return "";
    }

    decodedstr = std::string((const char*)outbuf, outlen);
    delete[] outbuf;

    EVP_PKEY_CTX_free(pkctx);
    return decodedstr;
}

/*私钥签名*/
std::string sm2PrivateKey::Signature(const std::string& message, std::string& error) {
    std::string signatured;
    EVP_MD_CTX* mdctx = NULL;
    unsigned char* outbuf = NULL;
    size_t outbuflen = 0;
    int retV = 0;
    if (!(mdctx = EVP_MD_CTX_create())) {//创建摘要上下文
        error = GetErrorStr();
        errorL("EVP_MD_CTX_create:" << error);
        return "";
    }
    retV = EVP_DigestSignInit(mdctx, NULL, EVP_sm3(),//使用sm3 摘要算法
        NULL, M_PKEY.get()->pkey);//签名初始化
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_DigestSignInit:" << error);
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    retV = EVP_DigestSignUpdate(mdctx, message.c_str(), message.size());//更新签名内容
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_DigestSignUpdate:" << error);
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    retV = EVP_DigestSignFinal(mdctx, NULL, &outbuflen);//获取签名长度
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_DigestSignFinal:" << error);
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    outbuf = new unsigned char[outbuflen];

    retV = EVP_DigestSignFinal(mdctx, outbuf, &outbuflen);//获取签名结果
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_DigestSignFinal:" << error);
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    signatured = std::string((const char*)outbuf, outbuflen);
    delete[] outbuf;
    return signatured;
}

/*公钥验签*/
bool sm2PublicKey::SignatureVerification(const std::string& signature, const std::string& message, std::string& error) {
    std::string signatured;
    EVP_MD_CTX* mdctx = NULL;
    int retV = 0;
    if (!(mdctx = EVP_MD_CTX_create())) {//创建摘要上下文
        error = GetErrorStr();
        errorL("EVP_MD_CTX_create:" << error);
        return false;
    }
    retV = EVP_DigestVerifyInit(mdctx, NULL, EVP_sm3(), NULL, m_pkey.get()->pkey);//验签初始化
    if (retV <= 0) {
        error = GetErrorStr();
        errorL("EVP_DigestVerifyInit:" << error);
        EVP_MD_CTX_free(mdctx);
        return false;
    }

    retV = EVP_DigestVerifyUpdate(mdctx, message.c_str(), message.size());//更新验签内容
    if (retV <= 0) {
        error = GetErrorStr();
        EVP_MD_CTX_free(mdctx);
        errorL("EVP_DigestVerifyUpdate:" << error);
        return false;
    }
    retV = EVP_DigestVerifyFinal(mdctx, (const unsigned char*)signature.c_str(), signature.size());//验证签名
    if (retV <= 0) {
        error = GetErrorStr();
        EVP_MD_CTX_free(mdctx);
        errorL("EVP_DigestVerifyFinal:" << error);
        return false;
    }
    EVP_MD_CTX_free(mdctx);
    return true;
}

std::string SM2_Encrypt(std::string plaintext, sm2PublicKey& pub_key, unsigned int& enc_len)
{
    std::string error;
    std::string encrypted;
    encrypted = pub_key.Encrypt(plaintext, error);
    const char* tmp;
    tmp = encrypted.c_str();
    enc_len = strlen(tmp);
    return encrypted;

    //std::cout << "加密后密文：";
    //for (int i = 0; i < strlen(en); i++)
    //{
    //    std::cout << std::hex << int(en[i]);
    //}
    //std::cout << "\n";
}

std::string SM2_Decrypt(std::string ciphertext, sm2PrivateKey& priv_key, unsigned int& dec_len)
{
    std::string error;
    std::string decrypted;
    decrypted = priv_key.Decrypt(ciphertext, error);
    const char* tmp;
    tmp = decrypted.c_str();
    dec_len = strlen(tmp);
    return decrypted;
}

std::string SM2_Sign(std::string plaintext, sm2PrivateKey& priv_key, unsigned int& sig_len)
{
    const int kNumBytes = 128;  // 1024 bits
    unsigned char buffer[kNumBytes];
    std::ostringstream oss;

    //生成随机数
    if (!RAND_bytes(buffer, kNumBytes))
    {
        std::cerr << "Error: failed to generate random bytes.\n";
        //return 1;
    }

    //将随机数转为16进制字符串
    for (int i = 0; i < kNumBytes; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]);
    }
    std::string error;

    //用私钥对消息签名
    std::string signature;
    signature = priv_key.Signature(plaintext, error);
    const char* tmp;
    tmp = signature.c_str();
    sig_len = strlen(tmp);
    return signature;
}

bool SM2_Verify(std::string plaintext, sm2PublicKey& pub_key, std::string signature)
{
    std::string error;
    //用公钥验证签名
    if (!pub_key.SignatureVerification(signature, plaintext, error))
    {
        printf("error!\n");
        return false;
    }
    else
        return true;
}

int main()
{
    clock_t start, end;
    start = clock();
    //生成密钥对
    sm2PrivateKey priv_key;

    //导出公钥
    sm2PublicKey pub_key = priv_key.CreatePublic();
    end = clock();
    std::string plaintext = "Hello world!";
    std::string ciphertext;
    std::string decrypted_text;
    std::string signature;
    unsigned int enc_len;  //加密长度
    unsigned int dec_len;  //解密长度
    unsigned int sig_len;  //签名长度

    std::cout << "密钥生成的运行时间为: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << std::endl;

    //加密
    ciphertext = SM2_Encrypt(plaintext, pub_key, enc_len);
    const char* en = ciphertext.c_str();
    std::cout << "加密后密文：";
    for (int i = 0; i < enc_len; i++)
    {
        std::cout << std::hex << int(en[i]);
    }
    std::cout << "\n";

    //解密
    decrypted_text = SM2_Decrypt(ciphertext, priv_key, dec_len);
    std::cout << "解密后密文：" << decrypted_text.c_str() << std::endl;
    printf("\n");

    //签名
    signature = SM2_Sign(plaintext, priv_key, sig_len);
    const char* sig = signature.c_str();
    std::cout << "签名：";
    for (int i = 0; i < sig_len; i++)
    {
        std::cout << std::hex << int(sig[i]);
    }
    std::cout << "\n";

    //验证
    std::cout << "验证结果为：" << std::endl;
    std::cout << SM2_Verify(plaintext, pub_key, signature);
    return 0;
}
