#include<iostream>
#include<chrono>
#include <thread>
#include <mutex>

using namespace std;

#define ll long long
#define a 0
#define b 1
#define c 2
#define d 3
#define e 4
#define f 5
#define g 6
#define h 7

const int MESS[] = { 0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,
					0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64,0x61,0x62,0x63,0x64, };	//每个元素一字节


const unsigned int IV[8] = { 0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600,
							 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e };

unsigned int T[64] = { 0 };

/* 初始化T，每次运行SM3前需调用 */
void _init_T()
{
	int i = 0;
	for (; i < 16; i++)
		T[i] = 0x79CC4519;
	for (; i < 64; i++)
		T[i] = 0x7A879D8A;
}
/* 循环左移，对int类型变量X循环左移n位 */
int ROTL(int X, int n)
{
	return (((X << n) & 0xffffffff) | ((X & 0xffffffff) >> (32 - n)));
}
/* 布尔函数 */
int FF(int X, int Y, int Z, int j) 
{
	if (j < 16 && j >= 0)
		return(X ^ Y ^ Z);
	if (j > 15 && j < 64)
		return((X & Y) | (X & Z) | (Y & Z));
	return false;
}

int GG(int X, int Y, int Z, int j) 
{
	if (j < 16 && j >= 0)
		return(X ^ Y ^ Z);
	if (j > 15 && j < 64)
		return ((X & Y) | ((~X) & Z));
	return false;
}

/* 压缩函数 */
void CF(int* v, int* B)
{
	int W68[68] = { 0 };
	int W64[64] = { 0 };
	int V[8] = { 0 };
	for (int i = 0; i < 8; i++)
		V[i] = v[i];
	/* 消息拓展 */
	int j = 0;
	for (; j < 16; j++) 
	{
		W68[j] = B[j];
	}
	for (; j <= 67; j++) 
	{
		int w0 = W68[j - 16] ^ W68[j - 9] ^ ROTL(W68[j - 3], 15);
		W68[j] = w0 ^ ROTL(w0, 15) ^ ROTL(w0, 23) ^ ROTL(W68[j - 13], 7) ^ W68[j - 6];
	}
	for (int i = 0; i < 64; i++) 
	{
		W64[i] = W68[i] ^ W68[i + 4];
	}
	/* 压缩函数 */
	int SS1 = 0, SS2 = 0, TT1 = 0, TT2 = 0;
	for (int j = 0; j < 64; j++) 
	{
		SS1 = ROTL(ROTL(V[a], 12) + V[e] + ROTL(T[j], j), 7);
		SS2 = SS1 ^ (((V[a] << 12) & 0xffffffff) | ((V[a] & 0xffffffff) >> (32 - 12)));
		TT1 = FF(V[a], V[b], V[c], j) + V[d] + SS2 + W64[j];
		TT2 = GG(V[e], V[f], V[g], j) + V[h] + SS1 + W68[j];
		V[d] = V[c];
		V[c] = ((V[b] << 9) & 0xffffffff) | ((V[b] & 0xffffffff) >> (32 - 9));
		V[b] = V[a];
		V[a] = TT1;
		V[h] = V[g];
		V[g] = ((V[f] << 19) & 0xffffffff) | ((V[f] & 0xffffffff) >> (32 - 19));
		V[f] = V[e];
		V[e] = TT2 ^ ROTL(TT2, 9) ^ ROTL(TT2, 17);
	}

	for (int i = 0; i < 8; i++)
		v[i] ^= V[i];
}
/* 哈希函数，输入输出均为每个元素4字节的int类型数组，size为字节数 */
void SM3(int* input, int* output, ll size)
{
	//填充
	ll n = size / 64;
	ll k = size % 64;
	size *= 8;	//总bit数
	/* 512bit，即16个字为一组B[i]，一共16 * (n + 1)个元素，即n+1组 */
	int* B = new int[16 * (n + 1)];
	int i = 0;
	int x = 16 * n;
	for (; i < x; i++) 
	{
		B[i] = input[i];
	}
	x += k / 4;
	for (; i < x; i++)
		B[i] = input[i];
	if (k % 4 == 0)
		B[i] = 0x80000000;
	else
		B[i] = input[i] | (0x80 << ((3 - (k % 4)) * 8));
	++i;
	//循环展开
	x = 16 * n + 14;
	for (; i < x; i++) 
	{
		B[i] = 0;
	}
	B[16 * n + 15] = size;
	B[16 * n + 14] = size >> 32;
	n++;
	//iterate(B, output, n);
	for (int j = 0; j < 8; j++)
	{
		output[j] = IV[j];
	}
	for (int i = 0; i < n; i++)
		CF(output, (B + (i * 16)));
}

/* 测试函数 */
void test() 
{
	_init_T();
	//首先把一个元素一个字节的数组MESS(字符串)，变为一个元素一个字的数组B(int)
	ll size = sizeof(MESS) / sizeof(int);	//size为所占的字节数
	int V[8] = { 0 };
	int* B = new int[size / 4 + (bool)(size % 4)];
	int i = 0;
	for (; i < size / 4; i++)
		B[i] = MESS[i * 4] << 24 | MESS[i * 4 + 1] << 16 | MESS[i * 4 + 2] << 8 | MESS[i * 4 + 3];
	if (size % 4) 
	{
		B[i] = 0;
		for (int k = 0; k < size % 4; k++)
			B[i] = B[i] | (MESS[i * 4 + k] << (8 * (3 - k)));
	}
	//然后将B作为SM3的输入
	SM3(B, V, size);
	cout << hex;
	for (int i = 0; i < 8; i++)
		cout << V[i] << ' ';
}

int main()
{
	std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
	test();
	std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = end - start;
	cout << "\n耗时\n" << elapsed.count() << "s";
	return 0;
}
