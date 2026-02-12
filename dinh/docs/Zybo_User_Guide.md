# Zybo Z7 보드 상세 사용 설명서

> **보드명:** Digilent Zybo Z7-10 / Z7-20  
> **FPGA:** Zynq-7000 SoC (XC7Z010-1CLG400C / XC7Z020-1CLG400C)  
> **프로세서:** Dual-core ARM Cortex-A9 @ 650MHz (32-bit)

---

## 목차

1. [보드 개요](#1-보드-개요)
2. [하드웨어 스펙](#2-하드웨어-스펙)
3. [보드 레이아웃](#3-보드-레이아웃)
4. [점퍼 및 부팅 설정](#4-점퍼-및-부팅-설정)
5. [연결 방법](#5-연결-방법)
6. [Vivado 하드웨어 설계](#6-vivado-하드웨어-설계)
7. [Vitis HLS 가속기 개발](#7-vitis-hls-가속기-개발)
8. [Vitis 베어메탈 개발](#8-vitis-베어메탈-개발)
9. [GPIO 및 주변장치](#9-gpio-및-주변장치)
10. [문제 해결](#10-문제-해결)

---

## 1. 보드 개요

Zybo Z7은 Digilent에서 제조한 저가형 Zynq SoC 개발 보드입니다.

### Z7-10 vs Z7-20 비교

| 특징 | Z7-10 | Z7-20 |
|------|-------|-------|
| **Logic Cells** | 28K | 85K |
| **LUT** | 17,600 | 53,200 |
| **Flip-Flop** | 35,200 | 106,400 |
| **Block RAM** | 60 (2.1Mb) | 140 (4.9Mb) |
| **DSP Slice** | 80 | 220 |
| **가격** | 약 $199 | 약 $299 |

### 적용 분야

- FPGA/SoC 입문 교육
- IoT 프로토타입
- 영상 처리 (HDMI 입출력)
- 저전력 임베디드 시스템

---

## 2. 하드웨어 스펙

### 프로세서 시스템 (PS)

| 항목 | 사양 |
|------|------|
| CPU | Dual-core ARM Cortex-A9 @ 650MHz |
| L1 Cache | 32KB I + 32KB D (per core) |
| L2 Cache | 512KB (shared) |
| 온칩 메모리 | 256KB OCM |

### 메모리

| 항목 | 사양 |
|------|------|
| DDR3L | 1GB, 16-bit, 1066MT/s |
| QSPI Flash | 16MB |

### I/O 인터페이스

| 인터페이스 | 사양 |
|------------|------|
| HDMI TX | 1080p@60Hz |
| HDMI RX | 1080p@60Hz |
| USB 2.0 | Host + OTG |
| Ethernet | 1G (RJ45) |
| SD | microSD 슬롯 |
| Audio | Line-in, Line-out, HP-out, Mic |
| PMOD | 6× Standard (8-pin) |
| Zynq MIO | 14핀 |

### 사용자 I/O

| 항목 | 수량 |
|------|------|
| LED | 4 (MIO) |
| RGB LED | 2 (PL) |
| 슬라이드 스위치 | 4 (PL) |
| 푸시 버튼 | 4 (PL) |

---

## 3. 보드 레이아웃

```
┌─────────────────────────────────────────────────────────────────┐
│    [USB-JTAG/UART]  [USB OTG]  [Ethernet]  [HDMI TX] [HDMI RX] │
│          ■              ■           ■          ■         ■      │
│                                                                  │
│    ┌────────────────────────────────────┐                        │
│    │                                    │    [Audio]             │
│    │         Zynq-7000 SoC              │      ■ ■ ■ ■          │
│    │       XC7Z010/XC7Z020              │                        │
│    └────────────────────────────────────┘                        │
│                                                                  │
│    [DDR3L 1GB]                                                   │
│                                                                  │
│    [JP5] Boot Mode                                               │
│      ■                                                           │
│                                                                  │
│    [PMOD A] [PMOD B] [PMOD C] [PMOD D] [PMOD E] [PMOD F]       │
│       ■        ■        ■        ■        ■        ■            │
│                                                                  │
│    [Zynq MIO]           [microSD]                               │
│        ■                    ■                                    │
│                                                                  │
│    (LED)  (RGB)  (SW)  (BTN)     [Power]  [Reset]               │
│    ○○○○  ◐◐    ■■■■   ●●●●        ●         ●                   │
└─────────────────────────────────────────────────────────────────┘
```

### 주요 커넥터

| 커넥터 | 위치 | 설명 |
|--------|------|------|
| J11 | 상단 좌측 | USB-JTAG/UART (Micro-USB) |
| J9 | 상단 | USB OTG (Micro-USB) |
| J6 | 상단 | Ethernet RJ45 |
| J12 | 상단 우측 | HDMI TX |
| J13 | 상단 우측 | HDMI RX |
| J4 | 하단 중앙 | microSD |
| JA-JF | 하단 | PMOD 커넥터 6개 |

---

## 4. 점퍼 및 부팅 설정

### JP5: 부팅 모드 점퍼

**위치:** 보드 중앙 좌측

| 모드 | JP5 설정 | 설명 |
|------|----------|------|
| **JTAG** | Open (점퍼 제거) | 개발 모드 |
| **SD** | Shorted (점퍼 연결) | SD 카드 부팅 |
| QSPI | - | QSPI 부팅 (별도 설정) |

### JP6: 전원 선택

| 설정 | 전원 소스 |
|------|-----------|
| USB | USB에서 전원 공급 (기본) |
| EXT | 외부 전원 공급 (5V 배럴잭) |

### JP7: UART 선택 (선택사항)

UART 0 또는 UART 1 선택

---

## 5. 연결 방법

### 5.1 기본 연결 (개발용)

```
1. JP5 점퍼 제거 (JTAG 모드)
2. USB 케이블 연결 (J11 → PC)
   - JTAG 프로그래밍
   - UART 시리얼 (COM 포트 2개)
3. 전원 자동 공급 (USB를 통해)
```

### 5.2 시리얼 터미널 설정

USB 연결 시 2개의 COM 포트 생성:
- **COM X**: JTAG (사용 안 함)
- **COM Y**: UART (시리얼 통신)

**터미널 설정:**
```
Baud Rate: 115200
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: None
```

### 5.3 PMOD 연결

| PMOD | 위치 | 연결 타입 |
|------|------|-----------|
| JA | PL | Standard (High-Speed) |
| JB | PL | Standard (High-Speed) |
| JC | PL | Standard |
| JD | PL | Standard |
| JE | PS (MIO) | MIO 핀 직접 연결 |
| JF | PS (MIO) | MIO 핀 직접 연결 |

---

## 6. Vivado 하드웨어 설계

### 6.1 보드 파일 설치

Digilent 보드 파일이 없으면:

```bash
git clone https://github.com/Digilent/vivado-boards.git
```

복사 위치:
```
<Vivado_Install>/2022.1/data/boards/board_files/
```

### 6.2 새 프로젝트 생성

```
File → New Project...
├── Project Name: zybo_project
├── Project Type: RTL Project
└── Default Part:
    └── Boards → zybo-z7-10 (또는 zybo-z7-20)
```

### 6.3 Block Design 생성

```
Create Block Design → system

Add IP → ZYNQ7 Processing System
└── Run Block Automation
    └── Apply Board Preset
```

### 6.4 PS 설정

더블클릭하여 설정:

**PS-PL Configuration:**
```
[✓] M AXI GP0 Interface (PL 접근용)
[✓] S AXI HP0 Interface (DMA용, 선택)
```

**Peripheral I/O Pins:**
```
[✓] SD 0 (microSD)
[✓] UART 1 (또는 UART 0)
[✓] USB 0
[✓] Enet 0
```

**Clock Configuration:**
```
PL Fabric Clocks:
    FCLK_CLK0: 100MHz
```

### 6.5 HLS IP 추가

```
Settings → IP → Repository → Add
└── <HLS_Project>/solution1/impl/ip 경로 추가

Diagram → Add IP → <your_hls_ip>
└── Run Connection Automation
```

### 6.6 인터럽트 연결

```
Add IP → Concat (xlconcat)

연결:
  HLS IP → interrupt → Concat → In0
  Concat → dout → PS → IRQ_F2P[0:0]
```

### 6.7 주소 맵 확인

```
Address Editor 탭
└── 모든 IP의 주소가 할당되었는지 확인
```

### 6.8 합성 및 내보내기

```
Generate HDL Wrapper
Run Synthesis
Run Implementation
Generate Bitstream
Export Hardware (Include bitstream)
```

---

## 7. Vitis HLS 가속기 개발

### 7.1 새 HLS 프로젝트

```
File → New Project...
├── Project Name: memcopy_accel
├── Add Source: memcopy_accel.cpp
├── Add Testbench: memcopy_accel_test.cpp
└── Part: xc7z010clg400-1 (Zybo Z7-10)
        또는 xc7z020clg400-1 (Zybo Z7-20)
```

### 7.2 인터페이스 설계 예제

```cpp
void memcopy_accel(
    uint32_t* src,   // AXI Master - 메모리 읽기
    uint32_t* dst,   // AXI Master - 메모리 쓰기
    uint32_t len     // 바이트 수
) {
    // AXI4 Master 인터페이스 (DDR 접근)
    #pragma HLS INTERFACE m_axi port=src offset=slave bundle=AXI_SRC depth=1024
    #pragma HLS INTERFACE m_axi port=dst offset=slave bundle=AXI_DST depth=1024
    
    // AXI4-Lite 제어 레지스터
    #pragma HLS INTERFACE s_axilite port=src bundle=CTRL_BUS
    #pragma HLS INTERFACE s_axilite port=dst bundle=CTRL_BUS
    #pragma HLS INTERFACE s_axilite port=len bundle=CTRL_BUS
    #pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS
    
    // 구현
    uint32_t num_words = len >> 2;
    for (uint32_t i = 0; i < num_words; i++) {
        #pragma HLS PIPELINE II=1
        dst[i] = src[i];
    }
}
```

### 7.3 HLS 최적화 지시자

| Pragma | 효과 | 사용 예 |
|--------|------|---------|
| `PIPELINE II=N` | 루프 파이프라인, N사이클 간격 | 단순 루프 |
| `UNROLL factor=N` | N배 병렬 전개 | 작은 루프 |
| `ARRAY_PARTITION` | 배열 분할 | 버퍼 병렬 접근 |
| `DATAFLOW` | 태스크 병렬화 | 다단계 처리 |

### 7.4 C 시뮬레이션

```
Project → Run C Simulation
```

테스트벤치에서 황금 참조(Golden Reference)와 결과 비교.

### 7.5 합성

```
Solution → Run C Synthesis
```

**리포트 확인:**
- Latency (사이클 수)
- Interval (II)
- Resource Usage (BRAM, DSP, FF, LUT)

### 7.6 Co-Simulation

```
Solution → Run C/RTL Cosimulation
```

RTL 시뮬레이션으로 기능 검증.

### 7.7 IP 내보내기

```
Solution → Export RTL → IP Catalog
└── 출력: solution1/impl/ip/
```

---

## 8. Vitis 베어메탈 개발

### 8.1 플랫폼 생성

```
File → New → Platform Project...
├── Platform Name: zybo_platform
├── Import XSA: zybo_hw.xsa
├── Processor: ps7_cortexa9_0
├── OS: standalone
└── Generate Boot Components: [✓]
```

### 8.2 BSP 설정

**필수 확인:**
- `xscugic`: 인터럽트 컨트롤러
- `xgpio`: GPIO 드라이버
- `xilffs`: SD 파일시스템 (필요시)

### 8.3 Application 생성

```
File → New → Application Project...
├── Application Name: my_app
├── Platform: zybo_platform
├── Domain: standalone on ps7_cortexa9_0
└── Template: Empty Application
```

### 8.4 인터럽트 핸들링

```c
#include "xscugic.h"

XScuGic gic;
volatile int accel_done = 0;

// ISR
void accel_isr(void *callback) {
    // 인터럽트 플래그 클리어
    Xil_Out32(ACCEL_BASE + ISR_OFFSET, 0x1);
    // AXI 쓰기 플러시를 위한 더미 읽기 (중요!)
    (void)Xil_In32(ACCEL_BASE + ISR_OFFSET);
    accel_done = 1;
}

// GIC 설정
int setup_interrupt() {
    XScuGic_Config *cfg = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    XScuGic_CfgInitialize(&gic, cfg, cfg->CpuBaseAddress);
    
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
        (Xil_ExceptionHandler)XScuGic_InterruptHandler, &gic);
    Xil_ExceptionEnable();
    
    XScuGic_Connect(&gic, ACCEL_IRQ_ID, (Xil_InterruptHandler)accel_isr, NULL);
    XScuGic_Enable(&gic, ACCEL_IRQ_ID);
    
    return 0;
}
```

### 8.5 캐시 관리

```c
#include "xil_cache.h"

// PS → PL 전송 전
Xil_DCacheFlushRange((UINTPTR)src_buf, size);

// PL → PS 수신 후
Xil_DCacheInvalidateRange((UINTPTR)dst_buf, size);
```

### 8.6 WFI (Wait For Interrupt)

```c
memcopy_accel_start(src, dst, len);

// 저전력 대기
while (!accel_done) {
    __asm__ volatile("wfi");
}
```

---

## 9. GPIO 및 주변장치

### 9.1 온보드 LED (MIO)

| LED | MIO 핀 | 설명 |
|-----|--------|------|
| LD0 | MIO 7 | User LED |
| LD1 | MIO 7 | (공유) |
| LD2 | - | (PL 연결) |
| LD3 | - | (PL 연결) |

### 9.2 버튼 및 스위치 (PL)

| 항목 | 핀 수 | 연결 |
|------|-------|------|
| BTN0-BTN3 | 4 | PL GPIO |
| SW0-SW3 | 4 | PL GPIO |
| RGB LED 0-1 | 6 | PL GPIO |

### 9.3 AXI GPIO 예제 (버튼-LED)

**Vivado 설정:**
- AXI GPIO IP 추가
- Channel 1: BTN (4-bit input)
- Channel 2: LED (4-bit output)

**베어메탈 코드:**
```c
#include "xgpio.h"

XGpio gpio;

int main() {
    XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
    
    // BTN = 입력, LED = 출력
    XGpio_SetDataDirection(&gpio, 1, 0xF);  // Channel 1: 입력
    XGpio_SetDataDirection(&gpio, 2, 0x0);  // Channel 2: 출력
    
    while (1) {
        uint32_t btn = XGpio_DiscreteRead(&gpio, 1) & 0xF;
        XGpio_DiscreteWrite(&gpio, 2, btn);  // 버튼 → LED 미러
    }
}
```

### 9.4 PMOD 인터페이스

**PMOD 핀아웃 (Standard 12-pin):**
```
      ┌───┬───┬───┬───┬───┬───┐
Pin:  │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │  (상단)
      ├───┼───┼───┼───┼───┼───┤
      │ 7 │ 8 │ 9 │10 │11 │12 │  (하단)
      └───┴───┴───┴───┴───┴───┘

1-4: I/O 핀 (High-Speed)
5:   GND
6:   VCC (3.3V)
7-10: I/O 핀 (Standard)
11:  GND
12:  VCC (3.3V)
```

---

## 10. 문제 해결

### 10.1 JTAG 연결 실패

**증상:** "Unable to connect to target"

**체크리스트:**
1. JP5 점퍼 제거됨 (JTAG 모드)
2. USB 케이블 연결 상태
3. 드라이버 설치됨 (Vivado에서 자동 설치)
4. 다른 USB 포트 시도

**드라이버 재설치:**
```
<Vivado_Install>/data/xicom/cable_drivers/nt64/
└── install_drivers_wrapper.bat (관리자 권한)
```

### 10.2 합성 오류: 리소스 초과

**증상:** "Place and Route failed: utilization exceeds 100%"

**해결책:**
1. Z7-20으로 업그레이드 (리소스 3배)
2. HLS 최적화 조정:
   - `UNROLL factor` 감소
   - `ARRAY_PARTITION` 타입 변경 (complete → cyclic)
   - 버퍼 크기 축소

### 10.3 인터럽트 핸들러 미호출

**증상:** ISR이 절대 실행되지 않음

**체크리스트:**
1. Block Design에서 인터럽트 연결 확인
   - HLS IP interrupt → Concat → PS IRQ_F2P
2. HLS IP 인터럽트 활성화:
   ```c
   // IP 인터럽트 활성화
   Xil_Out32(ACCEL_BASE + GIE_OFFSET, 0x1);  // Global IE
   Xil_Out32(ACCEL_BASE + IER_OFFSET, 0x1);  // ap_done IE
   ```
3. GIC에서 해당 IRQ 활성화:
   ```c
   XScuGic_Enable(&gic, ACCEL_IRQ_ID);
   ```

### 10.4 인터럽트 스톰 (연속 발생)

**증상:** ISR이 무한 반복 호출됨

**원인:** AXI-Lite 쓰기가 비동기라서 ISR 클리어 전에 리턴

**해결책:**
```c
void accel_isr(void *callback) {
    // 인터럽트 클리어
    Xil_Out32(ACCEL_BASE + ISR_OFFSET, 0x1);
    
    // 중요: 더미 읽기로 AXI 쓰기 버퍼 플러시
    (void)Xil_In32(ACCEL_BASE + ISR_OFFSET);
    
    accel_done = 1;
}
```

### 10.5 데이터 불일치 (캐시 문제)

**증상:** 가속기 출력이 예상과 다름

**원인:** CPU 캐시와 PL 메모리 뷰 불일치

**해결책:**
```c
// 전송 전: CPU 캐시 → DDR 플러시
Xil_DCacheFlushRange((UINTPTR)src, size);

// 가속기 실행...

// 수신 후: DDR → CPU 캐시 무효화
Xil_DCacheInvalidateRange((UINTPTR)dst, size);

// 또는 Non-cacheable 영역 사용
```

---

## 부록: 리소스 비교 (Z7-10 vs Z7-20)

| 리소스 | Z7-10 | Z7-20 | 비율 |
|--------|-------|-------|------|
| Logic Cells | 28K | 85K | 3.0x |
| LUT | 17,600 | 53,200 | 3.0x |
| FF | 35,200 | 106,400 | 3.0x |
| BRAM (Kb) | 2,100 | 4,900 | 2.3x |
| DSP48 | 80 | 220 | 2.75x |

**권장:**
- 학습/프로토타입: Z7-10 충분
- 복잡한 가속기: Z7-20 권장
- 다중 IP 통합: Z7-20 필수

---

*문서 작성일: 2026-02-06*  
*관련 문서: FPGA_Learning_Guide.md, ZCU104_User_Guide.md*
