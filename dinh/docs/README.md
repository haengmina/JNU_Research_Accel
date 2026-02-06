# FPGA 가속기 개발 학습 문서

> ZCU104 및 Zybo Z7 보드를 활용한 FPGA 가속기 개발 종합 학습 자료

---

## 문서 목록

### 1. 학습 가이드

| 문서 | 설명 | 난이도 |
|------|------|:------:|
| **[FPGA_Learning_Guide.md](./FPGA_Learning_Guide.md)** | 종합 학습 로드맵 및 커리큘럼 | 전체 |
| **[Lab_Practice_Guide.md](./Lab_Practice_Guide.md)** | 프로젝트별 상세 실습 가이드 | ★★★☆☆ |

### 2. 보드 사용 설명서

| 문서 | 대상 보드 | 주요 내용 |
|------|----------|----------|
| **[ZCU104_User_Guide.md](./ZCU104_User_Guide.md)** | ZCU104 | 하드웨어 스펙, 부팅 모드, Vivado/Vitis 설정 |
| **[Zybo_User_Guide.md](./Zybo_User_Guide.md)** | Zybo Z7-10/20 | 하드웨어 스펙, HLS 통합, 인터럽트 설정 |

### 3. 개발 가이드

| 문서 | 주제 | 주요 내용 |
|------|------|----------|
| **[HLS_Development_Guide.md](./HLS_Development_Guide.md)** | Vitis HLS | 인터페이스 설계, 최적화, 검증 워크플로우 |
| **[Troubleshooting_FAQ.md](./Troubleshooting_FAQ.md)** | 문제 해결 | 일반적인 오류 및 FAQ |

---

## 프로젝트 개요

### 프로젝트 1: zcu104-baremetal-imgio
- **목적:** SD 카드에서 BMP 이미지 로드
- **보드:** ZCU104
- **핵심 학습:** FatFs, BMP 파싱, NHWC 레이아웃

### 프로젝트 2: zybo_memcopy_accel_interrupt
- **목적:** HLS 기반 메모리 복사 가속기
- **보드:** Zybo Z7-10
- **핵심 학습:** HLS 인터페이스, AXI 버스트, 인터럽트

### 프로젝트 3: zcu104-a53-mobilenetv1-baseline
- **목적:** 양자화된 CNN으로 숫자 인식
- **보드:** ZCU104
- **핵심 학습:** INT8 양자화, DW/PW Conv, PyTorch 학습

---

## 권장 학습 순서

```
Week 1-2     Week 2-3           Week 4-5           Week 6+
┌─────────┐  ┌───────────────┐  ┌────────────────┐  ┌─────────────┐
│ SD BMP  │→ │ HLS Memcopy   │→ │ CNN 숫자 인식  │→ │ HLS IP 구현 │
│ 로더    │  │ 가속기        │  │ (MobileNet)    │  │ (도전)      │
└─────────┘  └───────────────┘  └────────────────┘  └─────────────┘
  초급           중급              고급              고급+
```

---

## 개발 환경

| 소프트웨어 | 버전 | 용도 |
|------------|------|------|
| Vivado | 2022.1 | 하드웨어 설계 |
| Vitis | 2022.1 | 소프트웨어 개발 |
| Vitis HLS | 2022.1 | C→RTL 합성 |
| Python | 3.8+ | 모델 학습 (선택) |

---

## 빠른 시작

### 1. 첫 프로젝트 실행 (SD BMP 로더)

```bash
# 1. SD 카드 준비
- FAT32 포맷
- test.bmp 파일 복사 (24-bit BMP)

# 2. Vivado
- ZCU104 프로젝트 생성
- PS 설정 (SD, UART 활성화)
- XSA 내보내기

# 3. Vitis
- 플랫폼 생성 (xilffs 라이브러리 추가)
- Application 생성
- sd_bmp_loader.cpp 추가
- 빌드 및 실행
```

### 2. HLS 가속기 실행 (Memcopy)

```bash
# 1. Vitis HLS
- memcopy_accel.cpp 프로젝트 생성
- C Simulation → Synthesis → Export IP

# 2. Vivado
- Block Design에 HLS IP 추가
- 인터럽트 연결
- 비트스트림 생성

# 3. Vitis
- 베어메탈 드라이버 작성
- 인터럽트 핸들링 구현
- 실행 및 검증
```

---

## 문서 활용 팁

1. **처음 시작:** `FPGA_Learning_Guide.md`의 로드맵 확인
2. **실습 진행:** `Lab_Practice_Guide.md`의 단계별 가이드 따라하기
3. **보드 설정:** 해당 보드의 User Guide 참조
4. **HLS 개발:** `HLS_Development_Guide.md`의 Pragma 참조
5. **문제 발생:** `Troubleshooting_FAQ.md`에서 해결책 검색

---

## 관련 리소스

### 공식 문서
- [Zynq UltraScale+ MPSoC TRM (UG1085)](https://docs.xilinx.com/v/u/en-US/ug1085-zynq-ultrascale-trm)
- [Vitis HLS User Guide (UG1399)](https://docs.xilinx.com/r/en-US/ug1399-vitis-hls)
- [ZCU104 User Guide (UG1267)](https://docs.xilinx.com/v/u/en-US/ug1267-zcu104-eval-bd)

### 예제 저장소
- [Xilinx Vitis Tutorials](https://github.com/Xilinx/Vitis-Tutorials)
- [Xilinx Vitis HLS Examples](https://github.com/Xilinx/Vitis-HLS-Introductory-Examples)

---

*문서 작성일: 2026-02-06*
