
#include "ref_kernels.h"
#include <math.h>
#include <string.h>

static inline float deq(uint8_t q, float s, int zp) {
  return s * ((int)q - zp);
}

static inline uint8_t req(float r, float s, int zp) {
  int q = zp + (int)lrintf(r / s);
  if (q < 0)
    q = 0;
  else if (q > 255)
    q = 255; // 클램핑 방지
  return (uint8_t)q;
}

// Bit-Serial Dot Product Emulator (Software version for verification)
static inline int32_t bit_serial_dot_product_sw(uint8_t activation, uint8_t weight_bits, int bits, int weight_zp) {
    int32_t acc = 0;
    // 1. Qw * X (Bit-serial)
    for (int i = 0; i < bits; i++) {
        if ((weight_bits >> i) & 1) {
            acc += (int32_t)activation << i;
        }
    }
    // 2. -Zp * X
    acc -= (int32_t)weight_zp * (int32_t)activation;
    return acc;
}

void dwconv3x3_nhwc_mixed(const tensor_u8_nhwc_t *in, const uint8_t *k3x3,
                          const int32_t *bias, float w_scale, int w_zp,
                          int bits, tensor_u8_nhwc_t *out, int apply_relu6) {
    int H = in->H, W = in->W, C = in->C;
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            for (int c = 0; c < C; ++c) {
                float acc_f = (bias ? (float)bias[c] * in->scale * w_scale : 0.0f);
                int32_t acc_int = 0;

                for (int ky = -1; ky <= 1; ++ky) {
                    int iy = y + ky;
                    if (iy < 0 || iy >= H) continue;
                    for (int kx = -1; kx <= 1; ++kx) {
                        int ix = x + kx;
                        if (ix < 0 || ix >= W) continue;

                        uint8_t q_in = in->data[(iy * W + ix) * C + c];
                        uint8_t q_k = k3x3[c * 9 + (ky + 1) * 3 + (kx + 1)];
                        acc_int += bit_serial_dot_product_sw(q_in, q_k, bits, w_zp);
                    }
                }
                // Convert back to float for output scaling (matching baseline process)
                float acc_total = acc_f + (float)acc_int * in->scale * w_scale;
                uint8_t q = req(acc_total, out->scale, out->zp);
                if (apply_relu6) {
                    int q6 = out->zp + (int)lrintf(6.0f / out->scale);
                    if (q > q6) q = (uint8_t)q6;
                    if (q < out->zp) q = (uint8_t)out->zp;
                }
                out->data[(y * W + x) * C + c] = q;
            }
}

void pwconv1x1_nhwc_mixed(const tensor_u8_nhwc_t *in, const uint8_t *k1x1,
                          const int32_t *bias, float w_scale, int w_zp,
                          int bits, tensor_u8_nhwc_t *out) {
    int H = in->H, W = in->W, Cin = in->C, Cout = out->C;
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            for (int co = 0; co < Cout; ++co) {
                float acc_f = (bias ? (float)bias[co] * in->scale * w_scale : 0.0f);
                int32_t acc_int = 0;
                const uint8_t *wrow = &k1x1[co * Cin];
                for (int ci = 0; ci < Cin; ++ci) {
                    uint8_t q_in = in->data[(y * W + x) * Cin + ci];
                    acc_int += bit_serial_dot_product_sw(q_in, wrow[ci], bits, w_zp);
                }
                float acc_total = acc_f + (float)acc_int * in->scale * w_scale;
                out->data[(y * W + x) * Cout + co] = req(acc_total, out->scale, out->zp);
            }
        }
}


void avgpool_global_nhwc_u8(const tensor_u8_nhwc_t *in, tensor_u8_nhwc_t *out) {
  int H = in->H, W = in->W, C = in->C;
  for (int c = 0; c < C; ++c) {
    float sum = 0.0f;
    for (int y = 0; y < H; ++y)
      for (int x = 0; x < W; ++x)
        sum += deq(in->data[(y * W + x) * C + c], in->scale, in->zp);
    float mean = sum / (float)(H * W);
    out->data[c] = req(mean, out->scale, out->zp);
  }
}

void softmax_u8(const tensor_u8_nhwc_t *in, tensor_u8_nhwc_t *out) {
  int C = in->C;
  float tmp[2048];
  if (C > 2048)
    return;
  float maxv = -1e30f;
  for (int c = 0; c < C; ++c) {
    float r = deq(in->data[c], in->scale, in->zp);
    tmp[c] = r;
    if (r > maxv)
      maxv = r;
  }
  float sum = 0.0f;
  for (int c = 0; c < C; ++c) {
    tmp[c] = expf(tmp[c] - maxv);
    sum += tmp[c];
  }
  for (int c = 0; c < C; ++c) {
    float prob = tmp[c] / sum;
    out->data[c] = req(prob, out->scale, out->zp);
  }
}
