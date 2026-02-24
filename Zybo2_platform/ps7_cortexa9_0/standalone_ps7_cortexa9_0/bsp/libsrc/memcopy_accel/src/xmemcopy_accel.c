// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
/***************************** Include Files *********************************/
#include "xmemcopy_accel.h"

/************************** Function Implementation *************************/
#ifndef __linux__
int XMemcopy_accel_CfgInitialize(XMemcopy_accel *InstancePtr, XMemcopy_accel_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->Ctrl_bus_BaseAddress = ConfigPtr->Ctrl_bus_BaseAddress;
    InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

    return XST_SUCCESS;
}
#endif

void XMemcopy_accel_Start(XMemcopy_accel *InstancePtr) {
    u32 Data;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL) & 0x80;
    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL, Data | 0x01);
}

u32 XMemcopy_accel_IsDone(XMemcopy_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 1) & 0x1;
}

u32 XMemcopy_accel_IsIdle(XMemcopy_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL);
    return (Data >> 2) & 0x1;
}

u32 XMemcopy_accel_IsReady(XMemcopy_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL);
    // check ap_start to see if the pcore is ready for next input
    return !(Data & 0x1);
}

void XMemcopy_accel_EnableAutoRestart(XMemcopy_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL, 0x80);
}

void XMemcopy_accel_DisableAutoRestart(XMemcopy_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_AP_CTRL, 0);
}

void XMemcopy_accel_Set_src(XMemcopy_accel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_SRC_DATA, (u32)(Data));
    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_SRC_DATA + 4, (u32)(Data >> 32));
}

u64 XMemcopy_accel_Get_src(XMemcopy_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_SRC_DATA);
    Data += (u64)XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_SRC_DATA + 4) << 32;
    return Data;
}

void XMemcopy_accel_Set_dst(XMemcopy_accel *InstancePtr, u64 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_DST_DATA, (u32)(Data));
    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_DST_DATA + 4, (u32)(Data >> 32));
}

u64 XMemcopy_accel_Get_dst(XMemcopy_accel *InstancePtr) {
    u64 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_DST_DATA);
    Data += (u64)XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_DST_DATA + 4) << 32;
    return Data;
}

void XMemcopy_accel_Set_len(XMemcopy_accel *InstancePtr, u32 Data) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_LEN_DATA, Data);
}

u32 XMemcopy_accel_Get_len(XMemcopy_accel *InstancePtr) {
    u32 Data;

    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Data = XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_LEN_DATA);
    return Data;
}

void XMemcopy_accel_InterruptGlobalEnable(XMemcopy_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_GIE, 1);
}

void XMemcopy_accel_InterruptGlobalDisable(XMemcopy_accel *InstancePtr) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_GIE, 0);
}

void XMemcopy_accel_InterruptEnable(XMemcopy_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER);
    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER, Register | Mask);
}

void XMemcopy_accel_InterruptDisable(XMemcopy_accel *InstancePtr, u32 Mask) {
    u32 Register;

    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    Register =  XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER);
    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER, Register & (~Mask));
}

void XMemcopy_accel_InterruptClear(XMemcopy_accel *InstancePtr, u32 Mask) {
    Xil_AssertVoid(InstancePtr != NULL);
    Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    XMemcopy_accel_WriteReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_ISR, Mask);
}

u32 XMemcopy_accel_InterruptGetEnabled(XMemcopy_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_IER);
}

u32 XMemcopy_accel_InterruptGetStatus(XMemcopy_accel *InstancePtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

    return XMemcopy_accel_ReadReg(InstancePtr->Ctrl_bus_BaseAddress, XMEMCOPY_ACCEL_CTRL_BUS_ADDR_ISR);
}

