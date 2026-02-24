// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef XMEMCOPY_ACCEL_H
#define XMEMCOPY_ACCEL_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xmemcopy_accel_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#else
typedef struct {
#ifdef SDT
    char *Name;
#else
    u16 DeviceId;
#endif
    u64 Ctrl_bus_BaseAddress;
} XMemcopy_accel_Config;
#endif

typedef struct {
    u64 Ctrl_bus_BaseAddress;
    u32 IsReady;
} XMemcopy_accel;

typedef u32 word_type;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XMemcopy_accel_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XMemcopy_accel_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XMemcopy_accel_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XMemcopy_accel_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
#ifdef SDT
int XMemcopy_accel_Initialize(XMemcopy_accel *InstancePtr, UINTPTR BaseAddress);
XMemcopy_accel_Config* XMemcopy_accel_LookupConfig(UINTPTR BaseAddress);
#else
int XMemcopy_accel_Initialize(XMemcopy_accel *InstancePtr, u16 DeviceId);
XMemcopy_accel_Config* XMemcopy_accel_LookupConfig(u16 DeviceId);
#endif
int XMemcopy_accel_CfgInitialize(XMemcopy_accel *InstancePtr, XMemcopy_accel_Config *ConfigPtr);
#else
int XMemcopy_accel_Initialize(XMemcopy_accel *InstancePtr, const char* InstanceName);
int XMemcopy_accel_Release(XMemcopy_accel *InstancePtr);
#endif

void XMemcopy_accel_Start(XMemcopy_accel *InstancePtr);
u32 XMemcopy_accel_IsDone(XMemcopy_accel *InstancePtr);
u32 XMemcopy_accel_IsIdle(XMemcopy_accel *InstancePtr);
u32 XMemcopy_accel_IsReady(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_EnableAutoRestart(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_DisableAutoRestart(XMemcopy_accel *InstancePtr);

void XMemcopy_accel_Set_src(XMemcopy_accel *InstancePtr, u64 Data);
u64 XMemcopy_accel_Get_src(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_Set_dst(XMemcopy_accel *InstancePtr, u64 Data);
u64 XMemcopy_accel_Get_dst(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_Set_len(XMemcopy_accel *InstancePtr, u32 Data);
u32 XMemcopy_accel_Get_len(XMemcopy_accel *InstancePtr);

void XMemcopy_accel_InterruptGlobalEnable(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_InterruptGlobalDisable(XMemcopy_accel *InstancePtr);
void XMemcopy_accel_InterruptEnable(XMemcopy_accel *InstancePtr, u32 Mask);
void XMemcopy_accel_InterruptDisable(XMemcopy_accel *InstancePtr, u32 Mask);
void XMemcopy_accel_InterruptClear(XMemcopy_accel *InstancePtr, u32 Mask);
u32 XMemcopy_accel_InterruptGetEnabled(XMemcopy_accel *InstancePtr);
u32 XMemcopy_accel_InterruptGetStatus(XMemcopy_accel *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
