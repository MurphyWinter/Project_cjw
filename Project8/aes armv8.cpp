#include <arm_neon.h>

void aes_encrypt(unsigned char *in, unsigned char *out, unsigned char *key) {
    uint8x16_t key_schedule[15], state;
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

    state = vld1q_u8(in);
    state = veorq_u8(state, key_schedule[0]);
    for (int i = 1; i < 10; i++) {
        state = vaesmcq_u8(state);
        state = vaeseq_u8(state, key_schedule[i]);
    }
    state = vaesmcq_u8(state);
    state = vaesfinalq_u8(state, key_schedule[10]);
    vst1q_u8(out, state);
}
