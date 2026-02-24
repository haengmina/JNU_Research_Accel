// ==============================================================
// Vitis HLS - High-Level Synthesis from C, C++ and OpenCL v2024.2 (64-bit)
// Tool Version Limit: 2024.11
// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
// 
// ==============================================================
#ifndef __linux__

#include "xstatus.h"
#ifdef SDT
#include "xparameters.h"
#endif
#include "xmemcopy_accel.h"

extern XMemcopy_accel_Config XMemcopy_accel_ConfigTable[];

#ifdef SDT
XMemcopy_accel_Config *XMemcopy_accel_LookupConfig(UINTPTR BaseAddress) {
	XMemcopy_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = (u32)0x0; XMemcopy_accel_ConfigTable[Index].Name != NULL; Index++) {
		if (!BaseAddress || XMemcopy_accel_ConfigTable[Index].Ctrl_bus_BaseAddress == BaseAddress) {
			ConfigPtr = &XMemcopy_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMemcopy_accel_Initialize(XMemcopy_accel *InstancePtr, UINTPTR BaseAddress) {
	XMemcopy_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMemcopy_accel_LookupConfig(BaseAddress);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMemcopy_accel_CfgInitialize(InstancePtr, ConfigPtr);
}
#else
XMemcopy_accel_Config *XMemcopy_accel_LookupConfig(u16 DeviceId) {
	XMemcopy_accel_Config *ConfigPtr = NULL;

	int Index;

	for (Index = 0; Index < XPAR_XMEMCOPY_ACCEL_NUM_INSTANCES; Index++) {
		if (XMemcopy_accel_ConfigTable[Index].DeviceId == DeviceId) {
			ConfigPtr = &XMemcopy_accel_ConfigTable[Index];
			break;
		}
	}

	return ConfigPtr;
}

int XMemcopy_accel_Initialize(XMemcopy_accel *InstancePtr, u16 DeviceId) {
	XMemcopy_accel_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	ConfigPtr = XMemcopy_accel_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		InstancePtr->IsReady = 0;
		return (XST_DEVICE_NOT_FOUND);
	}

	return XMemcopy_accel_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif

#endif

