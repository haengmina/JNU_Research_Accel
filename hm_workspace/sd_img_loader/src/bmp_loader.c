#include "bmp_loader.h"
#include "xil_printf.h"
#include <string.h>

int bmp_validate(const u8 *file_data, u32 file_size) {
  if (file_size < 54)
    return XST_FAILURE;
  if (file_data[0] != 'B' || file_data[1] != 'M')
    return XST_FAILURE;

  return XST_SUCCESS;
}

int bmp_parse_header(const u8 *file_data, BMP_Header *header) {
  memcpy(header, file_data, sizeof(BMP_Header));

  if (header->header_size != 40 || header->planes != 1 ||
      header->bits_per_pixel != 24 || header->compression != 0) {
    xil_printf("[BMP] Unsupported format: HeaderSize=%u, Planes=%d, BPP=%d, Comp=%u\n\r",
               header->header_size, header->planes, header->bits_per_pixel, header->compression);
    return XST_FAILURE;
  }

  return XST_SUCCESS;
}

int bmp_extract_pixels(const u8 *file_data, const BMP_Header *header,
                       u8 *rgb_buffer) {
  const u8 *pixel_data = file_data + header->data_offset;

  u32 w = header->width;
  u32 h = (header->height > 0) ? header->height : -header->height;
  u32 row_stride = ((w * 3 + 3) / 4) * 4; // 4-byte padding 처리

  for (u32 y = 0; y < h; y++) {
    // BMP는 기본적으로 하단(Bottom)부터 저장되어 있으므로,
    // 우리가 쓸 때는 뒤집어서(Top-down) 저장해야 함.
    u32 src_y = (header->height > 0) ? (h - 1 - y) : y;
    const u8 *src_row = pixel_data + (src_y * row_stride);
    u8 *dst_row = rgb_buffer + (y * w * 3);

    for (u32 x = 0; x < w; x++) {
      // BGR (BMP 원본) -> RGB (가속기 입력용) 변환
      dst_row[x * 3 + 0] = src_row[x * 3 + 2]; // R
      dst_row[x * 3 + 1] = src_row[x * 3 + 1]; // G
      dst_row[x * 3 + 2] = src_row[x * 3 + 0]; // B
    }
  }

  return XST_SUCCESS;
}