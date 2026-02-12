# 출처

[Zehaos](https://github.com/Zehaos/MobileNet) 깃허브 계정에서 가져왔습니다. 아래(MobileNet 항목부터) 내용은 해당 계정의 README파일입니다.

Language : Python


# MobileNet

Google의 모바일 비전 응용을 위한 효율적인 CNN에 대한 Tensorflow 구현. [MobileNets: Efficient Convolutional Neural Networks for Mobile Vision Applications](https://arxiv.org/abs/1704.04861)

공식 구현은 [tensorflow/model](https://github.com/tensorflow/models/blob/master/research/slim/nets/mobilenet_v1.md) 에서 확인.

객체 탐지(Object detection)에 대한 공식 구현도 공개됨. 참고 : [tensorflow/model/object_detection](https://github.com/tensorflow/models/tree/master/research/object_detection).

# News
YellowFin Optimizer가 통합되었지만, ImageNet을 학습할 GPU 자원이 없습니다. \~_\~

Official implement [click here](https://github.com/JianGoForIt/YellowFin)

## Base Module

<div align="center">
<img src="https://github.com/Zehaos/MobileNet/blob/master/figures/dwl_pwl.png"><br><br>
</div>

## Accuracy on ImageNet-2012 Validation Set

| Model | Width Multiplier |Preprocessing  | Accuracy-Top1|Accuracy-Top5 |
|--------|:---------:|:------:|:------:|:------:|
| MobileNet |1.0| Same as Inception | 66.51% | 87.09% |

Download the pretrained weight: [OneDrive](https://1drv.ms/u/s!AvkGtmrlCEhDhy1YqWPGTMl1ybee), [BaiduYun](https://pan.baidu.com/s/1i5xFjal) 

**Loss**
<div align="center">
<img src="https://github.com/Zehaos/MobileNet/blob/master/figures/epoch90_full_preprocess.png"><br><br>
</div>

## Time Benchmark
Environment: Ubuntu 16.04 LTS, Xeon E3-1231 v3, 4 Cores @ 3.40GHz, GTX1060.

TF 1.0.1(native pip install), TF 1.1.0(build from source, optimization flag '-mavx2')


| Device | Forward| Forward-Backward |Instruction set|Quantized|Fused-BN|Remark|
|--------|:---------:|:---------:|:---------:|:---------:|:---------:|:---------:|
|CPU|52ms|503ms|-|-|-|TF 1.0.1|
|CPU|44ms|177ms|-|-|On|TF 1.0.1|
|CPU|31ms| - |-|8-bits|-|TF 1.0.1|
|CPU|26ms| 75ms|AVX2|-|-|TF 1.1.0|
|CPU|128ms| - |AVX2|8-bits|-|TF 1.1.0|
|CPU|**19ms**| 89ms|AVX2|-|On|TF 1.1.0|
|GPU|3ms|16ms|-|-|-|TF 1.0.1, CUDA8.0, CUDNN5.1|
|GPU|**3ms**|15ms|-|-|On|TF 1.0.1, CUDA8.0, CUDNN5.1|
> Image Size: (224, 224, 3), Batch Size: 1

## Usage

### Train on Imagenet

1. Imagenet data를 준비하세요. Inception을 학습하는 방법은 Google의 튜토리얼 참고 [training inception](https://github.com/tensorflow/models/tree/master/inception#getting-started).

2. './script/train_mobilenet_on_imagenet.sh'를 본인 환경에 맞게 수정하세요.

```
bash ./script/train_mobilenet_on_imagenet.sh
```

### Benchmark speed
```
python ./scripts/time_benchmark.py
```

### Train MobileNet Detector (Debugging)

1. KITTI data 준비.

KITTI 데이터를 다운로드한 후, 이를 학습용과 검증용 데이터셋으로 나눠야 함.
```
cd /path/to/kitti_root
mkdir ImageSets
cd ./ImageSets
ls ../training/image_2/ | grep ".png" | sed s/.png// > trainval.txt
python ./tools/kitti_random_split_train_val.py
```
kitti_root 폴더는 아래처럼 구성.
```
kitti_root/
                  |->training/
                  |     |-> image_2/00****.png
                  |     L-> label_2/00****.txt
                  |->testing/
                  |     L-> image_2/00****.png
                  L->ImageSets/
                        |-> trainval.txt
                        |-> train.txt
                        L-> val.txt
```
이를 tfrecord 형식으로 변환.
```
python ./tools/tf_convert_data.py
```

2. './script/train_mobilenet_on_kitti.sh' 를 본인 환경에 맞게 수정.
```
bash ./script/train_mobilenetdet_on_kitti.sh
```

> 이 프로젝트의 코드는 대부분 SqueezeDet와 SSD-Tensorflow를 기반으로 작성.
> 버그에 대한 피드백을 주시면 감사하겠습니다.

## 문제 해결

1. MobileNet model size에 대하여

논문에 따르면 MobileNet은 약 330만(3.3M) 개의 파라미터를 가지며, 이는 입력 해상도에 따라 변하지 않습니다.
이는 fully-connected(fc) 레이어 때문에, 최종 모델의 전체 파라미터 수가 330만 개보다 커져야 함을 의미합니다.

RMSprop 학습 전략을 사용할 경우, 체크포인트 파일 크기는 모델 크기의 약 3배 정도가 되는데, 이는 RMSprop에서 사용하는 일부 보조 파라미터(auxiliary parameters) 때문입니다.
이와 관련된 내용은 inspect_checkpoint.py를 사용해 확인할 수 있습니다.

2. Slim 기반 multi-GPU 성능 문제

[#1390](https://github.com/tensorflow/models/issues/1390)
[#1428](https://github.com/tensorflow/models/issues/1428#issuecomment-302589426)

## TODO
- [x] Imagenet으로 학습
- [x] Width Multiplier 하이퍼파라미터 추가
- [x] 학습 결과 보고
- [ ] 객체 탐지 작업에 통합

## Reference
[MobileNets: Efficient Convolutional Neural Networks for Mobile Vision Applications](https://arxiv.org/abs/1704.04861)

[SSD-Tensorflow](https://github.com/balancap/SSD-Tensorflow)

[SqueezeDet](https://github.com/BichenWuUCB/squeezeDet)
