import hashlib
import random

# 计算哈希函数
def hash_func(x, r):
    return hashlib.sha256(str(x).encode() + str(r).encode()).hexdigest()

# 定义区间和比较值
age = 43 # Alice的年龄为43岁
lower_bound = 21 # 下限为21岁
upper_bound = 122 # 上限为122岁
r = random.randint # 随机数

# 计算哈希值和比较值
h = hash_func(age, r)
c = int(hash_func(lower_bound, r), 16) + (age - lower_bound) * int(hash_func(upper_bound - lower_bound, r), 16)

# Alice构建证明
proof = []
while True:
    # 如果当前值小于等于比较值，则证明结束
    if int(h, 16) <= c:
        break
    # 否则，计算新的哈希值和比较值，并将哈希值添加到证明中
    r = int(hash_func(r, h), 16)
    h = hash_func(age, r)
    c = int(hash_func(lower_bound, r), 16) + (age - lower_bound) * int(hash_func(upper_bound - lower_bound, r), 16)
    proof.append(h)

# 打印证明
print("Proof:", proof)

# Bob验证证明
r = random.randint(1,10000) # 重新选择随机数
for h in proof:
    c = int(hash_func(lower_bound, r), 16) + (age - lower_bound) * int(hash_func(upper_bound - lower_bound, r), 16)
    if int(h, 16) > c:
        print("Proof verification failed")
        break
    r = int(hash_func(r, h), 16)
else:
    print("Proof verified successfully")
