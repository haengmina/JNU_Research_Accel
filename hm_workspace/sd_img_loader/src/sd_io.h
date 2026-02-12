#ifndef SD_IO_H
#define SD_IO_H

#include "ff.h"
#include "xil_types.h"
#include "xstatus.h"

// SD 카드 초기화 및 마운트
int sd_init(void);

// 파일 존재 여부 확인 및 크기 획득
u32 sd_get_file_size(const char *filename);

// 파일을 버퍼로 읽기 (정렬된 버퍼 권장)
int sd_read_file(const char *filename, u8 *buffer, u32 buffer_size);

#endif