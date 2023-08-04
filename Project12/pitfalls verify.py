# -*- coding:utf-8 -*-
"""
作者:曹菁文 202100460082
日期:2023年07月
"""
import MyFunc
import secrets
from hashlib import sha256

'''设置有限域的阶以及椭圆曲线的阶'''
FIELD_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF
CURVE_ORDER = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFF7203DF6B21C6052B53BBF40939D54123
POINT_X = 0x32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7
POINT_Y = 0xBC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0
BASE_POINT = (POINT_X, POINT_Y)

'''设置椭圆曲线参数'''
CURVE_PARAM_A = 0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFC
CURVE_PARAM_B = 0x28E9FA9E9D9F5E344D5A9E4BCF6509A7F39789F515AB8F92DDBCBD414D940E93

'''生成公私钥对'''
sk,pk = MyFunc.generate_key()

'''利用私钥进行签名'''
def Sig(private_key, mes,k):
    res = []
    hash = int(sha256(mes.encode('utf-8')).hexdigest(),16)
    G1 = MyFunc.ECC_multiply(k, BASE_POINT)
    res.append(G1[0] % FIELD_ORDER)
    res.append(MyFunc.calculate_inverse(k, CURVE_ORDER) * (hash + res[0] * private_key) % CURVE_ORDER)
    return res




''' Leaking k leads to leaking of d'''
def Leaking_k(mes,r,s):
    e = int(sha256(mes.encode('utf-8')).hexdigest(), 16)
    r_ = MyFunc.calculate_inverse(r, CURVE_ORDER) % CURVE_ORDER
    d = r_*(k*s-e) % CURVE_ORDER
    print("(1)pitfalls1:Leaking k leads to leaking of d ")
    print("d:",d)


''' Reusing k leads to leaking of d'''
def Reuseing_k(sk1, mes1, mes2):
    print("(2)Reusing k leads to leaking of d")
    [r1,s1] = Sig(sk1, mes1,k)
    [r2,s2] = Sig(sk1,mes2,k)
    print('签名时使用的私钥d:',sk1)

    e1 = int(sha256(mes1.encode('utf-8')).hexdigest(),16)
    e2 = int(sha256(mes2.encode('utf-8')).hexdigest(),16)
    se = s1*e2 - s2*e1
    sr1 = s2*r1-s1*r2 % CURVE_ORDER
    sr1_ = MyFunc.calculate_inverse(sr1,CURVE_ORDER)
    d = (se *sr1_) % CURVE_ORDER
    print('在重复使用k后计算出泄露的私钥d‘=',d)
    print('泄露出的私钥d‘与签名所使用的的d是否相等：',d == sk1)

'''Two users, using k leads to leaking of d, that is they can deduce each other’s d'''
def reusing_k_by_2(sk11,sk22,mes11, mes22):
    [r,s11] = Sig(sk11,mes11,k)
    [r,s22] = Sig(sk22,mes22,k)
    print("(3)Two users, using k leads to leaking of d, that is they can deduce each other’s d")
    print("user1使用的私钥d1：",sk11)
    print("user2使用的私钥d2：",sk22)
    e11 = int(sha256(mes11.encode('utf-8')).hexdigest(),16)
    e22 = int(sha256(mes22.encode('utf-8')).hexdigest(),16)
    se = s22*e11 - s11*e22 +(s22 * r * sk11)
    sr1 = (s11*r) % CURVE_ORDER
    sr1_ = MyFunc.calculate_inverse(sr1,CURVE_ORDER)
    d = (se *sr1_) % CURVE_ORDER
    print('user1通过计算得出的user2使用的私钥d2’：',d)

    se_ = s11*e22 - s22*e11 +(s11 * r * sk22)
    sr2 = (s22*r) % CURVE_ORDER
    sr2_ = MyFunc.calculate_inverse(sr2,CURVE_ORDER)
    d_ = (se_ * sr2_) % CURVE_ORDER
    print('user1通过计算得出的user2使用的私钥d1',d_)
    print('user1与user2计算是否正确：',(d == sk22 and d_ == sk11))



'''verification check m'''
def verify_with_m(mes,r,s,pk):
    e = int(sha256(mes.encode('utf-8')).hexdigest(),16)
    s_ = MyFunc.calculate_inverse(s, CURVE_ORDER) % CURVE_ORDER
    es_ = e*s_ % CURVE_ORDER
    rs_ = r*s_ % CURVE_ORDER
    g1 = MyFunc.ECC_multiply(es_,BASE_POINT)
    g2 = MyFunc.ECC_multiply(rs_,pk)
    g = MyFunc.ECC_add(g1,g2)
    [x,y] = g
    if(x == r):
        print("pass verify")
        return True
    else:
        print("unpass verify")
        return False

'''verification does not check m'''
def verify_without_m(e,r,s,pk):
    s_ = MyFunc.calculate_inverse(s, CURVE_ORDER) % CURVE_ORDER
    es_ = e*s_ % CURVE_ORDER
    rs_ = r*s_ % CURVE_ORDER
    g1 = MyFunc.ECC_multiply(es_,BASE_POINT)
    g2 = MyFunc.ECC_multiply(rs_,pk)
    g = MyFunc.ECC_add(g1, g2)
    [x, y] = g
    if(x == r):
        print("pass verify")
        return True
    else:
        print("unpass verify")
        return False

'''Malleability, e.g. (r,s) and (r,-s)are both valid signatures, lead to blockchain network split'''
def Malleability(mes):
    [r,s] = Sig(sk,mes,k)
    print('(4)Malleability, e.g. (r,s) and (r,-s)are both valid signatures, lead to blockchain network split')
    verify_with_m(mes,r,s,pk)
    verify_with_m(mes,r,(-s)%CURVE_ORDER,pk)

'''One can forge signature if the verification does not check m'''
def forge():
    print("(6)One can forge signature if the verification does not check m")
    k1 = secrets.randbelow(CURVE_ORDER)
    k2 = secrets.randbelow(CURVE_ORDER)
    g1 = MyFunc.ECC_multiply(k1,BASE_POINT)
    g2 = MyFunc.ECC_multiply(k2,pk)
    [r1,s1] = MyFunc.ECC_add(g1,g2)
    r_k1 = r1*k1
    k2_ = MyFunc.calculate_inverse(k2,CURVE_ORDER)
    e1 = (r_k1*k2_) % CURVE_ORDER
    s1 = (r1*k2_) % CURVE_ORDER
    if (verify_without_m(e1, r1, s1, pk)):
        print('forge伪造成功')



'''Same d and k with ECDSA, leads to leaking of d'''
'''Schnorr:'''
def Schnorr(sk, mes):
    g1 = MyFunc.ECC_multiply(k,BASE_POINT)
    e = int(sha256((str(g1[0]) + mes).encode('utf-8')).hexdigest(), 16)
    s= (k + e * sk) % CURVE_ORDER
    return g1,e,s

def same_d_k(sk, mes):
    print("(7)Same d and k with ECDSA, leads to leaking of d")
    g1 = MyFunc.ECC_multiply(k,BASE_POINT)
    e = int(sha256((str(g1[0]) + str(mes)).encode('utf-8')).hexdigest(), 16)
    s= (k + e * sk) % CURVE_ORDER
    e1 = int(sha256(mes.encode('utf-8')).hexdigest(), 16)
    [r1,s1] = Sig(sk,mes,k)

    s1_ = e1 + r1*sk
    s1__ = MyFunc.calculate_inverse((s - e * sk) % CURVE_ORDER, CURVE_ORDER)
    s1 = (s1_*s1__) % CURVE_ORDER

    d_ = s1*s - e1
    d__ = MyFunc.calculate_inverse(s1*e+r1,CURVE_ORDER)
    d = (d_*d__) % CURVE_ORDER
    print('签名时使用的私钥d:', sk)
    print('与ECDSA使用相同的d、k从而计算出的私钥d:',d)
    print('使用相同d、k计算出的结果是否相等',sk==d)


if __name__=='__main__':
    text1 = "1234567890"
    text2 = "0987654321"
    '''先进行一次ecdsa签名'''
    k = secrets.randbelow(FIELD_ORDER)
    sign_ecdsa = Sig(sk, text1, k)
    [r, s] = sign_ecdsa
    '''Leaking k leads to leaking of d'''
    Leaking_k(text1,r,s)
    print("--------------------")

    '''Reusing k leads to leaking of d'''
    sk1, pk1 = MyFunc.generate_key()
    Reuseing_k(sk1, text1, text2)
    print("--------------------")

    '''Two users, using k leads to leaking of d, that is they can deduce each other’s d'''
    sk11, pk11 = MyFunc.generate_key()
    sk22, pk22 = MyFunc.generate_key()
    reusing_k_by_2(sk11, sk22, text1, text2)
    print("--------------------")

    '''Malleability, e.g. (r,s) and (r,-s)are both valid signatures, lead to blockchain network split'''
    Malleability(text1)
    print("--------------------")

    '''One can forge signature if the verification does not check m'''
    forge()
    print("--------------------")

    '''Same d and k with ECDSA, leads to leaking of d'''
    same_d_k(sk, text1)