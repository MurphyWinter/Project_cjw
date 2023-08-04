# -*- coding:utf-8 -*-
"""
作者:曹菁文
学号:202100460082
日期:2023年07月
"""
import secrets
from gmssl import sm3, func
import MyFunc as MF

'''导入必要的库和模块,初始化椭圆曲线参数和有限域阶'''
#设置椭圆曲线和有限域的阶
FIELD_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
CURVE_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
POINT_X = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
POINT_Y = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
BASE_POINT = (POINT_X, POINT_Y)

#设置椭圆曲线参数
CURVE_PARAM_A = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
CURVE_PARAM_B = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

secret_key, public_key = MF.generate_key()      #调用MF.generate_key()生成密钥对

'''计算消息标识符ZA'''
def calculate_ZA(identifier, a, b, x, y, sk, pk):
    entl = str(len(identifier.encode()) << 3)
    joint = entl + identifier + str(a) + str(b) + str(x) + str(y) + str(sk) + str(pk)
    joint_bytes = bytes(joint, encoding='utf-8')
    hash_value = sm3.sm3_hash(func.bytes_to_list(joint_bytes))
    return int(hash_value, 16)

'''生成私钥private_key1、private_key2，计算public_key1'''
if __name__ == '__main__':
    #生成私钥1、私钥2
    private_key1 = secrets.randbelow(CURVE_ORDER-1)
    private_key2 = secrets.randbelow(CURVE_ORDER-1)
    #生成私钥1、私钥2的乘法逆元
    private_key1_inverse = MF.calculate_inverse(private_key1, CURVE_ORDER)
    private_key2_inverse = MF.calculate_inverse(private_key2, CURVE_ORDER)
    public_key1 = MF.ECC_multiply(private_key1_inverse, BASE_POINT)     #计算公钥1
    print("private_key1 =", private_key1)
    print("private_key2 =", private_key2)
    print("public_key1 =", public_key1)

    '''生成共享公钥shared_public_key'''
    partial_public_key = MF.ECC_multiply(private_key2_inverse, public_key1)     #计算部分公钥
    base_point_inverse = [POINT_X, -POINT_Y]
    shared_public_key = MF.ECC_add(partial_public_key, base_point_inverse)      #计算共享公钥
    print("shared_public_key =", shared_public_key)

    '''设置计算Z_A为双方标识符,计算e和partial_public_key1并发送至接收方'''
    identifier = "murphywinter"
    Z_A = calculate_ZA(identifier, CURVE_PARAM_A, CURVE_PARAM_B, POINT_X, POINT_Y, secret_key, public_key)      #计算标识符Z_A
    message = b'HelloJason'
    _message = b''.join([MF.to_byte(i) for i in [Z_A, message]])        #构造消息,将Z_A和消息合并为字节串
    print("Message sent by murphywinter:", message)
    e = int.from_bytes(MF.to_byte(sm3.sm3_hash(func.bytes_to_list(_message))), byteorder='big')     #将合并后的消息进行哈希,转换得整数e
    random_number1 = secrets.randbelow(CURVE_ORDER-1)
    partial_public_key1 = MF.ECC_multiply(random_number1, BASE_POINT)
    print("e =", e)
    print("partial_public_key1 =", partial_public_key1)

    '''计算部分签名r'''
    random_number2 = secrets.randbelow(CURVE_ORDER-1)
    partial_public_key2 = MF.ECC_multiply(random_number2, BASE_POINT)
    partial_public_key3 = MF.ECC_multiply(random_number2, partial_public_key1)      #计算部分公钥3:将随机数2与部分公钥1相乘。
    combined_public_key = MF.ECC_add(partial_public_key3, partial_public_key2)
    r = (combined_public_key[0] + e) % CURVE_ORDER
    print("r =", r)
    s2 = (private_key2 * random_number2) % CURVE_ORDER
    s3 = (private_key2 * (r + random_number1)) % CURVE_ORDER
    print("s2 =", s2)
    print("s3 =", s3)

    '''生成签名s=(r,s)'''
    s = (private_key1 * random_number1 * s2 + private_key1 * s3 - r) % CURVE_ORDER
    if s != 0 and s != CURVE_ORDER - r:
        print("s =", s)
