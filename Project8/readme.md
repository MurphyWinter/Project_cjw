# AES impl with ARM instruction

## 前言、项目说明

受限于电脑处理器是Intel Core i7，无法运行实际的ARM程序，这里只给出一个用ARMv8指令集实现的函数，实际使用时可以在main函数中调用以下函数来完成AES的加密

## 一、具体实现

### 引入头文件和定义函数：

#### 代码说明：

- `#include <arm_neon.h>`：引入ARM NEON指令集的头文件，以便使用NEON指令。
- `void aes_encrypt(unsigned char *in, unsigned char *out, unsigned char *key)`：定义AES加密函数，输入为in数组（待加密数据）、key数组（密钥）、out数组（存放加密结果）。

### 初始化变量：

#### 代码说明：

- `uint8x16_t key_schedule[15], state;`：定义变量`key_schedule`和`state`，`key_schedule`用于存储轮密钥，`state`用于存储AES加密状态。

### 计算轮密钥（Round Key）：

#### 代码说明：

- AES加密过程中，需要进行多轮加密，每轮使用不同的子密钥。在该代码中，首先通过vld1q_u8函数从输入的key数组中加载第一个128位密钥，然后使用vaeseq_u8指令将该密钥与自身进行AES加密，生成下一个轮密钥，循环进行10轮。
- 在每轮加密之后，使用vreinterpretq_u8_u32和vsetq_lane_u32指令设置每个子密钥的第3个32位值为当前轮数i，这是AES加密算法中的一个步骤。
- 最后四个key_schedule[11]到key_schedule[14]初始化为0，这是为了处理后续加密轮数的边界情况。

#### 代码呈现：

```c++
key_schedule[0] = vld1q_u8(key);
    for (int i = 1; i < 10; i++) {
        key_schedule[i] = vaeseq_u8(key_schedule[i - 1], key_schedule[i - 1]);
        key_schedule[i] = vreinterpretq_u8_u32(vsetq_lane_u32(i, vreinterpretq_u32_u8(key_schedule[i]), 3));
    }
    key_schedule[10] = vaeseq_u8(key_schedule[9], key_schedule[9]);
    key_schedule[10] = vreinterpretq_u8_u32(vsetq_lane_u32(0, vreinterpretq_u32_u8(key_schedule[10]), 3));
    key_schedule[11] = vdupq_n_u8(0);
    key_schedule[12] = vdupq_n_u8(0);
    key_schedule[13] = vdupq_n_u8(0);
    key_schedule[14] = vdupq_n_u8(0);
```

### 加密数据：

#### 代码说明：

- 使用vld1q_u8函数加载输入数组in中的128位数据到state变量中。
- 将state与第一个轮密钥key_schedule[0]进行异或运算（veorq_u8），这是AES算法的初始操作。
- 循环进行10轮加密：使用vaesmcq_u8指令进行MixColumns操作（这是AES算法中的一步），然后再使用vaeseq_u8指令进行ShiftRows和SubBytes操作，同时与对应的轮密钥进行异或运算，得到下一轮的state值。
- 最后一轮加密后，再次使用vaesmcq_u8指令进行MixColumns操作，然后使用vaesfinalq_u8指令对state和最后一个轮密钥key_schedule[10]进行加密，得到最终的加密结果。

#### 代码呈现：

```C++
state = vld1q_u8(in);
    state = veorq_u8(state, key_schedule[0]);
    for (int i = 1; i < 10; i++) {
        state = vaesmcq_u8(state);
        state = vaeseq_u8(state, key_schedule[i]);
    }
    state = vaesmcq_u8(state);
    state = vaesfinalq_u8(state, key_schedule[10]);
```

### 将加密结果存入输出数组：

#### 代码说明：

- 使用vst1q_u8函数将state中的128位加密结果存入输出数组out。

#### 代码展示：

```c++
 vst1q_u8(out, state);
```

##二、实现效果 

受限于电脑处理器是Intel Core i7，无法运行实际的ARM程序。因而本项目不提供实现效果展示图。

## 三、参考

PPT