# Vitis HLS 개발 상세 가이드

> **도구:** Vitis HLS 2022.1  
> **목적:** C/C++ 코드를 FPGA 하드웨어 가속기로 변환

---

## 목차

1. [HLS 개요](#1-hls-개요)
2. [프로젝트 설정](#2-프로젝트-설정)
3. [인터페이스 설계](#3-인터페이스-설계)
4. [최적화 기법](#4-최적화-기법)
5. [검증 워크플로우](#5-검증-워크플로우)
6. [Vivado 통합](#6-vivado-통합)
7. [디버깅 기법](#7-디버깅-기법)
8. [고급 주제](#8-고급-주제)

---

## 1. HLS 개요

### 1.1 HLS란?

**High-Level Synthesis (HLS)**는 C/C++ 코드를 RTL (Register Transfer Level) 하드웨어로 자동 변환하는 기술입니다.

```
C/C++ 코드 → [Vitis HLS] → RTL (Verilog/VHDL) → [Vivado] → Bitstream
```

### 1.2 HLS의 장점

| 장점 | 설명 |
|------|------|
| 개발 속도 | RTL 대비 5-10배 빠른 개발 |
| 설계 탐색 | Pragma로 빠른 아키텍처 변경 |
| 검증 용이 | C Testbench 재사용 |
| 유지보수 | 고수준 코드로 가독성 향상 |

### 1.3 HLS의 제약

- 동적 메모리 할당 (malloc) 불가
- 재귀 함수 제한적 지원
- 포인터 연산 제한
- 시스템 콜 (printf, file I/O) 불가

---

## 2. 프로젝트 설정

### 2.1 새 프로젝트 생성

```
File → New Project...
├── Project Name: my_accel
├── Location: <workspace>
├── Next

Add Design Files:
├── my_accel.cpp
├── my_accel.h
└── Top Function: my_accel

Add Testbench Files:
├── my_accel_tb.cpp
└── test_data.h

Solution Configuration:
├── Part: xc7z010clg400-1 (Zybo Z7-10)
├── Clock Period: 10ns (100MHz)
└── Clock Uncertainty: 12.5% (기본값)
```

### 2.2 프로젝트 구조

```
my_accel/
├── src/
│   ├── my_accel.cpp      # 합성 대상 코드
│   └── my_accel.h        # 헤더
├── tb/
│   ├── my_accel_tb.cpp   # 테스트벤치
│   └── test_data.h       # 테스트 데이터
└── solution1/
    ├── syn/              # 합성 결과
    ├── sim/              # 시뮬레이션 결과
    └── impl/
        └── ip/           # IP 출력
```

### 2.3 디렉티브 파일 vs 소스 내 Pragma

**방법 1: 소스 내 Pragma (권장)**
```cpp
void my_func(int* data) {
    #pragma HLS INTERFACE m_axi port=data
    #pragma HLS PIPELINE II=1
    // ...
}
```

**방법 2: 디렉티브 파일 (directives.tcl)**
```tcl
set_directive_interface -mode m_axi "my_func" data
set_directive_pipeline "my_func/loop" -II 1
```

---

## 3. 인터페이스 설계

### 3.1 인터페이스 유형

| 인터페이스 | Pragma | 용도 |
|------------|--------|------|
| **AXI4 Master** | `m_axi` | DDR 메모리 접근 |
| **AXI4-Lite** | `s_axilite` | 제어 레지스터 |
| **AXI4-Stream** | `axis` | 스트리밍 데이터 |
| **BRAM** | `bram` | 블록 RAM 인터페이스 |
| **AP 핸드셰이크** | `ap_hs` | 단순 핸드셰이크 |
| **AP 없음** | `ap_none` | 와이어 (조합 논리) |

### 3.2 AXI4 Master (m_axi)

**DDR 메모리 읽기/쓰기에 사용**

```cpp
void memcopy(
    uint32_t* src,    // DDR에서 읽기
    uint32_t* dst,    // DDR에 쓰기
    uint32_t len
) {
    // AXI4 Master 포트 (메모리 접근)
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=SRC depth=1024
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=DST depth=1024
    
    // AXI-Lite 슬레이브 (주소 레지스터)
    #pragma HLS INTERFACE s_axilite port=src bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=dst bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=len bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL
}
```

**m_axi 옵션:**

| 옵션 | 설명 |
|------|------|
| `offset=slave` | s_axilite로 주소 설정 |
| `offset=direct` | 포트로 직접 주소 입력 |
| `bundle=NAME` | AXI 버스 그룹 |
| `depth=N` | Co-sim용 메모리 깊이 |
| `max_widen_bitwidth=N` | 최대 데이터 폭 |
| `num_read_outstanding=N` | 미처리 읽기 수 |
| `num_write_outstanding=N` | 미처리 쓰기 수 |

### 3.3 AXI4-Lite (s_axilite)

**제어 레지스터에 사용**

```cpp
void accel(int config1, int config2, int* result) {
    #pragma HLS INTERFACE s_axilite port=config1 bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=config2 bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=result bundle=CTRL
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL
}
```

**자동 생성 레지스터 맵:**

| 오프셋 | 레지스터 | 설명 |
|--------|----------|------|
| 0x00 | CTRL | ap_start, ap_done, ap_idle |
| 0x04 | GIE | Global Interrupt Enable |
| 0x08 | IER | IP Interrupt Enable |
| 0x0C | ISR | IP Interrupt Status |
| 0x10+ | 인수 | 함수 인수 레지스터 |

### 3.4 AXI4-Stream (axis)

**스트리밍 데이터에 사용**

```cpp
#include "ap_axi_sdata.h"
#include "hls_stream.h"

typedef ap_axis<32, 0, 0, 0> axis_t;  // 32-bit, no side-channel

void stream_proc(
    hls::stream<axis_t>& in,
    hls::stream<axis_t>& out
) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL
    
    axis_t pkt;
    pkt = in.read();
    pkt.data = pkt.data + 1;  // 처리
    out.write(pkt);
}
```

**ap_axis 템플릿:**
```cpp
ap_axis<DATA_WIDTH, USER_WIDTH, ID_WIDTH, DEST_WIDTH>
```

### 3.5 BRAM 인터페이스

```cpp
void bram_access(int mem[1024]) {
    #pragma HLS INTERFACE bram port=mem
    // mem은 외부 BRAM에 연결됨
}
```

---

## 4. 최적화 기법

### 4.1 파이프라인 (PIPELINE)

**루프를 파이프라인화하여 처리량 증가**

```cpp
for (int i = 0; i < N; i++) {
    #pragma HLS PIPELINE II=1  // 매 사이클 새 반복
    out[i] = in[i] * 2;
}
```

| II (Initiation Interval) | 의미 |
|--------------------------|------|
| II=1 | 매 사이클 새 데이터 |
| II=2 | 2사이클마다 새 데이터 |
| II=N | 완전 순차 실행 |

### 4.2 언롤 (UNROLL)

**루프를 병렬화**

```cpp
for (int i = 0; i < 8; i++) {
    #pragma HLS UNROLL factor=4  // 4배 병렬화
    acc += data[i];
}
```

| 옵션 | 효과 |
|------|------|
| `UNROLL` | 완전 전개 |
| `UNROLL factor=N` | N배 부분 전개 |
| `UNROLL skip_exit_check` | 종료 조건 생략 |

### 4.3 배열 분할 (ARRAY_PARTITION)

**배열을 분할하여 병렬 접근**

```cpp
int buffer[32];
#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1
// 32개 독립 레지스터로 분할
```

| 타입 | 설명 |
|------|------|
| `complete` | 완전 분할 (레지스터화) |
| `cyclic factor=N` | 순환 분할 |
| `block factor=N` | 블록 분할 |

### 4.4 데이터플로우 (DATAFLOW)

**태스크 수준 파이프라인**

```cpp
void top(int* in, int* out) {
    #pragma HLS DATAFLOW
    
    int tmp1[N], tmp2[N];
    
    stage1(in, tmp1);   // 동시 실행 가능
    stage2(tmp1, tmp2); // (FIFO로 연결)
    stage3(tmp2, out);
}
```

### 4.5 루프 최적화 조합

```cpp
// 중첩 루프 최적화 예시
for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
        #pragma HLS PIPELINE II=1
        for (int c = 0; c < C; c++) {
            #pragma HLS UNROLL
            // 채널 병렬 처리
        }
    }
}
```

---

## 5. 검증 워크플로우

### 5.1 C Simulation

**기능 검증 (빠름)**

```
Project → Run C Simulation
└── 또는: Ctrl+Shift+S
```

**테스트벤치 구조:**
```cpp
int main() {
    // 1. 테스트 데이터 준비
    int input[N], output[N], golden[N];
    
    // 2. 황금 참조 계산
    for (int i = 0; i < N; i++)
        golden[i] = input[i] * 2;
    
    // 3. DUT 호출
    my_accel(input, output);
    
    // 4. 결과 비교
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (output[i] != golden[i]) {
            errors++;
            printf("Mismatch at %d: got %d, expected %d\n",
                   i, output[i], golden[i]);
        }
    }
    
    // 5. 결과 반환 (0 = PASS)
    return errors;
}
```

### 5.2 C Synthesis

**RTL 생성**

```
Solution → Run C Synthesis
└── 또는: Ctrl+Shift+C
```

**리포트 확인 포인트:**

| 항목 | 확인 내용 |
|------|----------|
| Timing | 타겟 클럭 만족 여부 |
| Latency | 총 사이클 수 |
| Interval | II (처리량) |
| Resources | BRAM, DSP, FF, LUT 사용량 |

### 5.3 C/RTL Co-Simulation

**RTL 기능 검증**

```
Solution → Run C/RTL Cosimulation
├── RTL Selection: Verilog
├── Dump Trace: all (파형 확인용)
└── Optimizing Compile: ON
```

**Co-sim 옵션:**

| 옵션 | 설명 |
|------|------|
| `Verilog` / `VHDL` | RTL 언어 선택 |
| `Dump Trace: all` | 파형 파일 생성 |
| `setup only` | 테스트벤치만 생성 |

### 5.4 Export RTL

**IP Catalog로 내보내기**

```
Solution → Export RTL
├── Format: IP Catalog
├── Flow Target: Vivado IP Flow Target
└── Configuration: 기본값
```

---

## 6. Vivado 통합

### 6.1 IP Repository 추가

```
Vivado → Settings → IP → Repository → Add
└── <HLS_Project>/solution1/impl/ip
```

### 6.2 Block Design에 추가

```
Diagram → Add IP → <your_ip_name>
```

### 6.3 연결 패턴

**패턴 1: AXI Master + AXI-Lite**
```
PS ─┬─ M_AXI_GP0 ─── AXI Interconnect ─── HLS IP (s_axi_CTRL)
    │
    └─ S_AXI_HP0 ─── AXI Interconnect ─┬─ HLS IP (m_axi_SRC) 
                                       └─ HLS IP (m_axi_DST)
```

**패턴 2: AXI-Stream + DMA**
```
PS ─── M_AXI_GP0 ─── AXI Interconnect ─┬─ AXI DMA
                                       └─ HLS IP (s_axi_CTRL)

DDR ─── AXI DMA ─── MM2S (stream) ─── HLS IP ─── S2MM (stream) ─── DDR
```

### 6.4 인터럽트 연결

```
HLS IP.interrupt ─── xlconcat.In[0] ─── PS.IRQ_F2P[0:0]
```

### 6.5 주소 할당

```
Address Editor 탭에서 확인:
├── HLS_IP.s_axi_CTRL: 0x40000000
├── HLS_IP.m_axi_*: Auto (전체 DDR 범위)
```

---

## 7. 디버깅 기법

### 7.1 합성 경고 해석

| 경고 | 의미 | 해결 |
|------|------|------|
| `II violation` | 목표 II 미달성 | 의존성 확인, 리소스 추가 |
| `unroll failed` | 루프 경계 불명확 | 경계를 상수로 |
| `unable to schedule` | 스케줄링 실패 | 의존성 분석 |

### 7.2 Co-sim 파형 분석

```
Solution → Open Wave Viewer
```

**확인 포인트:**
- ap_start → ap_done 지연
- AXI 트랜잭션 타이밍
- 데이터 흐름

### 7.3 리소스 사용량 분석

```
합성 리포트 → Utilization Estimates

BRAM_18K: 10 (5%)
DSP48E:   4 (5%)
FF:       2000 (3%)
LUT:      1500 (9%)
```

### 7.4 레이턴시 분석

```
합성 리포트 → Performance Estimates

Loop 'main_loop':
├── Latency (cycles): 1024
├── Interval (cycles): 1024
├── Pipeline Type: no
└── Trip Count: 1024
```

---

## 8. 고급 주제

### 8.1 메모리 버스트 최적화

```cpp
// 비효율적: 개별 접근
for (int i = 0; i < N; i++) {
    out[i] = in[i];  // N번의 AXI 트랜잭션
}

// 효율적: 버스트 접근
int buffer[BURST_LEN];
for (int i = 0; i < N; i += BURST_LEN) {
    // 버스트 읽기
    for (int j = 0; j < BURST_LEN; j++)
        buffer[j] = in[i + j];
    
    // 버스트 쓰기
    for (int j = 0; j < BURST_LEN; j++)
        out[i + j] = buffer[j];
}
```

### 8.2 라인 버퍼 (스텐실 처리)

```cpp
template<int ROWS, int COLS>
void line_buffer_example(
    hls::stream<uint8_t>& in,
    hls::stream<uint8_t>& out
) {
    uint8_t line_buf[3][COLS];  // 3줄 버퍼
    #pragma HLS ARRAY_PARTITION variable=line_buf complete dim=1
    
    for (int row = 0; row < ROWS + 2; row++) {
        for (int col = 0; col < COLS; col++) {
            #pragma HLS PIPELINE II=1
            
            // 라인 버퍼 시프트
            line_buf[0][col] = line_buf[1][col];
            line_buf[1][col] = line_buf[2][col];
            
            // 새 픽셀 읽기
            if (row < ROWS)
                line_buf[2][col] = in.read();
            
            // 3x3 윈도우 처리 (row >= 2)
            if (row >= 2 && col >= 2) {
                // 스텐실 연산
                int sum = 0;
                for (int ky = 0; ky < 3; ky++)
                    for (int kx = 0; kx < 3; kx++)
                        sum += line_buf[ky][col - 2 + kx];
                out.write(sum / 9);
            }
        }
    }
}
```

### 8.3 고정 소수점 연산

```cpp
#include "ap_fixed.h"

// Q8.8 형식 (8비트 정수, 8비트 소수)
typedef ap_fixed<16, 8> fixed_t;

fixed_t multiply(fixed_t a, fixed_t b) {
    return a * b;  // 자동 오버플로우 처리
}
```

### 8.4 비트 수준 최적화

```cpp
#include "ap_int.h"

// 12비트 정수
typedef ap_uint<12> uint12_t;

// 비트 연산
ap_uint<32> pack_4x8(ap_uint<8> a, ap_uint<8> b, 
                     ap_uint<8> c, ap_uint<8> d) {
    return (a, b, c, d);  // 연결
}

// 비트 추출
ap_uint<8> extract(ap_uint<32> val, int idx) {
    return val.range(idx*8+7, idx*8);
}
```

---

## 부록: HLS Pragma 빠른 참조

### 인터페이스 Pragma

```cpp
#pragma HLS INTERFACE m_axi port=<port> offset=slave bundle=<name> depth=<n>
#pragma HLS INTERFACE s_axilite port=<port> bundle=<name>
#pragma HLS INTERFACE axis port=<port>
#pragma HLS INTERFACE bram port=<port>
#pragma HLS INTERFACE ap_none port=<port>
```

### 최적화 Pragma

```cpp
#pragma HLS PIPELINE II=<n>
#pragma HLS UNROLL factor=<n>
#pragma HLS ARRAY_PARTITION variable=<var> complete dim=<n>
#pragma HLS ARRAY_RESHAPE variable=<var> complete dim=<n>
#pragma HLS DATAFLOW
#pragma HLS INLINE
#pragma HLS ALLOCATION instances=<op> limit=<n>
```

### 리소스 Pragma

```cpp
#pragma HLS RESOURCE variable=<var> core=<core>
#pragma HLS BIND_STORAGE variable=<var> type=<type>
#pragma HLS BIND_OP variable=<var> op=<op> impl=<impl>
```

---

*문서 작성일: 2026-02-06*  
*관련 문서: FPGA_Learning_Guide.md, Lab_Practice_Guide.md*
