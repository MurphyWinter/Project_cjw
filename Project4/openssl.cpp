#include <stdio.h>
#include <iostream>
#include <string.h>
#include <chrono>
#include <openssl/evp.h>
#include <openssl/err.h>

#define SM3_DIGEST_LENGTH 32

#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"libcrypto.lib")

const int MESS[] = { 0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
                    0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
                    0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
                    0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64, };	//每个元素一字节

int main() {
    unsigned char* message = (unsigned char*)MESS;
    unsigned char digest[SM3_DIGEST_LENGTH];
    unsigned int digest_len;

    EVP_MD_CTX* mdctx;
    const EVP_MD* md;
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    md = EVP_sm3();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, message, strlen((const char*)message));
    EVP_DigestFinal_ex(mdctx, digest, &digest_len);
    std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "耗时\n" << elapsed.count() << "s" << std::endl;
    EVP_MD_CTX_free(mdctx);

    return 0;
}
