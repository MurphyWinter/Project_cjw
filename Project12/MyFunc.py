# -*- coding:utf-8 -*-
"""
作者:曹菁文
学号:202100460082
日期:2023年07月
"""
import secrets
'''初始化椭圆曲线参数和有限域阶'''
#设置椭圆曲线和有限域的阶
FIELD_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
CURVE_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
POINT_X = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
POINT_Y = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
BASE_POINT = (POINT_X, POINT_Y)

#设置椭圆曲线参数
CURVE_PARAM_A = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
CURVE_PARAM_B = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

'''基于扩展欧几里得算法求逆'''
def calculate_inverse(a, b):
    if a == b:
        return (a, 1, 0)        #若相等,则直接返回
    else:
        flag = False
        c = [a]
        d = [b]
        e = []
        res = []
        #循环判断,直到res==0
        i = 0
        while not flag:
            e.append(d[i] // c[i])
            res.append(d[i] % c[i])
            d.append(c[i])
            c.append(res[i])
            i += 1
            if res[i-1] == 0:
                flag = True
        i -= 1
        res1 = [1]      #系数
        res2 = [0]      #系数
        res3 = c[i]     #公因子
        i -= 1
        num = i
        while i >= 0:
            res2.append(res1[num - i])
            res1.append(res2[num - i] - e[i] * res1[num - i])
            i -= 1
        if res3 == 1:
            return res1[-1] % b
        else:
            return -1

'''Tonelli-Shanks求解二次剩余'''
def Tonelli_Shanks(y, p):
    #利用勒让德符号判断是否为二次剩余
    assert pow(y, (p - 1) // 2, p) == 1
    if p % 4 == 3:
        return pow(y, (p + 1) // 4, p)
    q = p - 1
    s = 0
    while q % 2 == 0:
        q = q // 2
        s += 1
    for t in range(2, p):
        if pow(t, (p - 1) // 2, p) == p - 1:
            c = pow(t, q, p)
            break
    r = pow(y, (q + 1) // 2, p)
    t = pow(y, q, p)
    m = s
    if t % p == 1:
        return r
    else:
        i = 0
        while t % p != 1:       #外层循环的判断条件
            temp = pow(t, 2**(i + 1), p)
            i += 1
            if temp % p == 1:
                b = pow(c, 2**(m - i - 1), p)
                r = r * b % p
                c = b * b % p
                t = t * c % p
                m = i
                i = 0       #每次内层循环结束后i值要更新为0
        return r

'''编写椭圆曲线加法与乘法运算'''
#椭圆曲线加法运算
def ECC_add(a, b):
    #考虑是否存在0的情况
    if a == 0 and b == 0:
        return 0
    if a == 0:
        return b
    if b == 0:
        return a
    if a == b:
        #此时无法直接求斜率，需要借助微分
        k = (3 * a[0] ** 2 + CURVE_PARAM_A) * calculate_inverse(2 * a[1], FIELD_ORDER) % FIELD_ORDER
        res = (k ** 2 - 2 * a[0]) % FIELD_ORDER
        list1 = [res, (k * (a[0] - res) - a[1]) % FIELD_ORDER]
        return list1
    else:
        #保证大数在前
        if a[0] > b[0]:
            temp = a
            a = b
            b = temp
        #计算斜率
        k = (b[1] - a[1]) * calculate_inverse(b[0] - a[0], FIELD_ORDER) % FIELD_ORDER
        #依据椭圆曲线的计算规则进行计算
        res = (k ** 2 - a[0] - b[0]) % FIELD_ORDER
        list2 = [res, (k * (a[0] - res) - a[1]) % FIELD_ORDER]
        return list2

#椭圆曲线乘法运算
def ECC_multiply(a, b):
    res = 0     #初始化res为无穷远点O
    a_bin = bin(a)[2:]      #将a转为二进制
    b_temp = b
    for i in reversed(range(len(a_bin))):
        if a_bin[i] == '1':
            res = ECC_add(res, b_temp)
        b_temp = ECC_add(b_temp, b_temp)
    return res

'''基于secret库的密钥生成函数'''
def generate_key():
    private_key = int(secrets.token_hex(32), 16)
    public_key = ECC_multiply(private_key, BASE_POINT)
    return private_key, public_key

'''变量类型转换函数'''
#转换为bytes，第二参数为字节数，可以为空白
def to_byte(x, size=None):
    if isinstance(x, int):
        if size is None:        #计算合适的字节数
            size = 0
            tmp = x >> 64
            while tmp:
                size += 8
                tmp >>= 64
            tmp = x >> (size << 3)
            while tmp:
                size += 1
                tmp >>= 8
        elif x >> (size << 3):      #指定的字节数不够则截取低位
            x &= (1 << (size << 3)) - 1
        return x.to_bytes(size, byteorder='big')
    elif isinstance(x, str):
        x = x.encode()
        if size is not None and len(x) > size:      #超过指定长度
            x = x[:size]         #截取左侧字节
        return x
    elif isinstance(x, bytes):
        if size is not None and len(x) > size:      #超过指定长度
            x = x[:size]         #截取左侧字节
        return x
    elif isinstance(x, tuple) and len(x) == 2 and type(x[0]) == type(x[1]) == int:
        #针对坐标形式(x, y)
        return to_byte(x[0], size) + to_byte(x[1], size)
    return bytes(x)