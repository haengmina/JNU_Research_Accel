# FPGA 가속기 개발 종합 학습 가이드

> **대상 보드:** ZCU104 (Zynq UltraScale+ MPSoC), Zybo Z7-10 (Zynq-7000)  
> **개발 도구:** Vivado/Vitis 2022.1, Vitis HLS  
> **최종 목표:** FPGA 가속기 설계부터 베어메탈 애플리케이션 통합까지 마스터

---

## 목차

1. [학습 로드맵](#1-학습-로드맵)
2. [사전 준비물](#2-사전-준비물)
3. [개발 환경 구축](#3-개발-환경-구축)
4. [프로젝트 개요](#4-프로젝트-개요)
5. [학습 단계별 상세 가이드](#5-학습-단계별-상세-가이드)
6. [핵심 개념 정리](#6-핵심-개념-정리)
7. [다음 단계](#7-다음-단계)

---

## 1. 학습 로드맵

### 전체 학습 경로 (권장 순서)

```
Week 1-2: 기초 다지기
┌─────────────────────────────────────────────────────────────┐
│  1. zcu104-baremetal-imgio (SD BMP 로더)                    │
│     - 목표: 베어메탈 환경 이해, FatFs 파일 I/O              │
│     - 핵심: SD 마운트, BMP 파싱, NHWC 데이터 레이아웃       │
│     - 예상 시간: 1-2일                                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
Week 2-3: HLS 가속기 설계
┌─────────────────────────────────────────────────────────────┐
│  2. zybo_memcopy_accel_interrupt (메모리 복사 가속기)       │
│     - 목표: HLS 가속기 개발 패턴 학습                       │
│     - 핵심: AXI 인터페이스, 버스트 전송, 인터럽트 처리      │
│     - 예상 시간: 3-5일                                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
Week 4-5: AI 추론 파이프라인
┌─────────────────────────────────────────────────────────────┐
│  3. zcu104-a53-mobilenetv1-baseline (숫자 인식 CNN)         │
│     - 목표: 양자화 신경망 추론 이해                         │
│     - 핵심: INT8 양자화, DW/PW Conv, PyTorch 학습           │
│     - 예상 시간: 5-7일                                      │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
Week 6+: 도전 과제
┌─────────────────────────────────────────────────────────────┐
│  4. HLS 가속기 구현 (선택)                                  │
│     - 목표: MobileNet 커널을 HLS IP로 변환                  │
│     - 참조: memcopy의 인터페이스 패턴 적용                  │
│     - 예상 시간: 2-4주                                      │
└─────────────────────────────────────────────────────────────┘
```

### 학습 목표별 추천 경로

| 목표 | 추천 경로 | 핵심 학습 내용 |
|------|-----------|----------------|
| **베어메탈 입문** | 프로젝트 1 | Vitis IDE, BSP 설정, SD 파일 I/O |
| **HLS 마스터** | 프로젝트 2 → 4 | 인터페이스 설계, 최적화, 테스트벤치 |
| **AI 가속화** | 프로젝트 1 → 3 → 4 | 양자화, 커널 구현, HLS 변환 |
| **전체 마스터** | 1 → 2 → 3 → 4 | 모든 기술 스택 통합 |

---

## 2. 사전 준비물

### 하드웨어

| 항목 | 필수 | 설명 |
|------|:----:|------|
| **ZCU104** 또는 **Zybo Z7-10** | ✓ | 최소 1개 필요 |
| microSD 카드 (8GB+) | ✓ | FAT32 포맷, Class 10 권장 |
| USB-UART 케이블 | ✓ | 보드에 포함 (Micro-USB) |
| JTAG 케이블 | ✓ | 보드에 포함 |
| 12V 전원 어댑터 | ✓ | ZCU104 전용 |

### 소프트웨어

| 소프트웨어 | 버전 | 용도 |
|------------|------|------|
| **Vivado** | 2022.1 | 하드웨어 설계, 비트스트림 생성 |
| **Vitis (Classic IDE)** | 2022.1 | 소프트웨어 개발, 베어메탈 앱 |
| **Vitis HLS** | 2022.1 | C/C++ → RTL 합성 |
| **Python** | 3.8+ | 모델 학습 (PyTorch), 가중치 변환 |
| **시리얼 터미널** | 최신 | PuTTY, Tera Term, minicom 등 |

### Python 패키지 (프로젝트 3용)

```bash
pip install torch torchvision pillow numpy
```

---

## 3. 개발 환경 구축

### 3.1 Vivado/Vitis 설치

1. **다운로드**: AMD Xilinx 다운로드 센터에서 Vivado 2022.1 웹 인스톨러
2. **설치 옵션**:
   - Vivado HL Design Edition (또는 Standard)
   - Vitis 체크박스 선택
   - 대상 디바이스: **Zynq UltraScale+ MPSoC** (ZCU104) + **Zynq-7000** (Zybo)
3. **라이선스**: Free 버전으로 충분 (WebPACK)

### 3.2 보드 파일 설치

#### ZCU104 (기본 포함)
Vivado에 기본 포함됨. 별도 설치 불필요.

#### Zybo Z7 (수동 설치)
1. Digilent 보드 파일 다운로드: https://github.com/Digilent/vivado-boards
2. `board_files` 폴더를 `<Vivado_Install>/data/boards/` 에 복사
3. Vivado 재시작

### 3.3 시리얼 터미널 설정

```
포트: COM? (장치 관리자에서 확인)
보드레이트: 115200
데이터 비트: 8
패리티: None
정지 비트: 1
흐름 제어: None
```

---

## 4. 프로젝트 개요

### 프로젝트 1: zcu104-baremetal-imgio

| 항목 | 내용 |
|------|------|
| **목적** | SD 카드에서 24비트 BMP 이미지 로드 |
| **타겟 보드** | ZCU104 (Zynq UltraScale+ MPSoC) |
| **프로세서** | Cortex-A53 (64-bit, 베어메탈) |
| **핵심 라이브러리** | xilffs (FatFs) |
| **학습 포인트** | SD 마운트, BMP 파싱, 메모리 레이아웃 |

**디렉토리 구조:**
```
zcu104-baremetal-imgio/
├── sd_bmp_loader.cpp      # 메인 소스
├── bmp_24.bmp             # 테스트 이미지
└── README.md
```

---

### 프로젝트 2: zybo_memcopy_accel_interrupt

| 항목 | 내용 |
|------|------|
| **목적** | HLS로 메모리 복사 가속기 구현 |
| **타겟 보드** | Zybo Z7-10 (Zynq-7000) |
| **프로세서** | Cortex-A9 (32-bit, 베어메탈) |
| **핵심 기술** | Vitis HLS, AXI4 Master, 인터럽트 |
| **학습 포인트** | HLS 인터페이스, 버스트 전송, 드라이버 작성 |

**디렉토리 구조:**
```
zybo_memcopy_accel_interrupt/
├── src/
│   ├── Vitis-HLS/
│   │   ├── memcopy_accel.cpp       # HLS 가속기
│   │   └── memcopy_accel_test.cpp  # 테스트벤치
│   ├── Vitis-BareMetal/
│   │   ├── main.c                  # 베어메탈 앱
│   │   ├── memcopy_accel.c/h       # 드라이버
│   └── GPIO-demo/                  # GPIO 예제
├── docs/
│   └── Memcopy-Guide.pdf
└── README.md
```

---

### 프로젝트 3: zcu104-a53-mobilenetv1-baseline

| 항목 | 내용 |
|------|------|
| **목적** | 양자화된 MobileNet으로 0-9 숫자 분류 |
| **타겟 보드** | ZCU104 (Zynq UltraScale+ MPSoC) |
| **프로세서** | Cortex-A53 (64-bit, 베어메탈) |
| **핵심 기술** | INT8 양자화, DW/PW Convolution |
| **학습 포인트** | 양자화 이론, CNN 추론, PyTorch 학습 |

**디렉토리 구조:**
```
zcu104-a53-mobilenetv1-baseline/
├── vitis_src/
│   ├── mobilenet_bm.cpp       # 메인 추론 앱
│   ├── ref_kernels.c/h        # 양자화 커널
├── tools/
│   ├── synth_digits_train.py  # 모델 학습
│   └── export_bins.py         # 가중치 내보내기
├── assets/
│   ├── dw3x3_c3.bin           # DW 가중치 (27B)
│   ├── pw1x1_c10x3.bin        # PW 가중치 (30B)
│   ├── labels.txt
│   └── samples/digit_*.bmp
├── platform/
│   └── standalone_zynq_core.xsa
└── README.md
```

---

## 5. 학습 단계별 상세 가이드

### Stage 1: 베어메탈 SD I/O (프로젝트 1)

#### Day 1: 환경 설정

1. **Vivado에서 하드웨어 생성**
   ```
   - New Project → RTL Project
   - Add Sources: 없음 (PS Only)
   - Board: ZCU104
   - Create Block Design
   - Add IP: Zynq UltraScale+ MPSoC
   - Run Block Automation (Apply board preset)
   - Enable: PS DDR4, UART0, SD0
   - Generate HDL Wrapper
   - Generate Bitstream (optional for PS-only)
   - Export Hardware (Include Bitstream)
   ```

2. **Vitis에서 플랫폼 생성**
   ```
   - New Platform Project
   - Import XSA
   - Processor: psu_cortexa53_0
   - OS: standalone
   - Build Platform
   ```

3. **BSP 설정**
   - Board Support Package Settings 열기
   - `xilffs` 라이브러리 추가
   - `fs_interface = 1` (SD)

#### Day 2: 코드 작성 및 테스트

1. **Application Project 생성**
   ```
   - Empty Application (C++)
   - sd_bmp_loader.cpp 추가
   ```

2. **링커 스크립트 수정**
   ```
   Heap: 4MB (0x400000) → psu_ddr_0
   Stack: 64KB (0x10000) → psu_ddr_0
   ```

3. **SD 카드 준비**
   ```
   test.bmp (24비트 비압축 BMP) → SD 카드 루트
   ```

4. **테스트 실행**
   ```
   Run → Debug As → Launch on Hardware
   UART 출력 확인
   ```

#### 핵심 코드 이해

```c
// SD 카드 마운트
FRESULT rc = f_mount(&fatfs, "0:/", 1);  // 드라이브 0, 즉시 마운트

// BMP 헤더 구조체 (1바이트 정렬 필수)
#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t bfType;      // 'BM' = 0x4D42
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;   // 픽셀 데이터 오프셋
};
#pragma pack(pop)

// 이미지 데이터 레이아웃 (NHWC)
// pixels[(y * width + x) * channels + c]
```

---

### Stage 2: HLS 가속기 설계 (프로젝트 2)

#### Day 1-2: HLS 기초

1. **Vitis HLS 프로젝트 생성**
   ```
   - New Project
   - Add Source: memcopy_accel.cpp
   - Add Testbench: memcopy_accel_test.cpp
   - Part: xc7z010clg400-1 (Zybo Z7-10)
   ```

2. **HLS 인터페이스 이해**
   ```cpp
   #pragma HLS INTERFACE m_axi port=src offset=slave bundle=AXI_SRC
   #pragma HLS INTERFACE s_axilite port=src bundle=CTRL_BUS
   #pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
   ```

   | 인터페이스 | 용도 |
   |------------|------|
   | `m_axi` | 메모리 접근 (DDR 읽기/쓰기) |
   | `s_axilite` | 제어 레지스터 (주소, 시작, 완료) |

3. **C 시뮬레이션**
   ```
   Project → Run C Simulation
   ```

4. **합성 (Synthesis)**
   ```
   Solution → Run C Synthesis
   ```
   - 리포트 확인: Latency, Interval, Resource Usage

5. **Co-Simulation**
   ```
   Solution → Run C/RTL Cosimulation
   ```

6. **IP 내보내기**
   ```
   Solution → Export RTL → IP Catalog
   ```

#### Day 3-4: Vivado 통합

1. **Block Design에 IP 추가**
   ```
   - Zynq PS 추가
   - memcopy_accel IP 추가
   - AXI Interconnect로 연결
   - 인터럽트 연결 (pl_ps_irq)
   ```

2. **Validate Design → Generate Bitstream → Export XSA**

#### Day 5: 베어메탈 드라이버

```c
// 레지스터 맵 (HLS 자동 생성)
#define CTRL_OFFSET   0x00  // ap_start, ap_done, ap_idle
#define GIE_OFFSET    0x04  // Global Interrupt Enable
#define IER_OFFSET    0x08  // IP Interrupt Enable
#define ISR_OFFSET    0x0C  // IP Interrupt Status
#define SRC_OFFSET    0x10  // 소스 주소
#define DST_OFFSET    0x1C  // 목적지 주소
#define LEN_OFFSET    0x28  // 바이트 수

// 가속기 시작
void memcopy_accel_start(uint32_t src, uint32_t dst, uint32_t len) {
    Xil_Out32(BASE + SRC_OFFSET, src);
    Xil_Out32(BASE + DST_OFFSET, dst);
    Xil_Out32(BASE + LEN_OFFSET, len);
    Xil_Out32(BASE + CTRL_OFFSET, 0x01);  // ap_start
}
```

#### 핵심 HLS 최적화

```cpp
// 버퍼 완전 분할 → 32개 레지스터로
#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1

// 파이프라인 → 매 사이클 새 반복 시작
#pragma HLS PIPELINE II=1

// 루프 언롤 → 병렬 실행
#pragma HLS UNROLL
```

---

### Stage 3: CNN 추론 (프로젝트 3)

#### Day 1-2: 양자화 이론

**양자화 공식:**
```
실수값 = scale × (양자화값 - zero_point)
양자화값 = zero_point + round(실수값 / scale)
```

**프로젝트 파라미터:**
| 파라미터 | 값 | 설명 |
|----------|-----|------|
| in_scale | 0.02 | 입력 스케일 |
| in_zp | 128 | 입력 영점 (unsigned) |
| w_scale | 0.00196 | 가중치 스케일 |
| w_zp | 128 | 가중치 영점 |

#### Day 3-4: 커널 구현

**Depthwise Convolution 3×3:**
- 각 입력 채널에 독립적인 3×3 필터 적용
- 출력 채널 수 = 입력 채널 수
- 파라미터: C × 9 (프로젝트: 3 × 9 = 27)

**Pointwise Convolution 1×1:**
- 모든 입력 채널의 가중합
- GEMM 연산과 동일
- 파라미터: Cout × Cin (프로젝트: 10 × 3 = 30)

#### Day 5-7: 실행 및 분석

1. **모델 학습 (선택)**
   ```bash
   cd tools
   python synth_digits_train.py
   python export_bins.py
   ```

2. **SD 카드 레이아웃**
   ```
   0:/assets/dw3x3_c3.bin      (27 bytes)
   0:/assets/pw1x1_c10x3.bin   (30 bytes)
   0:/assets/labels.txt        (10 lines)
   0:/assets/samples/digit_*.bmp
   ```

3. **실행 결과 분석**
   ```
   DWConv: 15 ms (15234 us)
   PWConv: 8 ms (8421 us)
   AvgPool: 2 ms
   Softmax: 0 ms
   Top-5:
    1) 1 : 241/255 (94%)
    2) 7 : 8/255 (3%)
    ...
   ```

---

## 6. 핵심 개념 정리

### AXI 인터페이스

| 프로토콜 | 특징 | 용도 |
|----------|------|------|
| **AXI4-Lite** | 단일 전송, 낮은 복잡도 | 제어 레지스터 |
| **AXI4 (Full)** | 버스트 전송, 고대역폭 | 메모리 접근 |
| **AXI4-Stream** | 지속적 데이터 흐름 | 스트리밍 처리 |

### HLS Pragma 참조

| Pragma | 효과 |
|--------|------|
| `INTERFACE m_axi` | AXI 마스터 포트 생성 |
| `INTERFACE s_axilite` | 제어 레지스터 생성 |
| `PIPELINE II=N` | N 사이클마다 새 반복 |
| `UNROLL factor=N` | 루프 N배 병렬화 |
| `ARRAY_PARTITION` | 배열을 레지스터로 분할 |

### 캐시 관리 (PS-PL 동기화)

```c
// PS → PL 전송 전 (CPU 쓰기 후)
Xil_DCacheFlushRange((UINTPTR)buf, size);

// PL → PS 수신 후 (가속기 완료 후)
Xil_DCacheInvalidateRange((UINTPTR)buf, size);
```

---

## 7. 다음 단계

### 단기 목표 (1-2주)
- [ ] 프로젝트 1-3 모두 직접 빌드 및 실행
- [ ] HLS 리포트 분석 능력 습득
- [ ] 인터럽트 핸들링 패턴 숙달

### 중기 목표 (1개월)
- [ ] DW 3×3 HLS IP 구현 (스트리밍 스텐실)
- [ ] PW 1×1 HLS IP 구현 (타일링 GEMM)
- [ ] PS-PL 데이터 경로 최적화

### 장기 목표 (3-6개월)
- [ ] 실제 MobileNetV1 전체 구현
- [ ] ImageNet 분류기 포팅
- [ ] Vitis AI / DPU 활용

---

## 부록: 유용한 명령어

### Vivado Tcl 콘솔
```tcl
# 보드 파일 확인
get_board_parts *zcu104*

# 합성 상태 확인
report_utilization -file util.txt
```

### Vitis 디버깅
```bash
# UART 로그 확인 (Linux)
minicom -D /dev/ttyUSB0 -b 115200
```

### HLS 합성 통계
```
Vivado HLS → Solution → Reports → Synthesis
- Timing: Latency, Interval
- Utilization: BRAM, DSP, FF, LUT
```

---

*문서 작성일: 2026-02-06*  
*관련 문서: ZCU104_User_Guide.md, Zybo_User_Guide.md, Lab_Practice_Guide.md*
