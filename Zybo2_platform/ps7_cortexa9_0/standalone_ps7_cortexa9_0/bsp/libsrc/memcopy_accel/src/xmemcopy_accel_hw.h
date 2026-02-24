// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
// CTRL_BUS
// 0x00 : Control signals
//        bit 0  - ap_start (Read/Write/COH)
//        bit 1  - ap_done (Read/COR)
//        bit 2  - ap_idle (Read)
//        bit 3  - ap_ready (Read/COR)
//        bit 7  - auto_restart (Read/Write)
//        bit 9  - interrupt (Read)
//        others - reserved
// 0x04 : Global Interrupt Enable Register
//        bit 0  - Global Interrupt Enable (Read/Write)
//        others - reserved
// 0x08 : IP Interrupt Enable Register (Read/Write)
//        bit 0 - enable ap_done interrupt (Read/Write)
//        bit 1 - enable ap_ready interrupt (Read/Write)
//        others - reserved
// 0x0c : IP Interrupt Status Register (Read/TOW)
//        bit 0 - ap_done (Read/TOW)
//        bit 1 - ap_ready (Read/TOW)
//        others - reserved
// 0x10 : Data signal of src
//        bit 31~0 - src[31:0] (Read/Write)
// 0x14 : Data signal of src
//        bit 31~0 - src[63:32] (Read/Write)
// 0x18 : reserved
// 0x1c : Data signal of dst
//        bit 31~0 - dst[31:0] (Read/Write)
// 0x20 : Data signal of dst
//        bit 31~0 - dst[63:32] (Read/Write)
// 0x24 : reserved
// 0x28 : Data signal of len
//        bit 31~0 - len[31:0] (Read/Write)
// 0x2c : reserved
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL  0x00
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_GIE      0x04
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER      0x08
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_ISR      0x0c
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_SRC_DATA 0x10
#define XMEMCOPY_ACCEL_CTRL_BUS_BITS_SRC_DATA 64
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_DST_DATA 0x1c
#define XMEMCOPY_ACCEL_CTRL_BUS_BITS_DST_DATA 64
#define XMEMCOPY_ACCEL_CTRL_BUS_ADDR_LEN_DATA 0x28
#define XMEMCOPY_ACCEL_CTRL_BUS_BITS_LEN_DATA 32

