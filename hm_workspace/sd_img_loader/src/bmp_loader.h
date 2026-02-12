#ifndef BMP_LOADER_H
#define BMP_LOADER_H

#include "xil_types.h"
#include "xstatus.h"

// BMP 헤더 구조체 (54 bytes)
typedef struct {
  u16 signature;      // "BM" (0x4D42)
  u32 file_size;      // 전체 파일 크기
  u32 reserved;       // 0
  u32 data_offset;    // 실제 픽셀 데이터 시작 위치
  u32 header_size;    // DIB 헤더 크기 (40)
  s32 width;          // 가로
  s32 height;         // 세로 (양수: Bottom-up, 음수: Top-down)
  u16 planes;         // 1
  u16 bits_per_pixel; // 24 (우리의 타겟)
  u32 compression;    // 0 (무압축)
  u32 image_size;     // 픽셀 데이터 크기
  s32 x_ppm;          // 해상도
  s32 y_ppm;
  u32 colors_used;
  u32 important_colors;
} __attribute__((packed)) BMP_Header;

int bmp_validate(const u8 *file_data, u32 file_size);
int bmp_parse_header(const u8 *file_data, BMP_Header *header);
int bmp_extract_pixels(const u8 *file_data, const BMP_Header *header,
                       u8 *rgb_buffer);

#endif
