#include <iostream>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bn.h>
#include <time.h>
#include <windows.h>

#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

const int SM4_KEY_LENGTH = 16;
const int SM4_BLOCK_SIZE = 16;

unsigned int plaintext[1048576] = { 0 };

void hex_print(const char* label, const unsigned char* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

int main() {
    // 原始数据
    unsigned char plaintext[] = "Hello world";
    unsigned char ciphertext[128] = { 0 };
    unsigned char decryptedtext[128] = { 0 };

    // 密钥和初始向量
    unsigned char key[16] = { 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89,
                  0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76 };
    unsigned char iv[16] = { 0x67, 0x45, 0x23, 0x01, 0xef, 0xcd, 0xab, 0x89,
                  0x98, 0xba, 0xdc, 0xfe, 0x10, 0x32, 0x54, 0x76 };

    hex_print("Plain text", plaintext, strlen((const char*)plaintext));
    
    //时间
    LARGE_INTEGER t1, t2, t3, t4, tc;
    QueryPerformanceFrequency(&tc);

    // 加密
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_sm4_cbc(), NULL, key, iv);
    QueryPerformanceCounter(&t1);
    EVP_EncryptInit_ex(ctx, EVP_sm4_ecb(), NULL, key, NULL);
    QueryPerformanceCounter(&t2);
    int len;
    EVP_EncryptUpdate(ctx, ciphertext, &len, (unsigned char*)plaintext, strlen((const char*)plaintext));
    int ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);

    hex_print("Encrypted text", ciphertext, ciphertext_len);

    std::cout << "加密花费时间：" << (double)(t2.QuadPart - t1.QuadPart) / (double)tc.QuadPart << "s\n";

    return 0;
}
