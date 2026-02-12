////////////////////////////////////////////////////////////////////////////////////
//
// mobilenet_v1.hpp
// MobileNetV1 모델 클래스 정의
// 
// email: brothermin00@gmail.com
// project : JNU Lab ML MobileNetV1 C++ LibTorch Implementation
// 2026-01-05
// 
///////////////////////////////////////////////////////////////////////////////////


#pragma once

#include <torch/torch.h>

struct ReLU6Impl : torch::nn::Module {                                             // ReLU6 활성화 함수 구현
  torch::Tensor forward(const torch::Tensor& x);                                   // 순전파 메서드 정의 : forward 함수는 입력 텐서 x를 받아 ReLU6 활성화 적용 후 반환           
};
TORCH_MODULE(ReLU6);                                                               // ReLU6 모듈 정의 매크로            

struct ConvBNReLUImpl : torch::nn::Module {                                        // 합성곱(Conv2d) + 배치 정규화(BN) + ReLU6 블록 구현
  ConvBNReLUImpl(int in_channels, int out_channels, int kernel_size,
                 int stride, int padding, int groups = 1);                         // 생성자 : 합성곱 층의 입력 채널, 출력 채널, 커널 크기, 스트라이드, 패딩, 그룹 수 설정

  torch::Tensor forward(const torch::Tensor& x);                                   // 순전파 메서드 정의 : forward 함수는 입력 텐서 x를 받아 Conv2d -> BN -> ReLU6 순으로 처리 후 반환

  torch::nn::Conv2d conv{nullptr};                                                 
  torch::nn::BatchNorm2d bn{nullptr};                                              
  ReLU6 relu{nullptr};
};
TORCH_MODULE(ConvBNReLU);                                                          // ConvBNReLU 모듈 정의 매크로

struct DepthwiseSeparableImpl : torch::nn::Module {                                // 깊이별 분리 합성곱 블록 구현 : Depthwise + Pointwise 합성곱
  DepthwiseSeparableImpl(int in_channels, int out_channels, int stride);           // 생성자 : 입력 채널, 출력 채널, 스트라이드 설정

  torch::Tensor forward(const torch::Tensor& x);

  ConvBNReLU depthwise{nullptr};
  ConvBNReLU pointwise{nullptr};
};
TORCH_MODULE(DepthwiseSeparable);

struct MobileNetV1Impl : torch::nn::Module {                                      // MobileNetV1 모델 구현. 전체 네트워크 구조 정의
  MobileNetV1Impl(int num_classes = 1000, float width_multiplier = 1.0f,          // width_multiplier : 채널 폭 조정 비율 (기본값 1.0f)
                  int min_depth = 8);                                             // 생성자 : 클래스 수, 채널 폭 배율, 최소 깊이 설정     

  torch::Tensor forward(const torch::Tensor& x);

  torch::nn::Sequential features;                                                 // 여러 개의 Conv/DepthwiseSeparable 블록을 순차적으로 포함하는 시퀀셜 모듈
  torch::nn::AdaptiveAvgPool2d avgpool{nullptr};                                  // 마지막 특징맵을 평균 풀링 
  torch::nn::Linear classifier{nullptr};                                          // 최종 분류를 위한 완전 연결 층
};
TORCH_MODULE(MobileNetV1);
