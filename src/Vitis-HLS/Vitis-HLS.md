# `dinh/zybo_memcopy_accel_interrupt/src/Vitis-HLS/` 코드 리뷰 보고서

이 보고서는 `zybo_memcopy_accel_interrupt` 프로젝트의 Vitis HLS 소스 코드에 대한 상세한 코드 리뷰 결과를 담고 있습니다. 이 폴더에는 메모리 복사 가속기의 HLS 구현(`memcopy_accel.cpp`), 테스트벤치(`memcopy_accel_test.cpp`), 그리고 테스트 데이터 헤더(`memcopy_accel_tb_data.h`)가 포함되어 있습니다.

---

# `memcopy_accel.cpp` 코드 리뷰 보고서

이 파일은 Vitis HLS를 사용하여 DDR 메모리 간 데이터 복사를 가속화하는 하드웨어 IP의 핵심 구현입니다.

## 1. 헤더 및 상수 정의

```cpp
#include <stdint.h>
#include <hls_stream.h>
#include <ap_int.h>

#define BURST_LEN 32  // number of 32-bit words per burst
```

*   **`stdint.h`**: `uint32_t`와 같은 고정 크기 정수 타입을 정의합니다.
*   **`hls_stream.h`**: Vitis HLS의 스트리밍 데이터 인터페이스를 위한 헤더입니다 (이 코드에서는 직접 사용되지 않지만 일반적인 HLS 프로젝트에서 포함됩니다).
*   **`ap_int.h`**: 임의 정밀도 정수 타입을 제공합니다 (이 코드에서는 직접 사용되지 않음).
*   **`BURST_LEN`**: 한 번의 버스트 전송에서 처리할 32비트 워드의 개수입니다. 32워드 = 128바이트 버스트.

## 2. 함수 시그니처 및 인터페이스 지시자

```cpp
void memcopy_accel(
    uint32_t* src,   // AXI4 Master port for source DDR
    uint32_t* dst,   // AXI4 Master port for destination DDR
    uint32_t len     // Number of bytes to copy
) {
#pragma HLS INTERFACE m_axi     port=src offset=slave bundle=AXI_SRC depth=1024
#pragma HLS INTERFACE m_axi     port=dst offset=slave bundle=AXI_DST depth=1024
#pragma HLS INTERFACE s_axilite port=src       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=dst       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=len       bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return    bundle=CTRL_BUS
```

### 2.1 인터페이스 분석

| 포트 | 인터페이스 유형 | 번들 | 설명 |
|------|----------------|------|------|
| `src` | m_axi + s_axilite | AXI_SRC + CTRL_BUS | 소스 DDR 주소 (AXI4 마스터로 읽기) |
| `dst` | m_axi + s_axilite | AXI_DST + CTRL_BUS | 목적지 DDR 주소 (AXI4 마스터로 쓰기) |
| `len` | s_axilite | CTRL_BUS | 복사할 바이트 수 |
| `return` | s_axilite | CTRL_BUS | 제어 레지스터 (ap_start, ap_done 등) |

### 2.2 인터페이스 설계 특징

*   **듀얼 AXI4 마스터**: `src`와 `dst`가 별도의 AXI4 마스터 번들(`AXI_SRC`, `AXI_DST`)로 분리되어 있어 동시 읽기/쓰기가 가능합니다.
*   **오프셋 슬레이브**: `offset=slave`는 주소가 PS(프로세서 시스템)에서 AXI-Lite를 통해 설정됨을 의미합니다.
*   **깊이 힌트**: `depth=1024`는 합성 도구에게 예상 전송 깊이를 알려줍니다.
*   **통합 제어 버스**: 모든 파라미터와 제어 신호가 단일 `CTRL_BUS` 번들로 통합되어 드라이버 개발이 단순화됩니다.

## 3. 핵심 로직

### 3.1 초기화 및 버퍼

```cpp
uint32_t num_words = len >> 2;  // divide by 4
uint32_t buffer[BURST_LEN];
#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1
```

*   **워드 계산**: `len >> 2`는 바이트 수를 4로 나누어 32비트 워드 수를 계산합니다.
*   **버퍼 분할**: `ARRAY_PARTITION complete`는 버퍼의 모든 요소를 개별 레지스터로 분할하여 동시 접근을 가능하게 합니다. 이는 버스트 전송의 병렬 처리에 필수적입니다.

### 3.2 브랜치리스(Branchless) 청크 계산

```cpp
copy_loop:
    for (uint32_t i = 0; i < num_words; i += BURST_LEN) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=1024
#pragma HLS PIPELINE II=1

        // 브랜치리스 청크 크기 계산
        uint32_t diff = num_words - i;
        uint32_t mask = (diff >= BURST_LEN); // 1 if full burst, 0 otherwise
        mask = -mask; // 0xFFFFFFFF if true, 0x0 if false
        uint32_t chunk = (mask & BURST_LEN) | (~mask & diff);
```

*   **기존 방식 (주석 처리됨)**:
    ```cpp
    uint32_t chunk = (i + BURST_LEN <= num_words) ? BURST_LEN : (num_words - i);
    ```
    이 조건문은 HLS에서 비효율적인 하드웨어를 생성할 수 있습니다.

*   **브랜치리스 구현**:
    1.  `diff = num_words - i`: 남은 워드 수 계산
    2.  `mask = (diff >= BURST_LEN)`: 전체 버스트 가능 여부 (0 또는 1)
    3.  `mask = -mask`: 비트 마스크로 변환 (0xFFFFFFFF 또는 0x0)
    4.  `chunk = (mask & BURST_LEN) | (~mask & diff)`: 조건 없이 청크 크기 선택

*   **HLS 최적화 효과**: 분기 명령어 없이 순수 조합 논리로 구현되어 파이프라인 II=1을 달성하기 쉬워집니다.

### 3.3 읽기 루프

```cpp
read_loop:
    for (uint32_t j = 0; j < BURST_LEN; j++) {
#pragma HLS UNROLL
        bool valid = (j < chunk);
        uint32_t vmask = -((uint32_t)valid);
        buffer[j] = src[i + j] & vmask;  // only valid data copied
    }
```

*   **완전 언롤링**: `#pragma HLS UNROLL`로 32개의 읽기 연산이 모두 병렬화됩니다.
*   **조건부 마스킹**: 유효하지 않은 인덱스(j >= chunk)에서 읽은 데이터는 마스크로 0이 됩니다.
*   **버스트 읽기**: 연속된 메모리 접근으로 AXI4 버스트 전송이 활성화됩니다.

### 3.4 쓰기 루프

```cpp
write_loop:
    for (uint32_t j = 0; j < BURST_LEN; j++) {
#pragma HLS UNROLL
        bool valid = (j < chunk);
        uint32_t vmask = -((uint32_t)valid);
        dst[i + j] = buffer[j] & vmask;  // only valid data written
    }
```

*   **동일한 패턴**: 읽기와 동일한 브랜치리스 마스킹 패턴을 사용합니다.
*   **버스트 쓰기**: 연속된 메모리 쓰기로 AXI4 버스트 전송이 활성화됩니다.

## 4. HLS 지시자 분석

| 지시자 | 적용 대상 | 효과 |
|--------|----------|------|
| `INTERFACE m_axi` | src, dst | AXI4 마스터 인터페이스 생성 |
| `INTERFACE s_axilite` | 모든 파라미터 | AXI-Lite 제어 인터페이스 생성 |
| `ARRAY_PARTITION complete` | buffer | 32개 레지스터로 완전 분할 |
| `LOOP_TRIPCOUNT` | copy_loop | 합성 보고서의 지연 시간 추정용 |
| `PIPELINE II=1` | copy_loop | 매 사이클 새로운 반복 시작 |
| `UNROLL` | read_loop, write_loop | 루프 완전 전개 |

## 5. 성능 분석

### 5.1 이론적 처리량

*   **버스트 크기**: 32 × 4 = 128 바이트/버스트
*   **파이프라인 II=1**: 매 사이클 새로운 버스트 시작 가능
*   **듀얼 AXI 포트**: 읽기와 쓰기가 별도 포트로 동시 진행 가능

### 5.2 제한 사항

*   **4바이트 정렬 필수**: `len >> 2`로 워드 수를 계산하므로 입력 길이는 4의 배수여야 합니다.
*   **마스킹 오버헤드**: 마지막 버스트에서 유효하지 않은 데이터도 읽고 쓰지만, 마스크로 0이 됩니다. 목적지 메모리에 불필요한 쓰기가 발생할 수 있습니다.

---

# `memcopy_accel_test.cpp` 코드 리뷰 보고서

이 파일은 `memcopy_accel` 함수의 C 시뮬레이션 테스트벤치입니다.

## 1. 테스트벤치 구조

```cpp
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "memcopy_accel_tb_data.h"

void memcopy_accel(uint32_t* src, uint32_t* dst, uint32_t len);

int main() {
    const int SIZE = 64;
    uint32_t src[SIZE];
    uint32_t dst[SIZE];

    // Initialize source data
    for (int i = 0; i < SIZE; i++)
        src[i] = i + 1;

    // Run accelerator
    memcopy_accel(src, dst, SIZE * 4);

    // Verify
    int errors = 0;
    for (int i = 0; i < SIZE; i++) {
        if (src[i] != dst[i]) {
            std::cout << "Mismatch at " << i << ": " << src[i]
                      << " != " << dst[i] << std::endl;
            errors++;
        }
    }

    if (errors == 0)
        std::cout << "Test passed!" << std::endl;
    else
        std::cout << "Test failed with " << errors << " mismatches." << std::endl;

    return errors;
}
```

### 1.1 테스트 설정

*   **데이터 크기**: 64개 워드 = 256 바이트
*   **소스 패턴**: 1, 2, 3, ..., 64 (순차 증가)
*   **목적지 초기화**: 기본값 (0 또는 미정의)

### 1.2 검증 로직

*   **비교**: `src[i] != dst[i]` 조건으로 각 요소 비교
*   **오류 보고**: 불일치 위치와 값 출력
*   **종료 코드**: 오류 수 반환 (0 = 성공)

## 2. 테스트 데이터 헤더

```cpp
// memcopy_accel_tb_data.h
#pragma once
#include <stdint.h>

// Optional external test data file (if needed later)
// For now, it's empty. You can define static arrays here for complex tests.
```

*   **현재 상태**: 빈 파일로 향후 복잡한 테스트 데이터를 위한 플레이스홀더입니다.
*   **사용 예시**: 대용량 테스트 패턴, 특수 케이스 데이터 등을 정의할 수 있습니다.

---

# 코드 품질 및 최적화 고려사항

## 장점

1.  **브랜치리스 설계**: 조건문을 비트 연산으로 대체하여 HLS 파이프라이닝에 최적화되었습니다.

2.  **듀얼 AXI 포트**: 읽기와 쓰기가 별도의 AXI4 마스터 포트를 사용하여 대역폭 활용도를 높입니다.

3.  **버퍼 완전 분할**: 버스트 버퍼가 완전히 분할되어 모든 요소에 동시 접근이 가능합니다.

4.  **통합 제어 버스**: 단일 AXI-Lite 인터페이스로 모든 파라미터를 제어하여 드라이버 복잡도를 줄입니다.

## 개선 제안

### 1. 마지막 버스트 처리 최적화

```cpp
// 현재 코드 - 마지막 버스트에서 유효하지 않은 데이터도 읽고 씀
dst[i + j] = buffer[j] & vmask;
```

*   **문제점**: 마지막 버스트에서 `chunk < BURST_LEN`일 때, 유효하지 않은 인덱스에도 메모리 쓰기가 발생합니다 (값은 0).
*   **개선 방안**: 유효한 범위만 쓰도록 쓰기 루프를 수정합니다.

```cpp
// 개선된 코드
write_loop:
    for (uint32_t j = 0; j < chunk; j++) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=32
        dst[i + j] = buffer[j];
    }
```

단, 이 방식은 HLS에서 가변 루프 바운드로 인해 효율이 떨어질 수 있으므로 트레이드오프를 고려해야 합니다.

### 2. 정렬 검사 추가

```cpp
// len이 4의 배수가 아닌 경우 처리
if (len & 0x3) {
    // 오류 처리 또는 패딩
}
```

### 3. 테스트벤치 확장

```cpp
// 다양한 케이스 테스트
void test_edge_cases() {
    // 1. 정확히 BURST_LEN 배수인 경우
    test_copy(BURST_LEN * 4);
    
    // 2. BURST_LEN 배수가 아닌 경우
    test_copy(BURST_LEN * 4 + 7);
    
    // 3. 작은 크기
    test_copy(4);
    
    // 4. 큰 크기
    test_copy(4096);
}
```

### 4. 바이트 단위 복사 지원

현재 구현은 4바이트 정렬된 데이터만 지원합니다. 바이트 단위 복사를 지원하려면 첫/마지막 워드의 부분 바이트 처리 로직이 필요합니다.

---

# 결론

`Vitis-HLS/` 폴더의 코드는 Zybo Z7-10 보드에서 DDR 메모리 간 고속 데이터 복사를 수행하는 하드웨어 가속기를 구현합니다.

*   **`memcopy_accel.cpp`**: 브랜치리스 설계와 완전 언롤링된 버스트 전송으로 HLS 최적화에 적합한 메모리 복사 가속기를 구현합니다. 듀얼 AXI4 마스터 포트를 사용하여 높은 처리량을 달성합니다.

*   **`memcopy_accel_test.cpp`**: 기본적인 기능 검증을 위한 C 시뮬레이션 테스트벤치를 제공합니다.

이 구현은 HLS의 일반적인 최적화 기법들(브랜치리스 코딩, 배열 분할, 파이프라이닝, 언롤링)을 잘 활용하고 있으며, `Vitis-BareMetal/` 폴더의 드라이버 코드와 함께 사용되어 완전한 하드웨어-소프트웨어 시스템을 구성합니다.
