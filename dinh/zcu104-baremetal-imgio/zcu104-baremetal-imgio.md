# `dinh/zcu104-baremetal-imgio/` 코드 리뷰 보고서

이 보고서는 `zcu104-baremetal-imgio` 프로젝트의 소스 코드에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 프로젝트는 ZCU104 보드의 베어메탈 환경에서 SD 카드로부터 BMP 이미지를 로드하는 기능을 구현합니다.

---

# `sd_bmp_loader.cpp` 코드 리뷰 보고서

이 파일은 AMD Xilinx Vitis의 베어메탈 환경에서 SD 카드로부터 24비트 BMP 파일을 읽고 파싱하는 C++ 애플리케이션입니다.

## 1. 헤더 및 네임스페이스

```cpp
// sd_bmp_loader.cpp
// Bare-metal C++ app for AMD Xilinx Vitis (standalone A53).
// Reads a 24-bit BMP from SD (FatFs) and prints basic info.

#include "xil_printf.h"
#include "xil_cache.h"
#include "ff.h"              // xilffs (FatFs)
#include <cstdint>
#include <vector>
#include <string>

using namespace std;
```

### 1.1 헤더 분석

| 헤더 | 용도 |
|------|------|
| `xil_printf.h` | Xilinx 베어메탈 경량 printf |
| `xil_cache.h` | 캐시 관리 함수 |
| `ff.h` | FatFs 파일 시스템 라이브러리 (xilffs) |
| `cstdint` | 고정 크기 정수 타입 (uint8_t, uint32_t 등) |
| `vector` | 동적 배열 컨테이너 |
| `string` | 문자열 클래스 |

### 1.2 베어메탈 C++ 사용

*   **std::vector 사용**: 베어메탈 환경에서도 C++ 표준 라이브러리의 일부를 사용할 수 있습니다. Xilinx standalone BSP는 newlib C 라이브러리를 포함하며, C++ 지원도 제공합니다.
*   **메모리 관리**: `vector`의 동적 할당은 힙을 사용하므로, 링커 스크립트에서 적절한 힙 크기가 설정되어야 합니다.

## 2. SD 카드 헬퍼 함수

### 2.1 SD 카드 마운트

```cpp
static FATFS fatfs;   // File system object
static FIL   file;    // File object

static bool sd_mount()
{
    FRESULT rc = f_mount(&fatfs, "0:/", 1);
    if (rc != FR_OK)
    {
        xil_printf("f_mount failed: %d\r\n", rc);
        return false;
    }
    xil_printf("SD mounted: 0:/\r\n");
    return true;
}
```

*   **전역 FatFs 객체**: `fatfs`와 `file`은 정적 전역 변수로 선언되어 함수 간에 공유됩니다.
*   **드라이브 번호**: "0:/"는 첫 번째 SD 카드 슬롯을 의미합니다.
*   **즉시 마운트**: 세 번째 매개변수 `1`은 즉시 마운트를 의미합니다 (지연 마운트 아님).

### 2.2 파일 전체 읽기

```cpp
static bool sd_read_all(const char* path, vector<uint8_t>& out)
{
    FRESULT rc = f_open(&file, path, FA_READ);
    if (rc != FR_OK) {
        xil_printf("f_open %s failed: %d\r\n", path, rc);
        return false;
    }

    const UINT fsize = f_size(&file);
    xil_printf("File size: %u bytes\r\n", fsize);

    try {
        out.resize(fsize);
    } catch (const std::bad_alloc&) {
        xil_printf("bad_alloc: cannot allocate %u bytes\r\n", fsize);
        f_close(&file);
        return false;
    }

    UINT br = 0;
    rc = f_read(&file, out.data(), fsize, &br);
    f_close(&file);

    return true;
}
```

### 2.2.1 함수 흐름

```
1. f_open() → 파일 열기
2. f_size() → 파일 크기 확인
3. vector.resize() → 메모리 할당
4. f_read() → 데이터 읽기
5. f_close() → 파일 닫기
```

### 2.2.2 예외 처리

```cpp
try {
    out.resize(fsize);
} catch (const std::bad_alloc&) {
    xil_printf("bad_alloc: cannot allocate %u bytes\r\n", fsize);
    f_close(&file);
    return false;
}
```

*   **메모리 부족 처리**: 베어메탈 환경에서 힙이 제한적이므로 `bad_alloc` 예외를 처리합니다.
*   **리소스 정리**: 예외 발생 시 파일을 닫아 리소스 누수를 방지합니다.

## 3. 이미지 구조체

```cpp
struct Image {
    int width = 0;
    int height = 0;
    int channels = 0;             // 3 for RGB
    vector<uint8_t> pixels;       // NHWC: rows*cols*channels (uint8)
};
```

*   **NHWC 레이아웃**: 픽셀 데이터는 행 우선(row-major) 순서로 저장됩니다.
*   **인덱싱**: `pixels[(y * width + x) * channels + c]`
*   **동적 크기**: `vector`를 사용하여 다양한 이미지 크기를 지원합니다.

## 4. BMP 파일 헤더 구조체

```cpp
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType;      // 'BM' = 0x4D42
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
struct BMPInfoHeader {
    uint32_t biSize;          // 40 for BITMAPINFOHEADER
    int32_t  biWidth;
    int32_t  biHeight;        // >0 bottom-up; <0 top-down
    uint16_t biPlanes;        // 1
    uint16_t biBitCount;      // 24
    uint32_t biCompression;   // 0 (BI_RGB)
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)
```

### 4.1 구조체 패킹

*   **`#pragma pack(push, 1)`**: 구조체 멤버를 1바이트 경계로 정렬하여 패딩 없이 파일 형식과 정확히 일치시킵니다.
*   **`#pragma pack(pop)`**: 이전 패킹 설정으로 복원합니다.

### 4.2 BMP 파일 헤더 (14 bytes)

| 필드 | 크기 | 설명 |
|------|------|------|
| bfType | 2 | 매직 넘버 ('BM' = 0x4D42) |
| bfSize | 4 | 파일 전체 크기 |
| bfReserved1 | 2 | 예약 (0) |
| bfReserved2 | 2 | 예약 (0) |
| bfOffBits | 4 | 픽셀 데이터 시작 오프셋 |

### 4.3 BMP 정보 헤더 (40 bytes - BITMAPINFOHEADER)

| 필드 | 크기 | 설명 |
|------|------|------|
| biSize | 4 | 헤더 크기 (40) |
| biWidth | 4 | 이미지 너비 (픽셀) |
| biHeight | 4 | 이미지 높이 (양수=bottom-up, 음수=top-down) |
| biPlanes | 2 | 색상 평면 수 (항상 1) |
| biBitCount | 2 | 픽셀당 비트 수 (24 for RGB) |
| biCompression | 4 | 압축 방식 (0 = BI_RGB, 비압축) |
| biSizeImage | 4 | 이미지 데이터 크기 (BI_RGB일 때 0 가능) |
| biXPelsPerMeter | 4 | 수평 해상도 (픽셀/미터) |
| biYPelsPerMeter | 4 | 수직 해상도 (픽셀/미터) |
| biClrUsed | 4 | 사용된 색상 수 |
| biClrImportant | 4 | 중요 색상 수 |

## 5. BMP 로더 함수

```cpp
static bool load_bmp_24(const vector<uint8_t>& buf, Image& img)
{
    // 크기 검증
    if (buf.size() < sizeof(BMPFileHeader) + sizeof(BMPInfoHeader))
    {
        xil_printf("BMP: buffer too small\r\n");
        return false;
    }

    // 파일 헤더 파싱
    const BMPFileHeader* fh = reinterpret_cast<const BMPFileHeader*>(buf.data());
    if (fh->bfType != 0x4D42) // 'BM'
    {
        xil_printf("BMP: bad signature\r\n");
        return false;
    }

    // 정보 헤더 파싱
    const BMPInfoHeader* ih = reinterpret_cast<const BMPInfoHeader*>(
        buf.data() + sizeof(BMPFileHeader));
    if (ih->biSize != 40 || ih->biPlanes != 1 || 
        ih->biBitCount != 24 || ih->biCompression != 0)
    {
        xil_printf("BMP: unsupported header ...\r\n");
        return false;
    }

    // 이미지 크기 및 방향 처리
    const int W = ih->biWidth;
    const int H = (ih->biHeight > 0) ? ih->biHeight : -ih->biHeight;
    const bool bottom_up = (ih->biHeight > 0);

    // 행 스트라이드 계산 (4바이트 정렬)
    const uint32_t row_stride_in = ((W * 3 + 3) & ~3);

    // 픽셀 데이터 변환 (BGR → RGB, bottom-up → top-down)
    img.width = W;
    img.height = H;
    img.channels = 3;
    img.pixels.resize(W * H * 3);

    const uint8_t* pixels_in = buf.data() + fh->bfOffBits;

    for (int y = 0; y < H; ++y)
    {
        const int src_y = bottom_up ? (H - 1 - y) : y;
        const uint8_t* row_in = pixels_in + src_y * row_stride_in;

        for (int x = 0; x < W; ++x)
        {
            const uint8_t b = row_in[x * 3 + 0];
            const uint8_t g = row_in[x * 3 + 1];
            const uint8_t r = row_in[x * 3 + 2];

            uint8_t* p = &img.pixels[(y * W + x) * 3];
            p[0] = r; p[1] = g; p[2] = b; // RGB
        }
    }

    return true;
}
```

### 5.1 유효성 검사

| 검사 항목 | 조건 | 오류 메시지 |
|-----------|------|-------------|
| 버퍼 크기 | `buf.size() < 54` | "buffer too small" |
| 매직 넘버 | `bfType != 0x4D42` | "bad signature" |
| 헤더 크기 | `biSize != 40` | "unsupported header" |
| 색상 평면 | `biPlanes != 1` | "unsupported header" |
| 비트 깊이 | `biBitCount != 24` | "unsupported header" |
| 압축 방식 | `biCompression != 0` | "unsupported header" |
| 픽셀 오프셋 | `bfOffBits >= buf.size()` | "bad pixel offset" |
| 데이터 크기 | `offset + data_size > buf.size()` | "truncated pixel data" |

### 5.2 행 스트라이드 계산

```cpp
const uint32_t row_stride_in = ((W * 3 + 3) & ~3);
```

*   **BMP 규격**: 각 행은 4바이트 경계로 정렬됩니다.
*   **계산 공식**: `(width * bytes_per_pixel + 3) & ~3`
*   **예시**: 너비 10픽셀 × 3바이트 = 30바이트 → 32바이트로 패딩

### 5.3 BGR → RGB 변환 및 수직 뒤집기

```cpp
for (int y = 0; y < H; ++y)
{
    const int src_y = bottom_up ? (H - 1 - y) : y;
    const uint8_t* row_in = pixels_in + src_y * row_stride_in;

    for (int x = 0; x < W; ++x)
    {
        const uint8_t b = row_in[x * 3 + 0];
        const uint8_t g = row_in[x * 3 + 1];
        const uint8_t r = row_in[x * 3 + 2];

        uint8_t* p = &img.pixels[(y * W + x) * 3];
        p[0] = r; p[1] = g; p[2] = b; // RGB
    }
}
```

*   **BGR → RGB**: BMP는 BGR 순서로 저장되므로 RGB로 변환합니다.
*   **Bottom-up 처리**: `biHeight > 0`이면 이미지가 아래에서 위로 저장되어 있으므로 뒤집습니다.

## 6. 메인 함수

```cpp
int main()
{
    xil_printf("SD BMP Loader (bare-metal)\r\n");

    if (!sd_mount())
    {
        xil_printf("Mount failed. Ensure SD is inserted and PS SD is enabled.\r\n");
        return -1;
    }

    vector<uint8_t> bmp_buf;
    const char* path = "0:/test.bmp";
    if (!sd_read_all(path, bmp_buf))
    {
        xil_printf("Failed to read %s\r\n", path);
        return -1;
    }

    Image img;
    if (!load_bmp_24(bmp_buf, img))
    {
        xil_printf("BMP parse failed\r\n");
        return -1;
    }

    xil_printf("Loaded BMP: %dx%d, %d channels\r\n", img.width, img.height, img.channels);

    // Example: print first pixel RGB
    if (!img.pixels.empty())
    {
        uint8_t r = img.pixels[0], g = img.pixels[1], b = img.pixels[2];
        xil_printf("Top-left pixel RGB = (%u,%u,%u)\r\n", r, g, b);
    }

    xil_printf("Done.\r\n");
    while (1) { /* idle */ }
    return 0;
}
```

### 6.1 실행 흐름

```
1. SD 카드 마운트
2. BMP 파일 읽기 (0:/test.bmp)
3. BMP 파싱 및 변환
4. 이미지 정보 출력
5. 첫 번째 픽셀 RGB 값 출력
6. 무한 루프 (idle)
```

---

# 코드 품질 및 최적화 고려사항

## 장점

1.  **견고한 오류 처리**: 각 단계에서 오류를 체크하고 적절한 메시지를 출력합니다.

2.  **메모리 안전성**: `bad_alloc` 예외를 처리하여 메모리 부족 상황에 대응합니다.

3.  **표준 BMP 지원**: 24비트 비압축 BMP의 표준 형식을 올바르게 처리합니다.

4.  **행 패딩 처리**: BMP의 4바이트 행 정렬을 올바르게 계산합니다.

5.  **Bottom-up 처리**: BMP의 수직 방향 저장 방식을 올바르게 처리합니다.

## 개선 제안

### 1. 다양한 BMP 형식 지원

```cpp
// 현재: 24비트만 지원
if (ih->biBitCount != 24 || ih->biCompression != 0)

// 개선: 8비트 그레이스케일, 32비트 RGBA 지원
switch (ih->biBitCount) {
    case 8:  return load_bmp_8(buf, img);   // 그레이스케일
    case 24: return load_bmp_24(buf, img);  // RGB
    case 32: return load_bmp_32(buf, img);  // RGBA
    default:
        xil_printf("Unsupported bit depth: %d\r\n", ih->biBitCount);
        return false;
}
```

### 2. 캐시 관리 추가

```cpp
// 이미지 로드 후 캐시 플러시 (DMA나 PL에서 접근할 경우)
Xil_DCacheFlushRange((UINTPTR)img.pixels.data(), img.pixels.size());
```

### 3. 파일 경로 파라미터화

```cpp
// 현재: 하드코딩된 경로
const char* path = "0:/test.bmp";

// 개선: 매크로 또는 설정 파일 사용
#ifndef BMP_FILE_PATH
#define BMP_FILE_PATH "0:/test.bmp"
#endif
const char* path = BMP_FILE_PATH;
```

### 4. 이미지 리사이징 함수 추가

```cpp
// 신경망 입력 크기에 맞게 리사이즈
bool resize_image(const Image& src, Image& dst, int new_width, int new_height) {
    dst.width = new_width;
    dst.height = new_height;
    dst.channels = src.channels;
    dst.pixels.resize(new_width * new_height * src.channels);
    
    // 최근접 이웃 보간
    for (int y = 0; y < new_height; ++y) {
        int src_y = y * src.height / new_height;
        for (int x = 0; x < new_width; ++x) {
            int src_x = x * src.width / new_width;
            for (int c = 0; c < src.channels; ++c) {
                dst.pixels[(y * new_width + x) * src.channels + c] =
                    src.pixels[(src_y * src.width + src_x) * src.channels + c];
            }
        }
    }
    return true;
}
```

### 5. JPEG 지원 추가 (확장)

```cpp
// libjpeg-turbo 또는 stb_image 사용 고려
// 베어메탈 환경에서는 stb_image가 더 적합
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool load_jpeg(const vector<uint8_t>& buf, Image& img) {
    int w, h, c;
    uint8_t* data = stbi_load_from_memory(buf.data(), buf.size(), &w, &h, &c, 3);
    if (!data) return false;
    
    img.width = w;
    img.height = h;
    img.channels = 3;
    img.pixels.assign(data, data + w * h * 3);
    stbi_image_free(data);
    return true;
}
```

### 6. 메모리 최적화 (대용량 이미지)

```cpp
// 현재: 전체 파일을 메모리에 로드
vector<uint8_t> bmp_buf;
sd_read_all(path, bmp_buf);

// 개선: 스트리밍 방식 (메모리 절약)
bool load_bmp_24_streaming(const char* path, Image& img) {
    FIL file;
    f_open(&file, path, FA_READ);
    
    // 헤더만 읽기
    BMPFileHeader fh;
    BMPInfoHeader ih;
    UINT br;
    f_read(&file, &fh, sizeof(fh), &br);
    f_read(&file, &ih, sizeof(ih), &br);
    
    // 픽셀 데이터 위치로 이동
    f_lseek(&file, fh.bfOffBits);
    
    // 한 행씩 읽어서 처리
    const uint32_t row_stride = ((ih.biWidth * 3 + 3) & ~3);
    vector<uint8_t> row_buf(row_stride);
    
    img.pixels.resize(ih.biWidth * ih.biHeight * 3);
    
    for (int y = 0; y < ih.biHeight; ++y) {
        f_read(&file, row_buf.data(), row_stride, &br);
        // BGR → RGB 변환 및 저장
        // ...
    }
    
    f_close(&file);
    return true;
}
```

---

# 결론

`zcu104-baremetal-imgio/` 프로젝트는 ZCU104 보드의 베어메탈 환경에서 SD 카드로부터 BMP 이미지를 로드하는 완전하고 견고한 구현을 제공합니다.

*   **SD 카드 접근**: FatFs 라이브러리를 사용하여 SD 카드를 마운트하고 파일을 읽습니다.

*   **BMP 파싱**: 24비트 비압축 BMP 형식을 지원하며, 행 패딩, BGR→RGB 변환, bottom-up 이미지 처리를 올바르게 수행합니다.

*   **C++ 활용**: 베어메탈 환경에서도 `std::vector`와 예외 처리를 활용하여 안전하고 깔끔한 코드를 작성했습니다.

이 코드는 `zcu104-a53-mobilenetv1-baseline` 프로젝트의 이미지 로딩 부분과 유사한 패턴을 따르며, 이미지 처리 파이프라인의 첫 단계로 사용될 수 있습니다. 위의 개선 제안들을 적용하면 더 다양한 이미지 형식을 지원하고, 메모리 효율성을 높이며, 신경망 추론 파이프라인과의 통합을 용이하게 할 수 있습니다.
