# JNU 연구 가속기 - FPGA AI 프로젝트

이 리포지토리는 Xilinx ZCU104 및 Zybo Z7-10 플랫폼 기반의 FPGA AI 가속기 개발을 위한 작업 공간 및 참조 자료를 포함하고 있습니다.

## 디렉토리 구조

### 1. `hm_workspace/` (현재 개발 공간)
Xilinx Vivado 및 Vitis 프로젝트를 위한 주요 작업 공간입니다.
- **사용 도구:** Vivado 2022.2, Vitis 2022.2
- **내용:**
  - `imgio/project_zcu104_imgio/`: Zynq UltraScale+ MPSoC 블록 디자인이 포함된 Vivado 하드웨어 프로젝트.
  - `imgio/vitis_imgio_zcu104/`: Vitis 플랫폼 프로젝트 (`zcu104_imgio_hw`) 및 애플리케이션 코드 (`imgio_app`).
  - 빌드 결과물(artifacts), 비트스트림, 실행 설정(launch configurations)을 포함합니다.

### 2. `dinh/` (참조 및 문서)
원본 참조 구현체와 학습 가이드를 포함합니다.
- **문서:** `docs/FPGA_Learning_Guide.md` (현재 따르고 있는 메인 가이드).
- **소스 코드:** 베어메탈 애플리케이션용 참조 C++ 코드 (예: `zcu104-baremetal-imgio`).
- **에셋:** 테스트 이미지 (`bmp_24.bmp`) 및 기타 리소스.
- **URL:** https://github.com/vandinhtranengg

### 3. `onnx_project/` (딥러닝 모델)
AI 추론 작업을 위한 딥러닝 프레임워크 및 모델 파일을 포함합니다.
- **프레임워크:** TensorFlow, Caffe, ONNX.
- **모델:** MobileNetV1, MobileNetV2, SSDLite.
- **목적:** FPGA 가속기를 위한 모델 학습, 양자화(quantization), 가중치(weights) 내보내기에 사용됩니다.

## 시작하기 (Getting Started)

1. **하드웨어 설정:** JTAG 및 UART를 통해 ZCU104 보드를 연결합니다.
2. **소프트웨어:** Vitis 2022.2를 실행하고 `hm_workspace` 작업 공간을 엽니다.
3. **빌드 흐름:**
   - 플랫폼 프로젝트 (`zcu104_imgio_hw`)를 열고 빌드합니다.
   - 애플리케이션 프로젝트 (`imgio_app`)를 열고 빌드합니다.
   - "Launch on Hardware"를 사용하여 하드웨어에서 실행합니다.

## 라이선스
이 프로젝트는 JNU 머신러닝 연구실 연구의 일부입니다.
