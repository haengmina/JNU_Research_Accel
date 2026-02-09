//////////////////////////////////////////////////////////////////////////////
// 
// 프로그램의 시작점
// MobileNetV1 모델을 생성하고 무작위 입력에 대해 추론을 수행
//
// email: brothermin00@gmail.com
// project : JNU Lab ML MobileNetV1 C++ LibTorch Implementation
// 2026-01-05
//
///////////////////////////////////////////////////////////////////////////////


#include <torch/torch.h>
#include <iostream>

#include "mobilenet_v1.hpp"                                           // MobileNetV1 클래스 정의 포함된 헤더 파일

int main() {
  const int num_classes = 1000;
  auto net = MobileNetV1(num_classes, 1.0f);                          // MobileNetV1 모델 생성 net에 저장 (1000 클래스, 1.0 배율(채널 폭, 기본값))

  const int batch_size = 100;
  const int epochs = 20;
  net->train();                                                       // 학습 모드로 전환
  torch::optim::SGD optimizer(net->parameters(), 0.01);               // 간단한 SGD 옵티마이저

  // 에폭 전체 평균을 계산하기 위한 누적 변수
  float total_loss = 0.0f;
  float total_acc = 0.0f;

  for (int epoch = 0; epoch < epochs; ++epoch) {
    auto inputs = torch::randn({batch_size, 3, 224, 224});            // 더미 입력
    auto targets = torch::randint(0, num_classes, {batch_size},
                                  torch::TensorOptions().dtype(torch::kLong)); // 더미 라벨

    optimizer.zero_grad();
    auto outputs = net->forward(inputs);
    auto loss = torch::nn::functional::cross_entropy(outputs, targets);   // 교차 엔트로피 손실 계산
    loss.backward();
    optimizer.step();

    // 배치 단위 정확도 계산
    auto preds = outputs.argmax(1);
    auto correct = preds.eq(targets).sum().item<float>();
    float acc = correct / static_cast<float>(batch_size);
    float loss_value = loss.item<float>();
    total_loss += loss_value;
    total_acc += acc;

    std::cout << "Epoch " << epoch + 1
              << " loss: " << loss_value
              << " acc: " << acc << "\n";
  }

  // 전체 에폭에 대한 평균 loss/accuracy 출력
  std::cout << "Avg loss: " << (total_loss / epochs)
            << " Avg acc: " << (total_acc / epochs) << "\n";

  net->eval();                                                        // 추론(test) 모드로 전환
  torch::NoGradGuard no_grad;                                         // 그래디언트 계산 비활성화 (추론 시 메모리 절약 및 속도 향상). 학습 안하는다는 뜻
  auto input = torch::randn({1, 3, 224, 224});                        // 무작위 입력 텐서 생성 (배치 크기 1, 3 채널, 224x224 이미지)
  auto output = net->forward(input);                                  // 모델에 입력 전달하여 출력 얻기          
  std::cout << "Output shape: " << output.sizes() << "\n";            // 출력 텐서의 크기 출력 [배치 크기, 클래스 수]
  return 0;
}
