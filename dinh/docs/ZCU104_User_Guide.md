# ZCU104 Evaluation Kit 상세 사용 설명서

> **보드명:** Xilinx ZCU104 Evaluation Kit  
> **FPGA:** Zynq UltraScale+ MPSoC (XCZU7EV-2FFVC1156)  
> **프로세서:** Quad-core ARM Cortex-A53 (APU) + Dual-core Cortex-R5 (RPU)

---

## 목차

1. [보드 개요](#1-보드-개요)
2. [하드웨어 스펙](#2-하드웨어-스펙)
3. [보드 레이아웃](#3-보드-레이아웃)
4. [부팅 모드 설정](#4-부팅-모드-설정)
5. [연결 방법](#5-연결-방법)
6. [Vivado 하드웨어 설계](#6-vivado-하드웨어-설계)
7. [Vitis 베어메탈 개발](#7-vitis-베어메탈-개발)
8. [SD 카드 사용법](#8-sd-카드-사용법)
9. [LED 및 GPIO](#9-led-및-gpio)
10. [문제 해결](#10-문제-해결)

---

## 1. 보드 개요

ZCU104는 Zynq UltraScale+ MPSoC를 탑재한 고성능 평가 보드입니다.

### 주요 특징

| 특징 | 설명 |
|------|------|
| **MPSoC** | PS (Processing System) + PL (Programmable Logic) 통합 |
| **APU** | Quad-core ARM Cortex-A53 @ 1.2GHz (64-bit) |
| **RPU** | Dual-core ARM Cortex-R5 @ 500MHz (32-bit) |
| **GPU** | ARM Mali-400 MP2 |
| **PL** | 504K Logic Cells, 1,728 DSP Slices |
| **메모리** | 2GB DDR4 (PS), 512MB DDR4 (PL) |

### 적용 분야

- 머신러닝 / AI 추론 가속
- 영상 처리 (4K Video Codec)
- 고속 통신 (100G Ethernet)
- 자동차 / 산업용 임베디드

---

## 2. 하드웨어 스펙

### 프로세서 시스템 (PS)

| 항목 | 사양 |
|------|------|
| APU | 4× Cortex-A53 @ 1.2GHz, 32KB L1 I/D, 1MB L2 |
| RPU | 2× Cortex-R5 @ 500MHz, 32KB L1 I/D, 256KB TCM |
| GPU | Mali-400 MP2 @ 600MHz |
| 온칩 메모리 | 256KB OCM |

### 메모리

| 항목 | 사양 |
|------|------|
| PS DDR4 | 2GB, 64-bit, 2400MT/s |
| PL DDR4 | 512MB, 16-bit |
| QSPI Flash | 128MB (듀얼) |
| eMMC | 8GB |

### 프로그래머블 로직 (PL)

| 리소스 | 수량 |
|--------|------|
| CLB | 65,340 |
| LUT | 504K |
| Flip-Flop | 1,008K |
| Block RAM | 4.5MB (312 × 36Kb) |
| UltraRAM | 1.5MB (96 × 288Kb) |
| DSP Slice | 1,728 |
| GTH Transceiver | 16 (12.5Gbps) |

### I/O 인터페이스

| 인터페이스 | 사양 |
|------------|------|
| PCIe | Gen3 x4 |
| DisplayPort | 1.2a, 4K@60Hz |
| HDMI TX/RX | 2.0 |
| USB | 3.0 × 2, 2.0 × 1 |
| SD | SDIO 3.0 (UHS-I) |
| Ethernet | 1G × 2 |
| UART | 2× (PS), 1× (PL) |
| PMOD | 2× |

---

## 3. 보드 레이아웃

```
┌─────────────────────────────────────────────────────────────────┐
│  [Power]    [JTAG]    [USB3]   [USB3]   [USB2]   [Ethernet×2]   │
│     ●         ■                                                  │
│                                                                  │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                                                          │   │
│  │              Zynq UltraScale+ MPSoC                      │   │
│  │                 XCZU7EV-2FFVC1156                        │   │
│  │                                                          │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                  │
│  [DDR4 2GB]              [DDR4 512MB]                            │
│                                                                  │
│  [PMOD J20]  [PMOD J21]  [FMC LPC]        [DisplayPort]         │
│      ■           ■           ■                 ■                │
│                                                                  │
│  [SD Card]  [QSPI]  [eMMC]                [HDMI TX] [HDMI RX]   │
│      ■                                        ■         ■        │
│                                                                  │
│  [SW19-Status LEDs]  [SW6-DIP]  [User Button]  [Reset]          │
│   ○ ○ ○ ○             ■■■■          ●             ●             │
└─────────────────────────────────────────────────────────────────┘
```

### 주요 커넥터 위치

| 커넥터 | 위치 | 설명 |
|--------|------|------|
| J10 | 상단 좌측 | 12V 전원 입력 |
| J2 | 상단 | USB-JTAG/UART |
| J96 | 중앙 좌측 | microSD 슬롯 |
| SW6 | 하단 중앙 | 부트 모드 DIP 스위치 |
| J20/J21 | 하단 좌측 | PMOD 커넥터 |

---

## 4. 부팅 모드 설정

### SW6 DIP 스위치 설정

**위치:** 보드 하단 중앙, 4-position DIP 스위치

| 모드 | SW6[4:1] | 설명 |
|------|----------|------|
| **JTAG** | 0000 | 개발 모드 (Vivado/Vitis 다운로드) |
| SD (SD1.0) | 0101 | SD 카드 부팅 (권장) |
| eMMC | 0110 | eMMC 부팅 |
| QSPI | 0010 | QSPI Flash 부팅 |
| USB | 0111 | USB 부팅 |

**스위치 읽는 방법:**
- ON = 1, OFF = 0
- 왼쪽(SW1)부터 오른쪽(SW4)

### JTAG 개발 모드 (권장)

```
SW6: [OFF][OFF][OFF][OFF] = 0000
```

- Vivado/Vitis에서 직접 비트스트림/애플리케이션 다운로드
- 개발 중 가장 편리한 모드
- SD 카드 없이도 실행 가능

### SD 부팅 모드

```
SW6: [OFF][ON][OFF][ON] = 0101
```

- SD 카드의 BOOT.BIN 파일에서 부팅
- 독립 실행용

---

## 5. 연결 방법

### 5.1 기본 연결 (개발용)

```
1. 전원 케이블 연결 (J10, 12V)
2. USB 케이블 연결 (J2 → PC)
   - JTAG 프로그래밍
   - UART 시리얼 통신
3. 부트 모드 설정 (SW6 = 0000, JTAG 모드)
4. 전원 스위치 ON (SW1)
```

### 5.2 시리얼 터미널 설정

USB 케이블 연결 후, PC에 2개의 COM 포트가 나타남:
- **COM X**: Interface 0 (JTAG) - 사용 안 함
- **COM Y**: Interface 1 (UART) - 시리얼 통신용

**터미널 설정:**
```
Port: COM Y (높은 번호)
Baud Rate: 115200
Data Bits: 8
Parity: None
Stop Bits: 1
Flow Control: None
```

### 5.3 SD 카드 연결

1. microSD 카드 삽입 (J96)
2. 카드 방향: 금속 접점이 위로, 레이블이 아래로
3. FAT32 포맷 필수

---

## 6. Vivado 하드웨어 설계

### 6.1 새 프로젝트 생성

```
File → New Project...
├── Project Name: zcu104_project
├── Project Location: <your_path>
├── Project Type: RTL Project
└── Default Part:
    └── Boards → zcu104 (Rev 1.0 또는 1.1)
```

### 6.2 Block Design 생성

```
Flow Navigator → Create Block Design
└── Design Name: system
```

### 6.3 Zynq MPSoC 추가

```
Diagram → Add IP → Zynq UltraScale+ MPSoC
└── Run Block Automation
    └── Apply Board Preset: ZCU104
```

### 6.4 PS 설정 확인/수정

더블클릭 → Re-customize IP:

**PS-PL Configuration:**
```
[✓] PS-PL Interfaces
    [✓] Master AXI: M_AXI_HPM0_FPD, M_AXI_HPM1_FPD
    [✓] Slave AXI: S_AXI_HP0_FPD (메모리 접근용)
```

**I/O Configuration:**
```
[✓] SD 0 (MIO 13-22)  - SD 카드
[✓] UART 0 (MIO 18-19) - 또는 UART 1
[✓] TTC 0             - 타이머
```

**Clock Configuration:**
```
PL Fabric Clocks:
    PL0: 100MHz (기본 PL 클럭)
    PL1: 200MHz (고속 로직용, 선택)
```

### 6.5 PL IP 추가 (예: AXI GPIO)

```
Add IP → AXI GPIO
└── Run Connection Automation
    └── Auto-connect to M_AXI_HPM0_FPD
```

### 6.6 HDL Wrapper 및 합성

```
Sources → Right-click design_1 → Create HDL Wrapper
└── Let Vivado manage...

Flow Navigator → Run Synthesis
Flow Navigator → Run Implementation
Flow Navigator → Generate Bitstream
```

### 6.7 하드웨어 내보내기

```
File → Export → Export Hardware...
├── [✓] Include bitstream
├── XSA File: zcu104_hw.xsa
└── Export to: <project_path>
```

---

## 7. Vitis 베어메탈 개발

### 7.1 플랫폼 프로젝트 생성

```
File → New → Platform Project...
├── Platform Name: zcu104_platform
├── Create from XSA: zcu104_hw.xsa
├── Operating System: standalone
├── Processor: psu_cortexa53_0
└── Generate Boot Components: [✓]
```

### 7.2 BSP 설정

```
Platform Explorer → psu_cortexa53_0 → Board Support Package
└── Modify BSP Settings
```

**필수 라이브러리:**

| 라이브러리 | 용도 | 설정 |
|------------|------|------|
| xilffs | FatFs (SD 파일시스템) | fs_interface = 1 (SD) |
| xilpm | 전원 관리 | 기본값 |
| xilsecure | 보안 기능 | 필요시 |

**xilffs 설정:**
```
fs_interface: 1 (SD)
read_only: false
use_lfn: 1 (Long File Name)
word_access: false
use_strfunc: 0
```

### 7.3 Application 프로젝트 생성

```
File → New → Application Project...
├── Application Name: my_app
├── Platform: zcu104_platform
├── Domain: standalone on psu_cortexa53_0
├── Template: Empty Application (C++)
└── Finish
```

### 7.4 링커 스크립트 설정

```
Project → Generate Linker Script...
```

| 섹션 | 메모리 영역 | 권장 크기 |
|------|------------|-----------|
| .heap | psu_ddr_0 | 4MB (0x400000) |
| .stack | psu_ddr_0 | 64KB (0x10000) |
| .text | psu_ddr_0 | - |
| .data | psu_ddr_0 | - |

### 7.5 빌드 및 실행

```
Project → Build Project  (Ctrl+B)

Run → Debug Configurations...
├── Xilinx C/C++ Application (System Debugger)
├── Target Connection: Local
└── Apply → Debug
```

---

## 8. SD 카드 사용법

### 8.1 카드 포맷

- **파일 시스템:** FAT32 (권장), FAT16
- **클러스터 크기:** 32KB 권장
- **포맷 도구:** Windows 기본 또는 SD Card Formatter

### 8.2 베어메탈 파일 I/O

```c
#include "ff.h"  // FatFs 헤더

static FATFS fatfs;
static FIL file;

// SD 카드 마운트
int sd_init() {
    FRESULT rc = f_mount(&fatfs, "0:/", 1);
    if (rc != FR_OK) {
        xil_printf("Mount failed: %d\r\n", rc);
        return -1;
    }
    xil_printf("SD mounted successfully\r\n");
    return 0;
}

// 파일 읽기
int read_file(const char* path, uint8_t* buffer, uint32_t size) {
    UINT br;
    FRESULT rc = f_open(&file, path, FA_READ);
    if (rc != FR_OK) return -1;
    
    rc = f_read(&file, buffer, size, &br);
    f_close(&file);
    
    return (rc == FR_OK) ? (int)br : -1;
}

// 파일 쓰기
int write_file(const char* path, const uint8_t* buffer, uint32_t size) {
    UINT bw;
    FRESULT rc = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (rc != FR_OK) return -1;
    
    rc = f_write(&file, buffer, size, &bw);
    f_close(&file);
    
    return (rc == FR_OK) ? (int)bw : -1;
}
```

### 8.3 파일 경로 규칙

```
"0:/file.txt"           - 루트의 file.txt
"0:/folder/data.bin"    - 폴더 내 파일
"0:/FOLDER/FILE.TXT"    - 대소문자 구분 없음 (FAT)
```

**주의사항:**
- 경로 구분자: `/` (백슬래시 아님)
- 드라이브 번호: `0:` (SD0)
- Long File Name 지원: BSP에서 `use_lfn=1` 설정 필요

---

## 9. LED 및 GPIO

### 9.1 온보드 LED

| LED | 기능 | 설명 |
|-----|------|------|
| DS36 (PWR) | 전원 | 보드 전원 상태 |
| DS35 (DONE) | FPGA Done | PL 설정 완료 |
| DS37-40 | Status | 사용자 정의 가능 (GPIO) |

### 9.2 MIO GPIO (PS)

```c
#include "xgpiops.h"

XGpioPs gpio;
XGpioPs_Config *config;

// 초기화
config = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
XGpioPs_CfgInitialize(&gpio, config, config->BaseAddr);

// LED 제어 (예: MIO 26)
XGpioPs_SetDirectionPin(&gpio, 26, 1);  // 1 = 출력
XGpioPs_SetOutputEnablePin(&gpio, 26, 1);
XGpioPs_WritePin(&gpio, 26, 1);  // LED ON
```

### 9.3 EMIO GPIO (PS-PL)

```c
// EMIO는 GPIO 78번부터 시작
#define EMIO_BASE 78

// EMIO 0번 핀 = 78 + 0 = 78
XGpioPs_WritePin(&gpio, EMIO_BASE + 0, value);
```

### 9.4 AXI GPIO (PL)

Vivado에서 AXI GPIO IP 추가 후:

```c
#include "xgpio.h"

XGpio axi_gpio;

// 초기화
XGpio_Initialize(&axi_gpio, XPAR_AXI_GPIO_0_DEVICE_ID);

// 방향 설정 (0 = 출력, 1 = 입력)
XGpio_SetDataDirection(&axi_gpio, 1, 0x00);  // 채널 1 전체 출력

// 쓰기
XGpio_DiscreteWrite(&axi_gpio, 1, 0x0F);  // LED[3:0] ON

// 읽기
uint32_t value = XGpio_DiscreteRead(&axi_gpio, 1);
```

---

## 10. 문제 해결

### 10.1 JTAG 연결 실패

**증상:** "Unable to detect target" 오류

**해결책:**
1. USB 케이블 재연결
2. 드라이버 재설치 (Vivado Lab Tools)
3. 부트 모드가 JTAG인지 확인 (SW6 = 0000)
4. 보드 전원 사이클

```
Vivado Hardware Manager → Open Target → Auto Connect
```

### 10.2 SD 카드 마운트 실패

**증상:** `f_mount` 반환값이 FR_OK가 아님

| 에러 코드 | 의미 | 해결 |
|-----------|------|------|
| FR_NOT_READY (3) | 드라이브 준비 안됨 | 카드 삽입 확인, FAT32 포맷 |
| FR_NO_FILESYSTEM (13) | 파일시스템 없음 | FAT32로 포맷 |
| FR_DISK_ERR (1) | 디스크 오류 | 다른 카드 시도 |

**디버깅 코드:**
```c
FRESULT rc = f_mount(&fatfs, "0:/", 1);
xil_printf("f_mount returned: %d\r\n", rc);
```

### 10.3 UART 출력 안됨

**확인 사항:**
1. 올바른 COM 포트 선택 (2개 중 높은 번호)
2. 보드레이트 115200 확인
3. BSP에서 UART 드라이버 활성화 확인
4. xil_printf 대신 print() 시도

```c
// 대안 1: xil_printf (권장)
xil_printf("Hello World\r\n");

// 대안 2: Standalone stdout
print("Hello World\r\n");

// 대안 3: 저수준 UART
outbyte('H'); outbyte('i'); outbyte('\r'); outbyte('\n');
```

### 10.4 애플리케이션 크래시

**증상:** 실행 직후 멈춤 또는 재부팅

**해결책:**
1. **스택 오버플로우**: 링커 스크립트에서 스택 크기 증가
2. **힙 부족**: 동적 할당 크기 확인, 힙 크기 증가
3. **메모리 정렬**: 캐시 라인 정렬 확인 (64바이트)

```c
// 정렬된 버퍼 선언
__attribute__((aligned(64))) uint8_t buffer[4096];
```

### 10.5 PL 클럭 문제

**증상:** PL IP가 동작하지 않음

**확인 사항:**
1. PS에서 PL 클럭 활성화 (PL0, PL1 등)
2. Block Design에서 클럭 연결 확인
3. Reset 신호 연결 확인

```
Zynq MPSoC → Re-customize IP → Clock Configuration
└── [✓] PL Fabric Clocks → PL0 = 100MHz
```

---

## 부록: 유용한 리소스

### 공식 문서

| 문서 | 내용 |
|------|------|
| UG1085 | Zynq UltraScale+ MPSoC TRM |
| UG1137 | Software Developer Guide |
| UG643 | Standalone BSP Libraries |
| UG1267 | ZCU104 Board User Guide |

### 예제 코드

```
<Vitis_Install>/examples/platforms/zcu104/
```

### 지원 포럼

- Xilinx Community Forums: https://forums.xilinx.com
- AMD/Xilinx GitHub: https://github.com/Xilinx

---

*문서 작성일: 2026-02-06*  
*관련 문서: FPGA_Learning_Guide.md, Zybo_User_Guide.md*
