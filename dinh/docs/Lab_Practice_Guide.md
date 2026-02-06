# 프로젝트별 실습 가이드

> 3개 프로젝트를 단계별로 실습하는 상세 가이드입니다.  
> 각 실습은 독립적으로 진행할 수 있지만, 순서대로 진행하면 학습 효과가 극대화됩니다.

---

## 목차

1. [Lab 1: SD BMP 로더 (zcu104-baremetal-imgio)](#lab-1-sd-bmp-로더)
2. [Lab 2: HLS 메모리 복사 가속기 (zybo_memcopy_accel_interrupt)](#lab-2-hls-메모리-복사-가속기)
3. [Lab 3: CNN 숫자 인식기 (zcu104-a53-mobilenetv1-baseline)](#lab-3-cnn-숫자-인식기)
4. [Challenge: DW/PW HLS IP 구현](#challenge-dwpw-hls-ip-구현)

---

## Lab 1: SD BMP 로더

**프로젝트:** `zcu104-baremetal-imgio`  
**타겟 보드:** ZCU104  
**예상 시간:** 4-6시간  
**난이도:** ★☆☆☆☆ (초급)

### 학습 목표

- [ ] Vivado에서 PS-Only 하드웨어 생성
- [ ] Vitis에서 베어메탈 플랫폼 구성
- [ ] xilffs (FatFs) 라이브러리 사용
- [ ] BMP 파일 구조 이해
- [ ] NHWC 데이터 레이아웃 이해

---

### Step 1: 하드웨어 설계 (Vivado)

#### 1.1 프로젝트 생성

```
File → New Project...
├── Name: zcu104_imgio
├── Location: <your_workspace>
├── Type: RTL Project
└── Board: ZCU104 Evaluation Board
```

#### 1.2 Block Design 생성

```
Flow Navigator → Create Block Design
└── Name: system
```

#### 1.3 Zynq MPSoC 추가 및 설정

```
Add IP → Zynq UltraScale+ MPSoC
Run Block Automation → Apply Board Preset
```

**PS 설정 확인 (Double-click):**

| 항목 | 설정 | 확인 |
|------|------|:----:|
| DDR4 | 활성화됨 | ✓ |
| UART 0 | MIO 18-19 | ✓ |
| SD 0 | MIO 13-22 | ✓ |

#### 1.4 Wrapper 생성 및 합성

```
Sources → Right-click system → Create HDL Wrapper
└── Let Vivado manage...

Generate Bitstream (선택사항, PS-Only라면 불필요)

File → Export → Export Hardware...
├── Include bitstream: Optional
└── File: zcu104_imgio.xsa
```

---

### Step 2: 플랫폼 생성 (Vitis)

#### 2.1 새 플랫폼

```
File → New → Platform Project...
├── Name: zcu104_platform
├── XSA: zcu104_imgio.xsa
├── Processor: psu_cortexa53_0
├── OS: standalone
└── Generate boot components: ✓
```

#### 2.2 BSP에 xilffs 추가

```
Platform Explorer → psu_cortexa53_0 → Board Support Package
└── Modify BSP Settings...

Libraries:
├── [✓] xilffs
│   ├── fs_interface: 1 (SD)
│   ├── read_only: false
│   └── use_lfn: 1
```

#### 2.3 플랫폼 빌드

```
Build Platform (Hammer icon)
```

---

### Step 3: Application 생성

#### 3.1 새 Application

```
File → New → Application Project...
├── Name: sd_bmp_loader
├── Platform: zcu104_platform
├── Domain: standalone_psu_cortexa53_0
└── Template: Empty Application (C++)
```

#### 3.2 소스 코드 추가

프로젝트 폴더의 `sd_bmp_loader.cpp` 복사:

```
zcu104-baremetal-imgio/sd_bmp_loader.cpp
→ Vitis: sd_bmp_loader/src/sd_bmp_loader.cpp
```

#### 3.3 링커 스크립트 수정

```
Project → Generate Linker Script...

Memory Region Mapping:
├── .heap → psu_ddr_0 (Size: 0x400000 = 4MB)
├── .stack → psu_ddr_0 (Size: 0x10000 = 64KB)
```

#### 3.4 빌드

```
Build Project (Ctrl+B)
```

---

### Step 4: SD 카드 준비

#### 4.1 SD 카드 포맷

- 파일 시스템: FAT32
- 클러스터 크기: 기본값 (4096 또는 32768)

#### 4.2 테스트 이미지 준비

24비트 비압축 BMP 파일 생성:
- 크기: 200×200 권장
- 형식: Windows BMP, 24-bit, BI_RGB (압축 없음)

**GIMP로 생성:**
```
File → Export As... → test.bmp
└── Compatibility Options: Do not write color space information
```

**ImageMagick으로 변환:**
```bash
convert input.png -type TrueColor BMP3:test.bmp
```

#### 4.3 파일 복사

```
SD 카드 루트에 test.bmp 복사
0:/test.bmp
```

---

### Step 5: 실행 및 테스트

#### 5.1 하드웨어 연결

1. SD 카드를 ZCU104에 삽입
2. USB 케이블 연결 (J2)
3. 부트 모드: JTAG (SW6 = 0000)
4. 전원 ON

#### 5.2 터미널 열기

```
PuTTY 또는 Tera Term
├── Port: COM? (높은 번호)
├── Baud: 115200
└── 8-N-1, No flow control
```

#### 5.3 실행

```
Run → Run As → Launch on Hardware (Single Application Debug)
```

#### 5.4 예상 출력

```
SD BMP Loader (bare-metal)
SD mounted: 0:/
File size: 120054 bytes
Loaded BMP: 200x200, 3 channels
Top-left pixel RGB = (255,0,0)
Done.
```

---

### 실습 과제

#### 과제 1: 다양한 이미지 테스트
다른 크기의 BMP 이미지로 테스트하고 결과 확인

#### 과제 2: 픽셀 분석 확장
이미지의 모서리 4개 픽셀과 중앙 픽셀 값 출력

```cpp
// 힌트: 픽셀 접근
int center_y = H / 2, center_x = W / 2;
int idx = (center_y * W + center_x) * 3;
uint8_t r = pixels[idx], g = pixels[idx+1], b = pixels[idx+2];
```

#### 과제 3: 그레이스케일 변환
RGB 이미지를 그레이스케일로 변환하고 평균 밝기 계산

```cpp
// 힌트: Luminance 공식
uint8_t gray = (uint8_t)(0.299f*r + 0.587f*g + 0.114f*b);
```

---

## Lab 2: HLS 메모리 복사 가속기

**프로젝트:** `zybo_memcopy_accel_interrupt`  
**타겟 보드:** Zybo Z7-10  
**예상 시간:** 8-12시간  
**난이도:** ★★★☆☆ (중급)

### 학습 목표

- [ ] Vitis HLS 워크플로우 이해
- [ ] AXI4 Master/AXI4-Lite 인터페이스 설계
- [ ] HLS Pragma로 최적화
- [ ] C/RTL Co-simulation 수행
- [ ] 베어메탈 드라이버 작성
- [ ] 인터럽트 기반 동기화 구현

---

### Step 1: HLS 프로젝트 생성

#### 1.1 Vitis HLS 실행

```
Vitis HLS 2022.1 실행
File → New Project...
├── Name: memcopy_accel
├── Location: <workspace>
└── Next
```

#### 1.2 소스 파일 추가

```
Add Files:
├── zybo_memcopy_accel_interrupt/src/Vitis-HLS/memcopy_accel.cpp
└── Top Function: memcopy_accel

Add Testbench:
├── memcopy_accel_test.cpp
└── memcopy_accel_tb_data.h
```

#### 1.3 솔루션 설정

```
Part Selection:
└── xc7z010clg400-1 (Zybo Z7-10)

Clock Period: 10ns (100MHz)
```

---

### Step 2: HLS 코드 분석

#### 2.1 인터페이스 분석

```cpp
void memcopy_accel(uint32_t* src, uint32_t* dst, uint32_t len)
```

| 포트 | HLS 인터페이스 | 번들 | 설명 |
|------|---------------|------|------|
| src | m_axi + s_axilite | AXI_SRC + CTRL_BUS | DDR 읽기 + 주소 레지스터 |
| dst | m_axi + s_axilite | AXI_DST + CTRL_BUS | DDR 쓰기 + 주소 레지스터 |
| len | s_axilite | CTRL_BUS | 바이트 수 레지스터 |
| return | s_axilite | CTRL_BUS | ap_start, ap_done 제어 |

#### 2.2 최적화 분석

```cpp
#pragma HLS ARRAY_PARTITION variable=buffer complete dim=1
// → 버퍼를 32개 레지스터로 분할

#pragma HLS PIPELINE II=1
// → 매 사이클 새 반복 시작

#pragma HLS UNROLL
// → 읽기/쓰기 루프 완전 전개
```

---

### Step 3: HLS 합성 및 검증

#### 3.1 C Simulation

```
Project → Run C Simulation
```

테스트벤치 출력 확인:
```
Source and destination buffers match!
Test PASSED!
```

#### 3.2 C Synthesis

```
Solution → Run C Synthesis
```

**리포트 확인:**
- Timing: Latency, Interval
- Utilization: BRAM, DSP, FF, LUT

#### 3.3 C/RTL Cosimulation

```
Solution → Run C/RTL Cosimulation
└── Dump Trace: all (선택)
```

#### 3.4 Export RTL

```
Solution → Export RTL
├── Format: IP Catalog
└── 출력: solution1/impl/ip/
```

---

### Step 4: Vivado 통합

#### 4.1 새 Vivado 프로젝트

```
File → New Project...
├── Name: zybo_memcopy
├── Board: Zybo Z7-10
```

#### 4.2 IP Repository 추가

```
Settings → IP → Repository → Add
└── <HLS_Project>/solution1/impl/ip
```

#### 4.3 Block Design 생성

```
Create Block Design → system

Add IP:
├── ZYNQ7 Processing System
├── memcopy_accel (HLS IP)
├── AXI Interconnect (2개)
└── Processor System Reset
```

#### 4.4 연결

**자동 연결:**
```
Run Connection Automation
└── 모든 인터페이스 선택
```

**인터럽트 연결:**
```
memcopy_accel.interrupt → xlconcat (추가) → PS.IRQ_F2P[0:0]
```

#### 4.5 주소 확인

```
Address Editor:
├── memcopy_accel_0: 0x40000000 (s_axi_CTRL_BUS)
├── memcopy_accel_0: 0x00000000-0x3FFFFFFF (AXI_SRC, DDR)
└── memcopy_accel_0: 0x00000000-0x3FFFFFFF (AXI_DST, DDR)
```

#### 4.6 합성 및 내보내기

```
Generate HDL Wrapper
Run Synthesis → Run Implementation → Generate Bitstream
Export Hardware (Include bitstream)
```

---

### Step 5: 베어메탈 애플리케이션

#### 5.1 Vitis 플랫폼 생성

```
XSA 가져오기 → Platform 생성
├── Processor: ps7_cortexa9_0
└── OS: standalone
```

#### 5.2 Application 생성

프로젝트 파일 복사:
```
src/Vitis-BareMetal/main.c → src/
src/Vitis-BareMetal/memcopy_accel.c → src/
src/Vitis-BareMetal/memcopy_accel.h → src/
```

#### 5.3 주소 매크로 확인

`memcopy_accel.h`에서 베이스 주소 확인:
```c
#define MEMCOPY_ACCEL_BASEADDR  0x40000000u
```

Vivado Address Editor의 값과 일치해야 함.

#### 5.4 빌드 및 실행

```
Build Project
Run → Debug As → Launch on Hardware
```

---

### Step 6: 결과 분석

#### 6.1 예상 출력

```
=== Memcopy Accelerator Demo ===
Interrupt system initialized
Accelerator configured
Starting accelerator...
Accelerator completed (interrupt received)
Verification: PASSED (8192 words match)
Performance:
  HW accelerator: 156 us
  SW memcpy:      312 us
  Speedup:        2.00x
```

#### 6.2 성능 분석

| 구현 | 대역폭 | 비고 |
|------|--------|------|
| SW memcpy | ~100 MB/s | CPU 루프 |
| HW accel | ~200 MB/s | AXI 버스트 |
| 이론 최대 | ~400 MB/s | 32-bit @ 100MHz |

---

### 실습 과제

#### 과제 1: 버스트 길이 변경
`BURST_LEN`을 16, 64로 변경하고 성능 비교

#### 과제 2: 폴링 모드 구현
인터럽트 대신 ap_done 폴링으로 변경

```c
// 힌트
while (!(Xil_In32(BASE + CTRL_OFFSET) & 0x02)); // ap_done bit
```

#### 과제 3: 64-bit 주소 지원
m_axi 포트를 64-bit 주소로 수정 (offset=slave → off)

---

## Lab 3: CNN 숫자 인식기

**프로젝트:** `zcu104-a53-mobilenetv1-baseline`  
**타겟 보드:** ZCU104  
**예상 시간:** 12-16시간  
**난이도:** ★★★★☆ (고급)

### 학습 목표

- [ ] INT8 양자화 이론 이해
- [ ] Depthwise/Pointwise Convolution 구현
- [ ] PyTorch로 모델 학습
- [ ] 양자화된 가중치 내보내기
- [ ] 베어메탈 추론 파이프라인 구현

---

### Step 1: 이론 학습

#### 1.1 양자화 기초

**양자화 공식:**
```
float_value = scale × (quant_value - zero_point)
quant_value = zero_point + round(float_value / scale)
```

**프로젝트 파라미터:**
```
입력: scale=0.02, zp=128
가중치: scale=0.00196, zp=128
출력: scale=0.02, zp=128
```

#### 1.2 MobileNet 구조

```
입력 (32×32×3)
     │
     ▼
┌───────────────┐
│ DW Conv 3×3   │  입력 채널별 독립 필터
│ + ReLU6       │  출력: 32×32×3
└───────────────┘
     │
     ▼
┌───────────────┐
│ PW Conv 1×1   │  채널 변환 (3→10)
│               │  출력: 32×32×10
└───────────────┘
     │
     ▼
┌───────────────┐
│ Global Avg    │  공간 차원 제거
│ Pooling       │  출력: 1×1×10
└───────────────┘
     │
     ▼
┌───────────────┐
│ Softmax       │  확률 변환
│               │  출력: 10 (각 숫자 확률)
└───────────────┘
```

---

### Step 2: 모델 학습 (선택)

#### 2.1 환경 준비

```bash
cd zcu104-a53-mobilenetv1-baseline/tools
pip install torch torchvision pillow numpy
```

#### 2.2 학습 실행

```bash
python synth_digits_train.py
```

**학습 과정:**
1. 합성 숫자 데이터셋 생성 (0-9)
2. TinyDigitDSCNN 모델 정의
3. 10 에포크 학습
4. 가중치 저장 (`tiny_digit_dscnn.pt`)

#### 2.3 가중치 내보내기

```bash
python export_bins.py
```

**출력 파일:**
- `dw3x3_c3.bin` (27 bytes): DW 가중치
- `pw1x1_c10x3.bin` (30 bytes): PW 가중치

---

### Step 3: 플랫폼 준비

#### 3.1 사전 빌드 XSA 사용

```
platform/standalone_zynq_core.xsa
```

#### 3.2 Vitis 플랫폼 생성

```
File → New → Platform Project...
├── Name: zcu104_mobilenet_platform
├── XSA: standalone_zynq_core.xsa
├── Processor: psu_cortexa53_0
└── OS: standalone
```

#### 3.3 BSP 설정

```
Libraries:
├── [✓] xilffs (SD 파일 I/O)
```

---

### Step 4: Application 생성

#### 4.1 파일 복사

```
vitis_src/mobilenet_bm.cpp → src/
vitis_src/ref_kernels.c → src/
vitis_src/ref_kernels.h → src/
```

#### 4.2 링커 스크립트

```
Heap: 4MB
Stack: 64KB
```

#### 4.3 빌드

```
Build Project
```

---

### Step 5: SD 카드 준비

#### 5.1 디렉토리 구조

```
SD 카드 (FAT32):
└── assets/
    ├── dw3x3_c3.bin        (27 bytes)
    ├── pw1x1_c10x3.bin     (30 bytes)
    ├── labels.txt          (10 lines: 0-9)
    └── samples/
        ├── digit_0.bmp
        ├── digit_1.bmp
        ...
        └── digit_9.bmp
```

#### 5.2 테스트 이미지 요구사항

- 크기: 32×32 픽셀
- 형식: 24-bit BMP (비압축)
- 배경: 흰색 또는 밝은 색
- 숫자: 검정색 또는 어두운 색

---

### Step 6: 실행 및 분석

#### 6.1 실행

```
Run → Debug As → Launch on Hardware
```

#### 6.2 예상 출력

```
mobilenet_bm: bare-metal demo
SD mounted: 0:/
File 0:/assets/dw3x3_c3.bin size: 27 bytes
File 0:/assets/pw1x1_c10x3.bin size: 30 bytes
File 0:/assets/labels.txt size: 20 bytes
Image loaded: 32x32
DWConv: 15 ms (15234 us)
PWConv: 8 ms (8421 us)
AvgPool: 2 ms (2103 us)
Softmax: 0 ms (156 us)
Top-5:
 1) 1 : 241/255 (94%)
 2) 7 : 8/255 (3%)
 3) 4 : 4/255 (2%)
 4) 9 : 2/255 (1%)
 5) 2 : 0/255 (0%)
Done.
```

#### 6.3 성능 분석

| 연산 | 시간 | MAC 연산 수 |
|------|------|-------------|
| DWConv 3×3 | ~15ms | 32×32×3×9 = 27,648 |
| PWConv 1×1 | ~8ms | 32×32×3×10 = 30,720 |
| AvgPool | ~2ms | 32×32×10 = 10,240 |
| Softmax | <1ms | 10 |

---

### 실습 과제

#### 과제 1: 다양한 숫자 테스트
각 숫자 이미지(digit_0 ~ digit_9)로 테스트하고 정확도 확인

#### 과제 2: 실시간 분류
여러 이미지를 연속 분류하고 평균 처리 시간 측정

#### 과제 3: 커널 프로파일링
각 커널 함수의 실행 시간을 개별 측정하고 병목 분석

```cpp
// 힌트: XTime 사용
XTime t0, t1;
XTime_GetTime(&t0);
// ... 커널 실행 ...
XTime_GetTime(&t1);
uint32_t us = us_from_counts(t0, t1);
```

---

## Challenge: DW/PW HLS IP 구현

**난이도:** ★★★★★ (고급+)  
**예상 시간:** 2-4주

### 목표

`ref_kernels.c`의 DW/PW 커널을 HLS IP로 변환하여 하드웨어 가속

### 권장 아키텍처

```
MM2S (AXI DMA) → DW3x3 IP → PW1x1 IP → S2MM (AXI DMA)
```

### Step 1: DW 3×3 스트리밍 설계

```cpp
void dw3x3_stream(
    hls::stream<axis_t>& s_in,   // AXI-Stream 입력
    hls::stream<axis_t>& s_out,  // AXI-Stream 출력
    const uint8_t* weights,      // m_axi: DW 가중치
    int H, int W, int C,
    int in_zp, int w_zp, int out_zp,
    int32_t alpha_q              // Q24 재양자화 승수
);
```

**핵심 최적화:**
- 라인 버퍼로 스텐실 윈도우 관리
- 채널 병렬 처리 (CHAN_ALIGN)
- 파이프라인 II=1

### Step 2: PW 1×1 시스톨릭 설계

```cpp
void pw1x1_systolic(
    hls::stream<axis_t>& s_in,
    hls::stream<axis_t>& s_out,
    const uint8_t* weights,      // m_axi: PW 가중치
    int H, int W, int Cin, int Cout,
    int in_zp, int w_zp, int out_zp,
    int32_t alpha_q
);
```

**핵심 최적화:**
- 타일링: M=H×W, K=Cin, N=Cout
- 웨이트-스테이셔너리 방식
- PE 어레이 (Tn 출력 병렬)

### Step 3: 퓨전 톱

```cpp
void dw_pw_fused(
    hls::stream<axis_t>& s_in,
    hls::stream<axis_t>& s_out,
    const uint8_t* w_dw,
    const uint8_t* w_pw,
    // ... 양자화 파라미터 ...
) {
    #pragma HLS DATAFLOW
    
    hls::stream<axis_t> s_mid;
    
    dw3x3_stream(s_in, s_mid, w_dw, ...);
    pw1x1_systolic(s_mid, s_out, w_pw, ...);
}
```

### 검증 체크리스트

- [ ] C Simulation: 황금 참조와 bit-exact 일치
- [ ] Co-Simulation: RTL 기능 검증
- [ ] 리소스: Zybo Z7-10 또는 ZCU104에 맞는지 확인
- [ ] 성능: SW 대비 속도 향상 측정

---

## 체크리스트: 전체 학습 진행

### Lab 1 완료 확인
- [ ] Vivado PS-Only 설계 완료
- [ ] Vitis 플랫폼 생성 완료
- [ ] xilffs 라이브러리 동작 확인
- [ ] BMP 로드 및 픽셀 출력 성공
- [ ] 과제 1-3 완료

### Lab 2 완료 확인
- [ ] HLS C Simulation 성공
- [ ] HLS Synthesis 성공
- [ ] Vivado 통합 완료
- [ ] 베어메탈 드라이버 동작
- [ ] 인터럽트 수신 확인
- [ ] 성능 측정 완료
- [ ] 과제 1-3 완료

### Lab 3 완료 확인
- [ ] 양자화 이론 이해
- [ ] PyTorch 학습 (선택)
- [ ] 가중치 파일 준비
- [ ] 추론 파이프라인 실행
- [ ] Top-5 예측 출력 확인
- [ ] 과제 1-3 완료

### Challenge 완료 확인
- [ ] DW HLS IP 설계
- [ ] PW HLS IP 설계
- [ ] 퓨전 톱 구현
- [ ] 시스템 통합
- [ ] 성능 검증

---

*문서 작성일: 2026-02-06*  
*관련 문서: FPGA_Learning_Guide.md, HLS_Development_Guide.md*
