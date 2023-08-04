import hashlib
import math
import json
import random
import time
import string

#创建一棵Merkle Tree
def create_MT(data):   
    # 计算原始数据的哈希值，hashes记录原始数据hash后的结果，nodes记录所有hash值
    hashes = [hashlib.sha256(item.encode()).hexdigest() for item in data]
    nodes = hashes

    # 构建Merkle Tree
    while len(hashes) > 1:
        # 如果数据长度为奇数，则将最后一个哈希值重复一次
        if len(hashes) % 2 == 1:
            hashes.append(hashes[-1])
        level = []
        for i in range(0, len(hashes), 2):
            left = hashes[i]
            right = hashes[i+1]
            level.append(hashlib.sha256((left + right).encode()).hexdigest())
        hashes = level
        nodes.extend(hashes)
    return hashes[0], nodes

#创建10w叶子结点
def gen_leafNode():
    return [''.join(random.sample('abcdefghijklmnopqrstuvwxyz0123456789',3)) for i in range(0,100000)]

#Inclusion proof（包含性证明）
def inclusion_proof(root_hash, nodes, element):
    # 找到包含特定元素的叶子节点的位置
    index = -1
    for i in range(len(nodes) // 2):
        if nodes[i] == hashlib.sha256(element.encode()).hexdigest():
            index = i
            break
    if index == -1:
        raise ValueError("Element not found in Merkle tree")

    # 构建包含证明
    proof = []
    while index > 0:
        # 找到当前节点的兄弟节点的位置
        sibling_index = index - 1 if index % 2 == 1 else index + 1
        # 将兄弟节点的哈希值添加到证明中
        proof.append(nodes[sibling_index])
        # 更新当前节点的位置为其父节点的位置
        index = (index - 1) // 2
    return proof

#Exclusion_proof（排除性证明）
def exclusion_proof(root_hash, nodes, element):
    # 找到包含特定元素的叶子节点的位置
    index = -1
    for i in range(len(nodes) // 2):
        if nodes[i] == hashlib.sha256(element.encode()).hexdigest():
            index = i
            break
    if index == -1:
        raise ValueError("Element not found in Merkle tree")

    # 构建排除证明
    proof = []
    while index > 0:
        # 找到当前节点的父节点的位置
        parent_index = (index - 1) // 2
        # 将不在路径上的兄弟节点的哈希值添加到证明中
        sibling_index = parent_index * 2 + 1 if index % 2 == 1 else parent_index * 2
        if sibling_index >= len(nodes):
            # 如果兄弟节点的位置超出了节点列表的范围，则将父节点的哈希值添加到证明中
            proof.append(nodes[parent_index])
        else:
            proof.append(nodes[sibling_index])
        # 更新当前节点的位置为其父节点的位置
        index = parent_index
    return proof

if __name__ == '__main__':
    #生成一个有10w个叶子结点的Merkle Tree
    Test_DATA = gen_leafNode()
    MT_1_root, hash_nodes = create_MT(Test_DATA)
    print("生成一个随机生成的10w数据作为叶子节点的Merkel Tree：")
    print(MT_1_root)

    #进行inclusion proof
    in_proof_path = inclusion_proof(MT_1_root, hash_nodes, '167')
    print(in_proof_path)

    #进行exclusion proof
    ex_proof_path = exclusion_proof(MT_1_root, hash_nodes, '132')
    print(ex_proof_path)
