import hashlib
import ecdsa

# 定义椭圆曲线和哈希算法
curve = ecdsa.SECP256k1
hash_func = hashlib.sha256

# 定义要哈希的元素
elements = [b'hello', b'world']

# 将每个元素进行哈希
hashes = [hash_func(element).digest() for element in elements]

# 将哈希值映射到椭圆曲线上，并相加
point_sum = None
for hash_value in hashes:
    #注意Bicoin协议中用的是大端序
    x = int.from_bytes(hash_value, byteorder='big')
    y = curve.generator * x
    if point_sum is None:
        point_sum = y
    else:
        point_sum += y

# 将相加后的点的坐标表示作为哈希值输出
hash_value = point_sum.x() 
print(hex(hash_value))
