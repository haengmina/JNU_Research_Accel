#include "xil_printf.h"
#include "sd_io.h"
#include "bmp_loader.h"
#include <stdio.h>
#include "xiltimer.h"

// Linker script symbols for memory usage stats
extern u8 _el3_stack_end[];
extern u8 __el3_stack[];
extern u8 _heap_start[];
extern u8 _heap_end[];

// Define maximum image size (e.g., 1920x1080 * 3 bytes + header)
// 6MB is sufficient for 1080p RGB
#define MAX_IMAGE_SIZE (1920 * 1080 * 3 + 54)

// Global buffers to avoid stack overflow
// Place these in DDR (linker script usually handles .bss in DDR)
u8 file_buffer[MAX_IMAGE_SIZE];
u8 image_buffer[MAX_IMAGE_SIZE]; // For extracted RGB pixels

static int run_image_test(const char *filename) {
    int status;
    BMP_Header header;
    u32 file_size;
    XTime tStart, tEnd;
    double load_time, process_time;

    xil_printf("\n\r[Test] %s\n\r", filename);

    file_size = sd_get_file_size(filename);
    if (file_size == 0) {
        xil_printf("ERROR: File %s not found or empty\n\r", filename);
        return XST_FAILURE;
    }

    // Buffer Occupancy Report
    u32 occupancy_int = (file_size * 100) / MAX_IMAGE_SIZE;
    u32 occupancy_frac = ((file_size * 10000) / MAX_IMAGE_SIZE) % 100;
    xil_printf("[MEM] Image Size: %u bytes\n\r", file_size);
    xil_printf("[MEM] Buffer Occupancy: %u.%02u%% \n\r", 
               occupancy_int, occupancy_frac);

    if (file_size > MAX_IMAGE_SIZE) {
        xil_printf("ERROR: File too large for buffer\n\r");
        return XST_FAILURE;
    }

    // Measure Load Time
    XTime_GetTime(&tStart);
    status = sd_read_file(filename, file_buffer, file_size);
    XTime_GetTime(&tEnd);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: Failed to read file\n\r");
        return XST_FAILURE;
    }
    load_time = (double)(tEnd - tStart) / COUNTS_PER_SECOND;
    u32 load_ms = (u32)(load_time * 1000.0);
    u32 load_us_frac = (u32)((load_time * 1000.0 - load_ms) * 1000.0);
    xil_printf("[PERF] Load Time: %u.%03u ms\n\r", load_ms, load_us_frac);

    status = bmp_validate(file_buffer, file_size);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: Invalid BMP file\n\r");
        return XST_FAILURE;
    }

    // Measure Process Time (Parse + Extract)
    XTime_GetTime(&tStart);
    status = bmp_parse_header(file_buffer, &header);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: Failed to parse header\n\r");
        return XST_FAILURE;
    }

    status = bmp_extract_pixels(file_buffer, &header, image_buffer);
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: Failed to extract pixels\n\r");
        return XST_FAILURE;
    }
    XTime_GetTime(&tEnd);
    process_time = (double)(tEnd - tStart) / COUNTS_PER_SECOND;
    u32 proc_ms = (u32)(process_time * 1000.0);
    u32 proc_us_frac = (u32)((process_time * 1000.0 - proc_ms) * 1000.0);
    xil_printf("[PERF] Process Time: %u.%03u ms\n\r", proc_ms, proc_us_frac);

    xil_printf("PASS: %s (%dx%d)\n\r", filename, header.width, header.height);
    return XST_SUCCESS;
}

int main() {
    int status;
    int i;
    char filename[32];

    xil_printf("\n\r--- ZCU104 Baremetal Image Loader ---\n\r");

    // Memory Usage Report
    u32 stack_size_kb = (u32)((UINTPTR)__el3_stack - (UINTPTR)_el3_stack_end) / 1024;
    u32 heap_size_mb = (u32)((UINTPTR)_heap_end - (UINTPTR)_heap_start) / 1024 / 1024;
    u32 file_buf_mb = MAX_IMAGE_SIZE / 1024 / 1024;
    u32 img_buf_mb = MAX_IMAGE_SIZE / 1024 / 1024;

    xil_printf("\n\r=== Memory Configuration ===\n\r");
    xil_printf("Stack Size: %u KB\n\r", stack_size_kb);
    xil_printf("Heap Size: %u MB\n\r", heap_size_mb);
    xil_printf("File Buffer Size: %u MB\n\r", file_buf_mb);
    xil_printf("Image Buffer Size: %u MB\n\r", img_buf_mb);
    xil_printf("============================\n\r");

    // 1. Initialize SD Card
    status = sd_init();
    if (status != XST_SUCCESS) {
        xil_printf("ERROR: SD Init failed\n\r");
        return XST_FAILURE;
    }

    xil_printf("\n\r--- 10-image sequential test ---\n\r");
    for (i = 1; i <= 10; i++) {
        sprintf(filename, "0:/img_%02d.bmp", i);
        status = run_image_test(filename);
        if (status != XST_SUCCESS) {
            xil_printf("FAIL: 10-image test stopped at %s\n\r", filename);
            return XST_FAILURE;
        }
    }

    xil_printf("PASS: 10/10 images\n\r");
    xil_printf("\n\r--- Final test: bmp_24.bmp ---\n\r");
    status = run_image_test("0:/bmp_24.bmp");
    if (status != XST_SUCCESS) {
        xil_printf("FAIL: Final bmp_24.bmp test\n\r");
        return XST_FAILURE;
    }

    xil_printf("\n\rALL PASS: 10-image test + bmp_24.bmp final test\n\r");

    return XST_SUCCESS;
}
