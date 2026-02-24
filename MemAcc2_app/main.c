#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "xparameters.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xiltimer.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "memcopy_accel.h"

/* Handle discrepancies in Xilinx macro names across BSP versions */
#ifndef XPAR_FABRIC_MEMCOPY_ACCEL_0_INTR
  #ifdef XPAR_FABRIC_MEMCOPY_ACCEL_0_INTERRUPT_INTR
    #define MEMCOPY_INTR_ID XPAR_FABRIC_MEMCOPY_ACCEL_0_INTERRUPT_INTR
  #else
    #define MEMCOPY_INTR_ID 29 /* Based on xparameters.h discovery */
  #endif
#else
  #define MEMCOPY_INTR_ID XPAR_FABRIC_MEMCOPY_ACCEL_0_INTR
#endif

#ifndef XPAR_MEMCOPY_ACCEL_0_BASEADDR
  #define MEMCOPY_BASE 0x40000000u
#else
  #define MEMCOPY_BASE XPAR_MEMCOPY_ACCEL_0_BASEADDR
#endif

#ifndef XPAR_CPU_CORE_CLOCK_FREQ_HZ
  #ifdef XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
    #define CPU_FREQ XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ
  #else
    #define CPU_FREQ 666666687
  #endif
#else
  #define CPU_FREQ XPAR_CPU_CORE_CLOCK_FREQ_HZ
#endif

#define INTC_DEVICE_ID         XPAR_SCUGIC_SINGLE_DEVICE_ID
#define MEMCOPY_ACCEL_INTR_ID  MEMCOPY_INTR_ID

// CPU memcpy reference implementation
void cpu_memcopy(uint32_t* src, uint32_t* dst, uint32_t len_bytes) {
    uint32_t num_words = len_bytes / 4;
    for (uint32_t i = 0; i < num_words; i++) {
        dst[i] = src[i];
    }
}

/* Global variables */
static XScuGic Intc;
static volatile int memcopy_done = 0;

/* ----------------------------------------------------
 * Interrupt service routine for memcopy_accel
 * ---------------------------------------------------- */
void memcopy_isr(void *CallbackRef)
{
    (void)CallbackRef;
    memcopy_accel_interrupt_clear();

    // Read-back to flush AXI write and avoid IRQ retrigger (required on Zynq)
    (void)Xil_In32(MEMCOPY_BASE + MEMCOPY_ACCEL_ISR_OFFSET);  // read-back barrier

    memcopy_done = 1;
}

/* ================== Interrupt Setup ================== */
int setup_interrupt_system(void)
{
    XScuGic_Config *IntcConfig;
    int Status;

    IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
    if (IntcConfig == NULL)
        return XST_FAILURE;

    Status = XScuGic_CfgInitialize(&Intc, IntcConfig, IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &Intc);

    Status = XScuGic_Connect(&Intc,
                             MEMCOPY_ACCEL_INTR_ID,
                             (Xil_InterruptHandler)memcopy_isr,
                             NULL);
    if (Status != XST_SUCCESS)
        return XST_FAILURE;

    XScuGic_Enable(&Intc, MEMCOPY_ACCEL_INTR_ID);
    Xil_ExceptionEnable();
    memcopy_accel_interrupt_enable();
    xil_printf("Interrupt system setup complete.\r\n");
    return XST_SUCCESS;
}


int main()
{
	XTime tStart, tEnd;

    xil_printf("\r\n--- Memcopy_accel robust demo (Zybo Z7-10) ---\r\n");

    memcopy_accel_init(MEMCOPY_BASE);

    if (setup_interrupt_system() != XST_SUCCESS) {
        xil_printf("ERROR: Interrupt setup failed!\r\n");
        return -1;
    }

    xil_printf("\r------------------------------------------\r\n\n");

    const uint32_t NUM_WORDS = 8192; /* 32KB */
    const uint32_t BYTE_LEN = NUM_WORDS * 4;

    /* Allocate 32-byte aligned buffers for cache efficiency (Zynq cache line size) */
    uint32_t *src_buf = (uint32_t *)memalign(32, BYTE_LEN);
    uint32_t *dst_buf = (uint32_t *)memalign(32, BYTE_LEN);
    uint32_t *dst_buf_cpu = (uint32_t *)memalign(32, BYTE_LEN);


    if (!src_buf || !dst_buf || !dst_buf_cpu) {
        xil_printf("Error: aligned allocation failed\r\n");
        return -1;
    }


    /* Initialize source with pattern */
    for (uint32_t i = 0; i < NUM_WORDS; ++i) {
        src_buf[i] = 0xA5A50000u | i;
        dst_buf[i] = 0x0;
        dst_buf_cpu[i] = 0x0;
    }

    xil_printf("Buffers: SRC=0x%08x, DST=0x%08x, len=%u bytes\r\n",
               (unsigned int)src_buf, (unsigned int)dst_buf, BYTE_LEN);

    /* Cache maintenance before DMA */
    Xil_DCacheFlushRange((unsigned int)src_buf, BYTE_LEN);
    Xil_DCacheFlushRange((unsigned int)dst_buf, BYTE_LEN);

    xil_printf("Starting accelerator...\r\n");
    XTime_GetTime(&tStart);
    memcopy_done = 0;
    memcopy_accel_start((uint32_t)src_buf, (uint32_t)dst_buf, BYTE_LEN);

    /* WFI with Timeout Protection */
    XTime tStartWFI, tNow;
    XTime_GetTime(&tStartWFI);
    const XTime TIMEOUT_VAL = (XTime)COUNTS_PER_SECOND * 2; // 2 seconds

    do {
        __asm__ volatile ("wfi");   // wakes on any IRQ
        XTime_GetTime(&tNow);
        if ((tNow - tStartWFI) > TIMEOUT_VAL) {
            xil_printf("ERROR: Timeout waiting for accelerator interrupt!\r\n");
            break;
        }
    } while (!memcopy_done);

    XTime_GetTime(&tEnd);
    uint32_t time_accel = (uint32_t)((tEnd - tStart) / (COUNTS_PER_SECOND / 1000000));

    xil_printf("Accelerator finished in %d us\r\n", time_accel);

    /* Invalidate cache to read fresh data written by DMA */
    Xil_DCacheInvalidateRange((unsigned int)dst_buf, BYTE_LEN);

    /* Verify Accelerator result */
    int errors = 0;
    for (uint32_t i = 0; i < NUM_WORDS; ++i) {
        if (dst_buf[i] != src_buf[i]) {
            xil_printf("Accel Mismatch at idx %u: src=0x%08x dst=0x%08x\r\n",
                       i, src_buf[i], dst_buf[i]);
            errors++;
            if (errors > 10) break;
        }
    }

    if (errors == 0) {
        xil_printf("Accelerator test PASSED!\r\n");
    } else {
        xil_printf("Accelerator test FAILED with %d errors.\r\n", errors);
    }

    /* CPU Memcopy Benchmark */
    XTime_GetTime(&tStart);
    cpu_memcopy(src_buf, dst_buf_cpu, BYTE_LEN);
    XTime_GetTime(&tEnd);
    uint32_t time_cpu = (uint32_t)((tEnd - tStart) / (COUNTS_PER_SECOND / 1000000));

    /* Verify CPU result */
    errors = 0;
    for (uint32_t i = 0; i < NUM_WORDS; ++i) {
        if (dst_buf_cpu[i] != src_buf[i]) {
            xil_printf("CPU Mismatch at idx %u: src=0x%08x dst=0x%08x\r\n",
                       i, src_buf[i], dst_buf_cpu[i]);
            errors++;
            if (errors > 10) break;
        }
    }

    if (errors == 0) {
        xil_printf("CPU test PASSED!\r\n");
    }

    xil_printf("\r------------------------------------------\r\n\n");
    xil_printf("CPU memcpy done in %d us\r\n", time_cpu);
    xil_printf("Accelerator memcpy done in %d us\r\n", time_accel);

    free(src_buf);
    free(dst_buf);
    free(dst_buf_cpu);

    xil_printf("Demo complete.\r\n");

    while (1) {
        /* idle */
    }

    return 0;
}
