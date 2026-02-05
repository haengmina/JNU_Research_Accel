<!--VScode Markdown Preview : 
    현재 파일 미리 보기 : Ctrl + Shift + V 
    분할 화면으로 미리 보기 : Ctrl + K -> V -->

# MobilenetV2 이후 모델
MobilenetV2 및 그 이후 모델에 대한 내용은 [mobilenet/README.md](mobilenet/README.md) 파일을 참조하십시오.

# MobileNetV1

[MobileNets](https://arxiv.org/abs/1704.04861)는 다양한 사용 사례의 리소스 제약을 만족시키기 위해 설계된 소형, 저지연, 저전력 딥러닝 모델입니다. MobileNet은 Inception과 같은 대규모 모델이 사용되는 방식과 유사하게 분류(Classification), 객체 검출(Detection), 임베딩(Embedding) 및 분할(Segmentation) 작업에 활용될 수 있습니다. MobileNet은 [TensorFlow Lite](https://www.tensorflow.org/lite)를 통해 모바일 디바이스에서 효율적으로 실행할 수 있습니다.

MobileNet은 지연 시간(latency), 모델 크기(size) 및 정확성(accuracy) 간의 trade-off를 제공하면서도, 기존 문헌에 등장하는 유명 모델들과 비교했을 때 경쟁력 있는 성능을 보여줍니다.

![alt text](mobilenet_v1.png "MobileNet 그래프")

# 사전 학습된 모델 (Pre-trained Models)

지연 시간 및 크기 예산에 맞는 올바른 MobileNet 모델을 선택할 수 있습니다.
* 모델 크기(메모리/디스크)는 매개변수 수(파라미터 수)에 비례합니다.
* 지연 시간 과 전력 소모는 MAC(Multiply-Accumulates) 수에 비례합니다. MAC은 곱셈과 덧셈이 결합된 연산의 개수를 의미합니다.

이러한 MobileNet 모델은
[ILSVRC-2012-CLS](http://www.image-net.org/challenges/LSVRC/2012/)
이미지 분류 데이터 세트에서 훈련되었습니다.

정확도는 단일 이미지 크롭(single image crop) 기준으로 평가되었습니다.

| Model | Million MACs | Million Parameters | Top-1 Accuracy | Top-5 Accuracy |
| :---: | :---: | :---: | :---: | :---: |
| [MobileNet_v1_1.0_224](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_224.tgz) | 569 | 4.24 | 70.9 | 89.9 |
| [MobileNet_v1_1.0_192](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_192.tgz) | 418 | 4.24 | 70.0 | 89.2 |
| [MobileNet_v1_1.0_160](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_160.tgz) | 291 | 4.24 | 68.0 | 87.7 |
| [MobileNet_v1_1.0_128](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_128.tgz) | 186 | 4.24 | 65.2 | 85.8 |
| [MobileNet_v1_0.75_224](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_224.tgz) | 317 | 2.59 | 68.4 | 88.2 |
| [MobileNet_v1_0.75_192](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_192.tgz) | 233 | 2.59 | 67.2 | 87.3 |
| [MobileNet_v1_0.75_160](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_160.tgz) | 162 | 2.59 | 65.3 | 86.0 |
| [MobileNet_v1_0.75_128](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_128.tgz) | 104 | 2.59 | 62.1 | 83.9 |
| [MobileNet_v1_0.50_224](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_224.tgz) | 150 | 1.34 | 63.3 | 84.9 |
| [MobileNet_v1_0.50_192](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_192.tgz) | 110 | 1.34 | 61.7 | 83.6 |
| [MobileNet_v1_0.50_160](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_160.tgz) | 77 | 1.34 | 59.1 | 81.9 |
| [MobileNet_v1_0.50_128](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_128.tgz) | 49 | 1.34 | 56.3 | 79.4 |
| [MobileNet_v1_0.25_224](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_224.tgz) | 41 | 0.47 | 49.8 | 74.2 |
| [MobileNet_v1_0.25_192](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_192.tgz) | 34 | 0.47 | 47.7 | 72.3 |
| [MobileNet_v1_0.25_160](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_160.tgz) | 21 | 0.47 | 45.5 | 70.3 |
| [MobileNet_v1_0.25_128](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_128.tgz) | 14 | 0.47 | 41.5 | 66.3 |
| [MobileNet_v1_1.0_224_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_224_quant.tgz) | 569 | 4.24 | 70.1 | 88.9 |
| [MobileNet_v1_1.0_192_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_192_quant.tgz) | 418 | 4.24 | 69.2 | 88.3 |
| [MobileNet_v1_1.0_160_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_160_quant.tgz) | 291 | 4.24 | 67.2 | 86.7 |
| [MobileNet_v1_1.0_128_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_128_quant.tgz) | 186 | 4.24 | 63.4 | 84.2 |
| [MobileNet_v1_0.75_224_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_224_quant.tgz) | 317 | 2.59 | 66.8 | 87.0 |
| [MobileNet_v1_0.75_192_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_192_quant.tgz) | 233 | 2.59 | 66.1 | 86.4 |
| [MobileNet_v1_0.75_160_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_160_quant.tgz) | 162 | 2.59 | 62.3 | 83.8 |
| [MobileNet_v1_0.75_128_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.75_128_quant.tgz) | 104 | 2.59 | 55.8 | 78.8 |
| [MobileNet_v1_0.50_224_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_224_quant.tgz) | 150 | 1.34 | 60.7 | 83.2 |
| [MobileNet_v1_0.50_192_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_192_quant.tgz) | 110 | 1.34 | 60.0 | 82.2 |
| [MobileNet_v1_0.50_160_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_160_quant.tgz) | 77 | 1.34 | 57.7 | 80.4 |
| [MobileNet_v1_0.50_128_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.5_128_quant.tgz) | 49 | 1.34 | 54.5 | 77.7 |
| [MobileNet_v1_0.25_224_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_224_quant.tgz) | 41 | 0.47 | 48.0 | 72.8 |
| [MobileNet_v1_0.25_192_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_192_quant.tgz) | 34 | 0.47 | 46.0 | 71.2 |
| [MobileNet_v1_0.25_160_quant](http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_160_quant.tgz) | 21 | 0.47 | 43.4 | 68.5 |
| [MobileNet_v1_0.25_128_quant](http.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_0.25_128_quant.tgz) | 14 | 0.47 | 39.5 | 64.4 |

### 모델 개정:
* 2018년 7월 12일: narrow_range 가중치 변환을 지원하도록 수정하여 정확도 문제를 해결한 TFLite 모델 업데이트. → 에뮬레이션된 양자화 수치가 아닌 실제 TensorFlow Lite 모델 기준 검증 결과 보고.
* 2018년 8월 2일: TF 양자화 학습과 TFLite 양자화 수치가 정확히 일치하도록 업데이트.

### 모델 파일 구성
: 각 모든 tar 파일에는 다음 항목들이 포함됩니다.
* 학습된 모델 체크포인트
* 평가용 그래프 텍스트 프로토(쉽게 볼 수 있도록)
* 고정 학습 모델
* 입력 및 출력 정보가 포함된 정보 파일
* 변환된 [TensorFlow Lite](https://www.tensorflow.org/lite) flatbuffer 모델

### 양자화(Quantization) 관련 설명
양자화된 모델 GraphDefs는 float 모델이며 양자화를 시뮬레이션하기 위해 FakeQuantization 작업이 내장되어 있습니다. 이 모델들은 [TensorFlow Lite](https://www.tensorflow.org/lite)에 의해 완전한 정수 양자화 모델로 변환됩니다. 양자화의 최종 효과는 고정된 가짜 양자화 그래프(fake-quantized graph)와 TFLite flatbuffer 크기와 비교하여 확인할 수 있습니다. 즉, TFLite flatbuffer는 약 1/4 크기입니다.

여기에서 사용되는 양자화 기술에 대한 자세한 내용은 [여기](https://github.com/tensorflow/tensorflow/tree/r1.15/tensorflow/contrib/quantize)를 참조하십시오.
TF2.x에는 아직 동등한 것이 없으며 자세한 내용은 [이 RFC](https://github.com/tensorflow/community/blob/master/rfcs/20180907-contrib-sunset.md)에서 확인할 수 있습니다.

<br>

## MobileNet_v1_1.0_224 체크포인트 다운로드 예

```shell
$ CHECKPOINT_DIR=/tmp/checkpoints
$ mkdir ${CHECKPOINT_DIR}
$ wget http://download.tensorflow.org/models/mobilenet_v1_2018_08_02/mobilenet_v1_1.0_224.tgz
$ tar -xvf mobilenet_v1_1.0_224.tgz
$ mv mobilenet_v1_1.0_224.ckpt.* ${CHECKPOINT_DIR}
```

<br>

# MobileNet V1 스크립트

이 패키지에는 부동 소수점(float) 및 8비트 고정 소수점(INT8) TensorFlow 모델을 학습하기 위한 스크립트가 포함되어 있습니다.

사용되는 양자화 도구는 [여기](https://github.com/tensorflow/tensorflow/tree/r1.15/tensorflow/contrib/quantize)에 설명되어 있습니다. 
<br> TF2.x에는 아직 동등한 것이 없으며 자세한 내용은 [이 RFC](https://github.com/tensorflow/community/blob/master/rfcs/20180907-contrib-sunset.md)에서 찾을 수 있습니다.
<br> 모바일용 완전 양자화 모델로의 변환은 [TensorFlow Lite](https://www.tensorflow.org/lite)를 통해 수행할 수 있습니다.

## 사용법

### GPU용 빌드

```
$ bazel build -c opt --config=cuda mobilenet_v1_{eval,train}
```

### 실행 중

#### 부동 소수점 학습 및 평가

학습:

```
$ ./bazel-bin/mobilenet_v1_train --dataset_dir "path/to/dataset" --checkpoint_dir "path/to/checkpoints"
```

평가:

```
$ ./bazel-bin/mobilenet_v1_eval --dataset_dir "path/to/dataset" --checkpoint_dir "path/to/checkpoints"
```

#### 양자화된 학습 및 평가

기존 부동 소수점 체크포인트에서 훈련:

```
$ ./bazel-bin/mobilenet_v1_train --dataset_dir "path/to/dataset" --checkpoint_dir "path/to/checkpoints" \
  --quantize=True --fine_tune_checkpoint=float/checkpoint/path
```

처음부터 양자화 학습:

```
$ ./bazel-bin/mobilenet_v1_train --dataset_dir "path/to/dataset" --checkpoint_dir "path/to/checkpoints" --quantize=True
```

평가:

```
$ ./bazel-bin/mobilenet_v1_eval --dataset_dir "path/to/dataset" --checkpoint_dir "path/to/checkpoints" --quantize=True
```

결과적인 부동 소수점 및 양자화된 모델은 [TensorFlow Lite](https://www.tensorflow.org/lite)를 통해 실제 디바이스에서 실행할 수 있습니다.

