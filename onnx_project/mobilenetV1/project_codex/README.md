# MobileNet V1

Depthwise conv + Pointwise conv

Width Multiplier(α), Resolution Multiplier(ρ)

# Implementation

language : C++

Framework : LibTorch (PyTorch)

### Framework 비교

| 구분 | LibTorch (PyTorch C++) | TensorFlow C++ API | ONNX Runtime / TensorRT |
| :--- | :--- | :--- | :--- |
| 주용도 | 모델 생성, 학습, 추론 (All-in-one) | 주로 추론 (학습은 매우 복잡) | 고성능 추론 전용|
| 난이도 | 중 (Python PyTorch와 유사) | 상 (문서 부족, 복잡) | 중 (초기 설정 필요) |
| 장점 | C++만으로 전체 파이프라인 구축 가능 | Tensorflow 생태계 호환 | 최고의 추론 속도 및 호환성 |
| 추천 대상 | C++ 환경에서 학습까지 필요한 연구/개발 | 기존 TF 레거시가 있는 경우 | 상용 제품 배포, 엣지 디바이스 |
---
<br>
<br>

# 환경 구축
1. 준비 확인

    가장 먼저 결정
    - CPU만 사용할 지 CUDA(GPU)까지 사용할지 결정 : CPU-only
2. 기본 폴더 구조
    <pre>
    project/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── build/
    └── README.md
    </pre>
3. LibTorch 다운로드
    PyTorch 공식 페이지에서 LibTorch 다운로드 : 
    <br> https://pytorch.org/get-started/locally/
4. CMake 설정

5. C++ 코드 생성

6. 빌드 & 실행



# Reference

paper : https://doi.org/10.48550/arXiv.1704.04861

### code
official tensorflow : https://github.com/tensorflow/models 

Mobilenet on Zynq (Mintzh) : https://github.com/Mintzh/MobilenetOnZynq

