# Zybo Z7-10 메모리 복사 가속기 (인터럽트 기반)

이 프로젝트는 **Vivado HLS**를 사용하여 설계된 하드웨어 가속 메모리 복사(memcpy) 기능을 **Zybo Z7-10** 보드의 **Zynq PS**(ARM Cortex-A9)와 통합한 예제입니다.

## 🚀 프로젝트 개요
이 가속기는 **인터럽트 기반 메커니즘**을 사용하여 동작하며, PS와 PL(FPGA fabric) 사이의 효율적인 동기화를 보장합니다. CPU가 직접 메모리를 복사하는 대신 하드웨어 가속기가 DMA(Direct Memory Access)를 통해 데이터를 전송하므로 CPU 점유율을 낮추고 성능을 높일 수 있습니다.

## 📂 주요 구조
- **src/Vitis-HLS/**: 하드웨어 가속기 로직 (C++ HLS)
- **src/Vitis-BareMetal/**: 베어메탈 환경을 위한 소프트웨어 드라이버 및 메인 어플리케이션
- **MemAcc_app/**: Vitis IDE용 어플리케이션 소스 코드

## 🛠 핵심 기술
1. **AXI4 Master Interface**: 가속기가 DDR 메모리에 직접 접근하여 데이터를 읽고 씁니다.
2. **Interrupt System**: 가속 완료 시 CPU에 인터럽트를 발생시켜 작업 완료를 알립니다.
3. **Cache Coherency**: `Xil_DCacheFlushRange` 및 `Xil_DCacheInvalidateRange`를 사용하여 소프트웨어와 하드웨어 간의 데이터 일관성을 유지합니다.

## 💻 실행 방법
1. Vivado를 통해 하드웨어 비트스트림을 생성합니다.
2. Vitis에서 플랫폼 및 어플리케이션 프로젝트를 생성합니다.
3. Zybo Z7-10 보드에 연결하여 어플리케이션을 실행하고 시리얼 터미널(UART)을 통해 성능 결과를 확인합니다.

---
**작성자:** Van Dinh Tran  
**보드:** Zybo Z7-10  
**도구:** Vivado / Vitis 2022.1  
