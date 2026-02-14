#include "sd_io.h"
#include "xil_printf.h"
#include "xil_cache.h"

static FATFS fatfs;
static int is_mounted = 0;

int sd_init(void) {
  FRESULT res;

  if (is_mounted)
    return XST_SUCCESS;

  // ZCU104 SD 슬롯 마운트
  res = f_mount(&fatfs, "0:/", 1);

  if (res != FR_OK) {
    xil_printf("[SD] ERROR: Mount failed (%d)\n\r", res);
    return XST_FAILURE;
  }

  is_mounted = 1;
  xil_printf("[SD] SUCCESS: SD Card mounted\n\r");

  return XST_SUCCESS;
}

u32 sd_get_file_size(const char *filename) {
  FILINFO fno;
  FRESULT res = f_stat(filename, &fno);

  if (res != FR_OK)
    return 0;
  return (u32)fno.fsize;
}

int sd_read_file(const char *filename, u8 *buffer, u32 buffer_size) {
  FIL fil;
  FRESULT res;
  UINT br;

  res = f_open(&fil, filename, FA_READ);

  if (res != FR_OK)
    return XST_FAILURE;

  res = f_read(&fil, buffer, buffer_size, &br);

  f_close(&fil);

  if (res != FR_OK || br != buffer_size) {
    xil_printf("[SD] ERROR: Read incomplete (%d/%d)\n\r", br, buffer_size);
    return XST_FAILURE;
  }

  // Invalidate Data Cache to ensure CPU reads new data from DDR
  Xil_DCacheInvalidateRange((UINTPTR)buffer, buffer_size);

  return XST_SUCCESS;
}
