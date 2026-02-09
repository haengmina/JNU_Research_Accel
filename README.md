# JNU Research Accelerator - FPGA AI Project

This repository contains the workspace and reference materials for developing FPGA-based AI accelerators on Xilinx ZCU104 and Zybo Z7-10 platforms.

## Directory Structure

### 1. `hm_workspace/` (Active Development)
This is the primary workspace for Xilinx Vivado and Vitis projects.
- **Tools used:** Vivado 2022.2, Vitis 2022.2
- **Contents:**
  - `imgio/project_zcu104_imgio/`: Vivado hardware project with Zynq UltraScale+ MPSoC block design.
  - `imgio/vitis_imgio_zcu104/`: Vitis platform project (`zcu104_imgio_hw`) and application code (`imgio_app`).
  - Contains build artifacts, bitstreams, and launch configurations.

### 2. `dinh/` (Reference & Documentation)
Contains the original reference implementations and learning guides.
- **Documentation:** `docs/FPGA_Learning_Guide.md` (The main guide we are following).
- **Source Code:** Reference C++ code for bare-metal applications (e.g., `zcu104-baremetal-imgio`).
- **Assets:** Test images (`bmp_24.bmp`) and other resources.

### 3. `onnx_project/` (Deep Learning Models)
Contains deep learning frameworks and model files for AI inference tasks.
- **Frameworks:** TensorFlow, Caffe, ONNX.
- **Models:** MobileNetV1, MobileNetV2, SSDLite.
- **Purpose:** Used for training models, quantization, and exporting weights for the FPGA accelerator.

## Getting Started

1. **Hardware Setup:** Connect ZCU104 board via JTAG and UART.
2. **Software:** Launch Vitis 2022.2 and open the workspace `hm_workspace`.
3. **Build Flow:**
   - Open the Platform Project (`zcu104_imgio_hw`) and build.
   - Open the Application Project (`imgio_app`) and build.
   - Run on hardware using "Launch on Hardware".

## License
This project is part of the JNU Machine Learning Lab research.
