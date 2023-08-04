
initial_vector = '7380166f4914b2b9172442d7da8a0600a96f30bc163138aae38dee4db0fb0e4e'

#轮常量函数.根据j返回对应的常量值
def t(j):
    if j < 16:
        return 0x79cc4519
    return 0x7a879d8a

#循环左移函数.对x执行循环左移k位操作
def cyclic_shift_left(x, k):        #csl改为cyclic_shift_left
    x = '{:032b}'.format(x)
    k = k % 32
    x = x[k:] + x[:k]
    return int(x, 2)

#轮函数.根据索引j对x,y,z执行逻辑运算
def ff(x, y, z, j):
    if j < 16:
        return x ^ y ^ z
    return (x & y) | (y & z) | (z & x)

#轮函数.根据索引j对x,y,z执行逻辑运算
def gg(x, y, z, j):
    if j < 16:
        return x ^ y ^ z
    return (x & y) | (~x & z)

#消息扩展函数
def p0(x):
    return x ^ cyclic_shift_left(x, 9) ^ cyclic_shift_left(x, 17)
def p1(x):
    return x ^ cyclic_shift_left(x, 15) ^ cyclic_shift_left(x, 23)

#消息填充函数.将m按照SM3算法的填充规则进行填充，保证消息长度满足要求.
def plaintext_padding(m):
    l = len(m) * 4
    m = m + '8'
    k = 112 - (len(m) % 128)
    m = m + '0' * k + '{:016x}'.format(l)
    return m

#消息分组函数.将填充后的消息按照每组128位进行分组
def group_message(m):
    n = len(m) // 128
    b = []
    for i in range(n):
        b.append(m[i * 128: (i + 1) * 128])
    return b

#消息扩展函数.将每组128位消息扩展为132个32位字，用于后续的压缩函数。
def extend_message(bi):
    w = []
    for i in range(16):
        w.append(int(bi[i * 8: (i + 1) * 8], 16))
    for j in range(16, 68):
        w.append(p1(w[j - 16] ^ w[j - 9] ^ cyclic_shift_left(w[j - 3], 15)) ^ cyclic_shift_left(w[j - 13], 7) ^ w[j - 6])
    for j in range(68, 132):
        w.append(w[j - 68] ^ w[j - 64])
    return w

#压缩函数，对当前的状态vi和一个分组bi进行SM3算法的压缩步骤
def compression(vi, bi):
    w = extend_message(bi)
    a, b, c, d, e, f, g, h = int(vi[0:8], 16), int(vi[8:16], 16), int(vi[16:24], 16), int(vi[24:32], 16), int(vi[32:40], 16), int(vi[40:48], 16), int(vi[48:56], 16), int(vi[56:64], 16)
    for j in range(64):
        ss1 = cyclic_shift_left((cyclic_shift_left(a, 12) + e + cyclic_shift_left(t(j), j)) % pow(2, 32), 7)
        ss2 = ss1 ^ cyclic_shift_left(a, 12)
        tt1 = (ff(a, b, c, j) + d + ss2 + w[j + 68]) % pow(2, 32)
        tt2 = (gg(e, f, g, j) + h + ss1 + w[j]) % pow(2, 32)
        d = c
        c = cyclic_shift_left(b, 9)
        b = a
        a = tt1
        h = g
        g = cyclic_shift_left(f, 19)
        f = e
        e = p0(tt2)
    abcdefgh = int('{:08x}'.format(a) + '{:08x}'.format(b) + '{:08x}'.format(c) + '{:08x}'.format(d) + '{:08x}'.format(e) + '{:08x}'.format(f) + '{:08x}'.format(g) + '{:08x}'.format(h), 16)
    return '{:064x}'.format(abcdefgh ^ int(vi, 16))

#迭代函数.对所有分组进行迭代运算，不断更新状态。
def hash_iteration(b):
    n = len(b)
    v = initial_vector
    for i in range(n):
        v = compression(v, b[i])
    return v

#SM3算法的入口函数，接收一个消息m，首先填充消息，然后进行分组，最后进行迭代运算，输出128位的哈希值
def sm3_hash(m):
    m = plaintext_padding(m)
    b = group_message(m)
    return hash_iteration(b)
