////////////////////////////////////////////////////////////////////////////////////
//
// mobilenet_v1.cpp
// MobileNetV1 모델 클래스 정의
// 
// email: brothermin00@gmail.com
// project : JNU Lab ML MobileNetV1 C++ LibTorch Implementation
// 2026-01-05
// 
// channels, strides → MobileNetV1 구조의 설계도
// features → 실제 레이어들을 순서대로 저장
// avgpool + classifier → 최종 분류를 위한 부분
// forward → 입력이 모델을 통과하는 순서 정의
// 
///////////////////////////////////////////////////////////////////////////////////

#include "mobilenet_v1.hpp"                                                               // MobileNetV1 클래스 정의 포함된 헤더 파일         

#include <algorithm>                                                                      // std::max 사용 위해 포함 
#include <cmath>                                                                          // std::round 사용 위해 포함  
#include <vector>                                                                         // std::vector 사용 위해 포함 

namespace {                                                                               // 파일 안에서만 쓰는 '내부 함수' 영역

int make_depth(int depth, float multiplier, int min_depth) {                              // 채널 수(depth)를 폭 조정 비율(multiplier)과 최소 깊이(min_depth)를 고려해 계산
  int d = static_cast<int>(std::round(depth * multiplier));                               // depth * multiplier 후 반올림
  return std::max(min_depth, d);                                                          // 최소 깊이와 비교해 더 큰 값 반환
}

}  // namespace

torch::Tensor ReLU6Impl::forward(const torch::Tensor& x) {                                // ReLU6 활성화 함수 구현
  return torch::clamp(x, 0.0, 6.0);                                                       // 입력 x를 0과 6 사이로 클램핑하여 ReLU6 효과 구현
}

ConvBNReLUImpl::ConvBNReLUImpl(int in_channels, int out_channels, int kernel_size,        // Conv2d 옵션 설정, conv, bn, relu 모듈 생성 및 등록
                               int stride, int padding, int groups) {                     
  auto options = torch::nn::Conv2dOptions(in_channels, out_channels, kernel_size)           
                     .stride(stride)
                     .padding(padding)
                     .groups(groups)
                     .bias(false);
  conv = register_module("conv", torch::nn::Conv2d(options));                             // Conv2d 모듈 등록. "conv" 이름으로 -> 내부 멤버 변수 conv에 저장
  bn = register_module("bn", torch::nn::BatchNorm2d(out_channels));                       // BatchNorm2d 모듈 등록. "bn" 이름으로 -> 내부 멤버 변수 bn에 저장. out_channels 기준으로 정규화
  relu = register_module("relu", ReLU6());                                                // ReLU6 모듈 등록. "relu" 이름으로 -> 내부 멤버 변수 relu에 저장
}

torch::Tensor ConvBNReLUImpl::forward(const torch::Tensor& x) {
  return relu->forward(bn->forward(conv->forward(x)));                                   // 입력 x를 Conv2d -> BatchNorm2d -> ReLU6 순으로 처리 후 반환   
}

DepthwiseSeparableImpl::DepthwiseSeparableImpl(int in_channels, int out_channels,
                                               int stride) {
  depthwise = register_module(
      "depthwise",
      ConvBNReLU(in_channels, in_channels, 3, stride, 1, in_channels));                  // Depthwise Convolution : 그룹 수를 입력 채널 수와 동일하게 설정
  pointwise = register_module(
      "pointwise",
      ConvBNReLU(in_channels, out_channels, 1, 1, 0, 1));                                // Pointwise Convolution : 1x1 커널 사용, 그룹 수 1로 설정
}

torch::Tensor DepthwiseSeparableImpl::forward(const torch::Tensor& x) {
  auto out = depthwise->forward(x);
  return pointwise->forward(out);
}

MobileNetV1Impl::MobileNetV1Impl(int num_classes, float width_multiplier,                // MobileNetV1 모델 전체 구조 정의
                                 int min_depth) {
  const std::vector<int> channels = {
      32, 64, 128, 128, 256, 256, 512, 512, 512, 512, 512, 512, 1024, 1024};
  const std::vector<int> strides = {
      2, 1, 2, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1};                                         // stride = 2 면 다운샘플링 수행. input 크기 통일

  const int c0 = make_depth(channels[0], width_multiplier, min_depth);
  features = register_module("features", torch::nn::Sequential());                       // 여러 레이어를 순차적으로 쌓는 시퀀셜 모듈(컨테이너) 생성 및 등록. "features" 이름으로 관리
  features->push_back(ConvBNReLU(3, c0, 3, strides[0], 1, 1));                           // 첫 번째 레이어는 ConvBNReLU 블록. c0 : 출력 채널 수. 1,1: 패딩

  int in_c = c0;
  for (std::size_t i = 1; i < channels.size(); ++i) {                                    // 나머지 레이어들은 DepthwiseSeparable 블록으로 구성                   
    const int out_c = make_depth(channels[i], width_multiplier, min_depth);              // 다음 레이어의 출력 채널 수 계산
    features->push_back(DepthwiseSeparable(in_c, out_c, strides[i]));
    in_c = out_c;                                                                        // 다음 반복을 위해 입력 채널 수 갱신
  }

  avgpool = register_module("avgpool", torch::nn::AdaptiveAvgPool2d(
                                           torch::nn::AdaptiveAvgPool2dOptions(1)));     // 마지막 특징맵의 공간 크기를 1x1로 줄이는 적응형 평균 풀링
  classifier = register_module("classifier", torch::nn::Linear(in_c, num_classes));      // 최종 분류를 위한 완전 연결 층.
}

torch::Tensor MobileNetV1Impl::forward(const torch::Tensor& x) {
  auto out = features->forward(x);                                                       // 입력 x를 features 시퀀셜 모듈에 순차적으로 통과시켜 특징 추출      
  out = avgpool->forward(out);                                                           // 추출된 특징맵을 평균 풀링하여 공간 크기 1x1로 축소
  out = out.view({out.size(0), -1});                                                     // 1x1 특징맵을 벡터 형태로 변환 (flatten)
  return classifier->forward(out);                                                       // 벡터를 분류기(classifier)에 통과시켜 최종 클래스 점수 출력
}
