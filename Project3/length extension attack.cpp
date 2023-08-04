#include<iostream>
#include<chrono>
using namespace std;
/*
const int MESS[] = { 0x61,0x62,0x63 };	//每个元素一字节
*/
const int MESS1[] = { 0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63 };	//每个元素一字节
const int MESS2[] = { 0x12,0x32,0x32,0x67,0x34 };
//填充的长度
const int padding[] = { 0x0,0x228 };
//填充后的MESS1
const int EXMESS1_MESS2[] = { 0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
								0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
								0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
								0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xB8,
								0x12,0x32,0x32,0x67,0x34 };
#define ll long long
#define a 0
#define b 1
#define c 2
#define d 3
#define e 4
#define f 5
#define g 6
#define h 7

const unsigned int IV[8] =
{
	0x7380166f,	0x4914b2b9,
	0x172442d7,	0xda8a0600,
	0xa96f30bc,	0x163138aa,
	0xe38dee4d,	0xb0fb0e4e,
};

unsigned int T[64] = { 0 };
//初始化T，每次运行SM3前需调用
void _init_T()
{
	int i = 0;
	for (; i < 16; i++)
		T[i] = 0x79CC4519;
	for (; i < 64; i++)
		T[i] = 0x7A879D8A;
}
//循环左移，对int类型变量X循环左移n位
int ROTL(int X, int n) {
	return (((X << n) & 0xffffffff) | ((X & 0xffffffff) >> (32 - n)));
}
//布尔函数
int FF(int X, int Y, int Z, int j) {
	if (j < 16 && j >= 0)
		return(X ^ Y ^ Z);
	if (j > 15 && j < 64)
		return((X & Y) | (X & Z) | (Y & Z));
	return false;
}
int GG(int X, int Y, int Z, int j) {
	if (j < 16 && j >= 0)
		return(X ^ Y ^ Z);
	if (j > 15 && j < 64)
		return ((X & Y) | ((~X) & Z));
	return false;
}
//压缩函数中的置换函数
int P0(int X) {
	return X ^ ROTL(X, 9) ^ ROTL(X, 17);
}
//消息扩展中的置换函数
int P1(int X) {
	return X ^ ROTL(X, 15) ^ ROTL(X, 23);
}
//压缩函数
void CF(int* v, int* B) {
	int W68[68] = { 0 };
	int W64[64] = { 0 };
	int V[8] = { 0 };
	for (int i = 0; i < 8; i++)
		V[i] = v[i];
	//消息拓展
	int j = 0;
	for (; j < 16; j++)
		W68[j] = B[j];
	for (; j <= 67; j++)
		W68[j] = P1(W68[j - 16] ^ W68[j - 9] ^ ROTL(W68[j - 3], 15)) ^ ROTL(W68[j - 13], 7) ^ W68[j - 6];
	for (j = 0; j < 64; j++)
		W64[j] = W68[j] ^ W68[j + 4];
	//压缩函数
	int SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
	for (j = 0; j < 64; j++) {
		SS1 = ROTL(ROTL(V[a], 12) + V[e] + ROTL(T[j], j), 7);
		SS2 = SS1 ^ ROTL(V[a], 12);
		TT1 = FF(V[a], V[b], V[c], j) + V[d] + SS2 + W64[j];
		TT2 = GG(V[e], V[f], V[g], j) + V[h] + SS1 + W68[j];
		V[d] = V[c];
		V[c] = ROTL(V[b], 9);
		V[b] = V[a];
		V[a] = TT1;
		V[h] = V[g];
		V[g] = ROTL(V[f], 19);
		V[f] = V[e];
		V[e] = P0(TT2);
	}
	for (int i = 0; i < 8; i++)
		v[i] ^= V[i];
}
//迭代过程，B为填充后的分组，共 512*n bit
void iterate(int* B, int* V, int n) {
	for (int i = 0; i < 8; i++)
		V[i] = IV[i];
	for (int i = 0; i < n; i++)
		CF(V, (B + (i * 16)));
}
//哈希函数，输入输出均为每个元素4字节的int类型数组，size为字节数
void SM3(int* input, int* output, ll size) {
	_init_T();
	//填充
	ll n = size / 64;
	ll k = size % 64;
	size *= 8;	//总bit数
	//512bit，即16个字为一组B[i]，一共16 * (n + 1)个元素，即n+1组
	int* B = new int[16 * (n + 1)];
	B[16 * n + 15] = size;
	B[16 * n + 14] = size >> 32;
	int i = 0;
	for (; i < 16 * n + k / 4; i++)
		B[i] = input[i];
	if (k % 4 == 0)
		B[i] = 0x80000000;
	else
		B[i] = input[i] | (0x80 << ((3 - (k % 4)) * 8));
	++i;
	for (; i < 16 * n + 14; i++)
		B[i] = 0;
	n++;

	iterate(B, output, n);
}

void SM3_LEA(int* input, int* output, ll size, int* Iv) {
	_init_T();
	//填充
	ll n = size / 64;
	ll k = size % 64;
	size *= 8;	//总bit数
	//512bit，即16个字为一组B[i]，一共16 * (n + 1)个元素，即n+1组
	int* B = new int[16 * (n + 1)];
	B[16 * n + 15] = padding[1];
	B[16 * n + 14] = padding[0];
	int i = 0;
	for (; i < 16 * n + k / 4; i++)
		B[i] = input[i];
	if (k % 4 == 0)
		B[i] = 0x80000000;
	else
		B[i] = input[i] | (0x80 << ((3 - (k % 4)) * 8));
	++i;
	for (; i < 16 * n + 14; i++)
		B[i] = 0;
	n++;
	for (int i = 0; i < 8; i++)
		output[i] = Iv[i];
	for (int i = 0; i < n; i++)
		CF(output, (B + (i * 16)));
}
void attack() {
	//先获得MESS1的哈希值
	ll size = sizeof(MESS1) / sizeof(int);	//size为所占的字节数
	int V[8] = { 0 };
	int* B = new int[size / 4 + (bool)(size % 4)];
	int i = 0;
	for (; i < size / 4; i++)
		B[i] = MESS1[i * 4] << 24 | MESS1[i * 4 + 1] << 16 | MESS1[i * 4 + 2] << 8 | MESS1[i * 4 + 3];
	if (size % 4) {
		B[i] = 0;
		for (int k = 0; k < size % 4; k++)
			B[i] = B[i] | (MESS1[i * 4 + k] << (8 * (3 - k)));
	}
	SM3(B, V, size);
	size = sizeof(MESS2) / sizeof(int);
	int V2[8] = { 0 };
	int* B2 = new int[size / 4 + (bool)(size % 4)];
	i = 0;
	for (; i < size / 4; i++)
		B2[i] = MESS2[i * 4] << 24 | MESS2[i * 4 + 1] << 16 | MESS2[i * 4 + 2] << 8 | MESS2[i * 4 + 3];
	if (size % 4) {
		B2[i] = 0;
		for (int k = 0; k < size % 4; k++)
			B2[i] = B2[i] | (MESS2[i * 4 + k] << (8 * (3 - k)));
	}
	SM3_LEA(B2, V2, size, V);
	cout << hex;
	for (int i = 0; i < 8; i++)
		cout << V2[i] << ' ';
	cout << endl;

}
void test() {
	//首先把一个元素一个字节的数组MESS(字符串)，变为一个元素一个字的数组B(int)
	ll size = sizeof(EXMESS1_MESS2) / sizeof(int);	//size为所占的字节数
	int V[8] = { 0 };

	int* B = new int[size / 4 + (bool)(size % 4)];
	int i = 0;
	for (; i < size / 4; i++)
		B[i] = EXMESS1_MESS2[i * 4] << 24 | EXMESS1_MESS2[i * 4 + 1] << 16 | EXMESS1_MESS2[i * 4 + 2] << 8 | EXMESS1_MESS2[i * 4 + 3];
	if (size % 4) {
		B[i] = 0;
		for (int k = 0; k < size % 4; k++)
			B[i] = B[i] | (EXMESS1_MESS2[i * 4 + k] << (8 * (3 - k)));
	}
	SM3(B, V, size);

	cout << hex;
	for (int i = 0; i < 8; i++)
		cout << V[i] << ' ';

}
int main()
{
	attack();
	test();
}