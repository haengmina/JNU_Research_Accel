#include <stdint.h>
#include <hls_stream.h>
#include <ap_int.h>
#include <string.h>

#define BURST_LEN 32  // number of 32-bit words per burst

void memcopy_accel(
    uint32_t* src,   // AXI4 Master port for source DDR
    uint32_t* dst,   // AXI4 Master port for destination DDR
    uint32_t len     // Number of bytes to copy
) {
#pragma HLS INTERFACE m_axi     port=src offset=slave bundle=AXI_SRC depth=1024
#pragma HLS INTERFACE m_axi     port=dst offset=slave bundle=AXI_DST depth=1024
#pragma HLS INTERFACE s_axilite port=src       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=dst       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=len       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return    bundle=CTRL_BUS

    uint32_t num_words = len >> 2;  // divide by 4
    uint32_t buffer[BURST_LEN];

copy_loop:
    for (uint32_t i = 0; i < num_words; i += BURST_LEN) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=1024
        uint32_t chunk = (i + BURST_LEN <= num_words) ? BURST_LEN : (num_words - i);

        // Optimized burst transfer via memcpy
        // HLS memcpy infers AXI4 Burst for II=1 throughput
        memcpy(buffer, (const uint32_t*)&src[i], chunk * sizeof(uint32_t));
        memcpy((uint32_t*)&dst[i], buffer, chunk * sizeof(uint32_t));
    }
}
