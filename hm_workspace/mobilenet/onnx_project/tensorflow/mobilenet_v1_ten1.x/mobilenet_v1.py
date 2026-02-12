# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# python version: 3.13.11
# TensorFlow version: 2.20.0
# TensorFlow 1.x -> 2.x
# 
# tf-slim 라이브러리의 batch_norm 함수가 최신 텐서플로우에서 더 이상 지원하지 않는
# 오래된 방식을 사용 했었음.
#
#
# =============================================================================
"""MobileNet v1.

MobileNet은 범용적인 아키텍처로, 다양한 사용 사례에 활용할 수 있다.
사용 목적에 따라 입력 레이어의 크기와 헤드 구조를 다르게 사용할 수 있으며,
예를 들어 임베딩, 위치 추정(localization), 분류(classification)와 같은 작업에 맞게 구성할 수 있다.

참고 논문 : https://arxiv.org/abs/1704.04861.

  MobileNets: 모바일 비전 응용을 위한 효율적인 신경망
  저자 : Andrew G. Howard, Menglong Zhu, Bo Chen, Dmitry Kalenichenko, Weijun Wang,
    Tobias Weyand, Marco Andreetto, Hartwig Adam

100% Mobilenet V1 (base) with input size 224x224:

See mobilenet_v1()

Layer                                                     params           macs
--------------------------------------------------------------------------------
MobilenetV1/Conv2d_0/Conv2D:                                 864      10,838,016
MobilenetV1/Conv2d_1_depthwise/depthwise:                    288       3,612,672
MobilenetV1/Conv2d_1_pointwise/Conv2D:                     2,048      25,690,112
MobilenetV1/Conv2d_2_depthwise/depthwise:                    576       1,806,336
MobilenetV1/Conv2d_2_pointwise/Conv2D:                     8,192      25,690,112
MobilenetV1/Conv2d_3_depthwise/depthwise:                  1,152       3,612,672
MobilenetV1/Conv2d_3_pointwise/Conv2D:                    16,384      51,380,224
MobilenetV1/Conv2d_4_depthwise/depthwise:                  1,152         903,168
MobilenetV1/Conv2d_4_pointwise/Conv2D:                    32,768      25,690,112
MobilenetV1/Conv2d_5_depthwise/depthwise:                  2,304       1,806,336
MobilenetV1/Conv2d_5_pointwise/Conv2D:                    65,536      51,380,224
MobilenetV1/Conv2d_6_depthwise/depthwise:                  2,304         451,584
MobilenetV1/Conv2d_6_pointwise/Conv2D:                   131,072      25,690,112
MobilenetV1/Conv2d_7_depthwise/depthwise:                  4,608         903,168
MobilenetV1/Conv2d_7_pointwise/Conv2D:                   262,144      51,380,224
MobilenetV1/Conv2d_8_depthwise/depthwise:                  4,608         903,168
MobilenetV1/Conv2d_8_pointwise/Conv2D:                   262,144      51,380,224
MobilenetV1/Conv2d_9_depthwise/depthwise:                  4,608         903,168
MobilenetV1/Conv2d_9_pointwise/Conv2D:                   262,144      51,380,224
MobilenetV1/Conv2d_10_depthwise/depthwise:                 4,608         903,168
MobilenetV1/Conv2d_10_pointwise/Conv2D:                  262,144      51,380,224
MobilenetV1/Conv2d_11_depthwise/depthwise:                 4,608         903,168
MobilenetV1/Conv2d_11_pointwise/Conv2D:                  262,144      51,380,224
MobilenetV1/Conv2d_12_depthwise/depthwise:                 4,608         225,792
MobilenetV1/Conv2d_12_pointwise/Conv2D:                  524,288      25,690,112
MobilenetV1/Conv2d_13_depthwise/depthwise:                 9,216         451,584
MobilenetV1/Conv2d_13_pointwise/Conv2D:                1,048,576      51,380,224
--------------------------------------------------------------------------------
Total:                                                 3,185,088     567,716,352


75% Mobilenet V1 (base) with input size 128x128:

See mobilenet_v1_075()

Layer                                                     params           macs
--------------------------------------------------------------------------------
MobilenetV1/Conv2d_0/Conv2D:                                 648       2,654,208
MobilenetV1/Conv2d_1_depthwise/depthwise:                    216         884,736
MobilenetV1/Conv2d_1_pointwise/Conv2D:                     1,152       4,718,592
MobilenetV1/Conv2d_2_depthwise/depthwise:                    432         442,368
MobilenetV1/Conv2d_2_pointwise/Conv2D:                     4,608       4,718,592
MobilenetV1/Conv2d_3_depthwise/depthwise:                    864         884,736
MobilenetV1/Conv2d_3_pointwise/Conv2D:                     9,216       9,437,184
MobilenetV1/Conv2d_4_depthwise/depthwise:                    864         221,184
MobilenetV1/Conv2d_4_pointwise/Conv2D:                    18,432       4,718,592
MobilenetV1/Conv2d_5_depthwise/depthwise:                  1,728         442,368
MobilenetV1/Conv2d_5_pointwise/Conv2D:                    36,864       9,437,184
MobilenetV1/Conv2d_6_depthwise/depthwise:                  1,728         110,592
MobilenetV1/Conv2d_6_pointwise/Conv2D:                    73,728       4,718,592
MobilenetV1/Conv2d_7_depthwise/depthwise:                  3,456         221,184
MobilenetV1/Conv2d_7_pointwise/Conv2D:                   147,456       9,437,184
MobilenetV1/Conv2d_8_depthwise/depthwise:                  3,456         221,184
MobilenetV1/Conv2d_8_pointwise/Conv2D:                   147,456       9,437,184
MobilenetV1/Conv2d_9_depthwise/depthwise:                  3,456         221,184
MobilenetV1/Conv2d_9_pointwise/Conv2D:                   147,456       9,437,184
MobilenetV1/Conv2d_10_depthwise/depthwise:                 3,456         221,184
MobilenetV1/Conv2d_10_pointwise/Conv2D:                  147,456       9,437,184
MobilenetV1/Conv2d_11_depthwise/depthwise:                 3,456         221,184
MobilenetV1/Conv2d_11_pointwise/Conv2D:                  147,456       9,437,184
MobilenetV1/Conv2d_12_depthwise/depthwise:                 3,456          55,296
MobilenetV1/Conv2d_12_pointwise/Conv2D:                  294,912       4,718,592
MobilenetV1/Conv2d_13_depthwise/depthwise:                 6,912         110,592
MobilenetV1/Conv2d_13_pointwise/Conv2D:                  589,824       9,437,184
--------------------------------------------------------------------------------
Total:                                                 1,800,144     106,002,432

"""

# Tensorflow mandates these.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from collections import namedtuple
import functools

import tensorflow as tf
import tf_slim as slim


# Conv and DepthSepConv는 MobileNet 레이어 구성을 정의
# Conv defines 3x3 convolution layers
# DepthSepConv : 3x3 depthwise conv. + 1x1 conv.
# stride is the stride of the convolution
# depth is the number of channels or filters in a layer
# namedtuple : 이름이 있는 필드로 구성된 불변의 시퀀스형 자료구조를 생성
Conv = namedtuple('Conv', ['kernel', 'stride', 'depth'])
DepthSepConv = namedtuple('DepthSepConv', ['kernel', 'stride', 'depth'])

# MOBILENETV1_CONV_DEFS specifies the MobileNet body
# 네트워크의 Conv layer (커널 크기, stride, 깊이)를 정의하는 conv, DepthSepConv의 리스트
MOBILENETV1_CONV_DEFS = [
    Conv(kernel=[3, 3], stride=2, depth=32),
    DepthSepConv(kernel=[3, 3], stride=1, depth=64),
    DepthSepConv(kernel=[3, 3], stride=2, depth=128),
    DepthSepConv(kernel=[3, 3], stride=1, depth=128),
    DepthSepConv(kernel=[3, 3], stride=2, depth=256),
    DepthSepConv(kernel=[3, 3], stride=1, depth=256),
    DepthSepConv(kernel=[3, 3], stride=2, depth=512),
    DepthSepConv(kernel=[3, 3], stride=1, depth=512),
    DepthSepConv(kernel=[3, 3], stride=1, depth=512),
    DepthSepConv(kernel=[3, 3], stride=1, depth=512),
    DepthSepConv(kernel=[3, 3], stride=1, depth=512),
    DepthSepConv(kernel=[3, 3], stride=1, depth=512),
    DepthSepConv(kernel=[3, 3], stride=2, depth=1024),
    DepthSepConv(kernel=[3, 3], stride=1, depth=1024)
]


def _fixed_padding(inputs, kernel_size, rate=1):
  # padding= 'SAME' 옵션을 수동으로 구현.
  """
  입력 크기와 무관하게 spatial dimensions(공간적 차원)에 대해 입력을 패딩한다.

  'VALID' 패딩을 사용하는 합성곱에서 이 입력을 사용할 경우,
  패딩되지 않은 입력에 'SAME' 패딩 합성곱을 적용했을 때와
  동일한 출력 크기를 갖도록 입력을 패딩한다.
  => 입력을 미리 패딩해 두어서,
     합성곱 연산에서는 'VALID' 패딩을 사용하더라도
     결과 출력 크기가 'SAME' 패딩을 사용한 것과 같아지도록 한다.

  Args:
    inputs: [batch, height_in, width_in, channels] 형태의 텐서
    kernel_size: conv2d 또는 max_pool2d 연산에 사용될 커널 크기
    rate: atrous(확장) 합성곱을 위한 rate 값 (정수)

    ● ● ●
    ● ● ●
    ● ● ● => rate=1. 커널 크기 3x3

    ●   ●   ●
      (빈칸)
    ●   ●   ●
      (빈칸)
    ●   ●   ● => rate=2. 커널 크기 5x5. atrous(확장) 합성곱

    => 파라미터 수는 그대로, 보는 영역은 커짐.
    => atrous conv(확장 합성곱)은 해상도를 줄이지 않고도 넓은 문맥 정보 파악 가능.
    => 주로 사용되는 곳 : Semantic Segmentation, DeepLab, stride를 키우지 않고
       receptive field(수용 영역)을 키우고 싶을 때.
    => Segmentation : 이미지 내에서 픽셀 단위로 객체를 분류하는 작업.
    => Segmentation용 MobileNet에서 특히 유용.

  Returns:
    output: [batch, height_out, width_out, channels] 형태의 텐서.
      kernel_size가 1인 경우 입력은 그대로 반환되며,
      kernel_size가 1보다 큰 경우에는 패딩된 입력이 반환된다.
  """
  kernel_size_effective = [kernel_size[0] + (kernel_size[0] - 1) * (rate - 1),    # 예. rate = 1 : 3 + (3-1)*0 = 3. => [3,3]
                           kernel_size[1] + (kernel_size[1] - 1) * (rate - 1)]    # 예. rate = 2 : 3 + (3-1)*1 = 5. => [5,5]
  pad_total = [kernel_size_effective[0] - 1, kernel_size_effective[1] - 1]
    # 크기를 동일하게 유지하려면 (커널 크기 - 1) 만큼 패딩 필요.
  pad_beg = [pad_total[0] // 2, pad_total[1] // 2]                                # 패딩의 시작 부분 크기             
  pad_end = [pad_total[0] - pad_beg[0], pad_total[1] - pad_beg[1]]                # 패딩의 끝 부분 크기
    # 총 패딩 크기를 시작 부분과 끝 부분으로 나눔.
  padded_inputs = tf.pad(
      tensor=inputs,
      paddings=[[0, 0], [pad_beg[0], pad_end[0]], [pad_beg[1], pad_end[1]],
                [0, 0]])
  return padded_inputs
    # 원본 inputs 텐서에 계산된 패딩을 더한 새로운 텐서를 반환. 


def mobilenet_v1_base(inputs,
                      final_endpoint='Conv2d_13_pointwise',
                      min_depth=8,
                      depth_multiplier=1.0,
                      conv_defs=None,
                      output_stride=None,
                      use_explicit_padding=False,
                      scope=None):
  """Mobilenet v1.

 입력으로부터 지정된 최종 엔드포인트까지 MobileNet V1 네트워크를 구성한다.

  Args:
    inputs: [batch_size, height, width, channels] 형태의 텐서
    final_endpoint: 네트워크를 어디까지 구성할지 지정하는 엔드포인트.
      다음 값 중 하나를 가질 수 있다:
      ['Conv2d_0', 'Conv2d_1_pointwise', 'Conv2d_2_pointwise',
      'Conv2d_3_pointwise', 'Conv2d_4_pointwise', 'Conv2d_5_pointwise',
      'Conv2d_6_pointwise', 'Conv2d_7_pointwise', 'Conv2d_8_pointwise',
      'Conv2d_9_pointwise', 'Conv2d_10_pointwise', 'Conv2d_11_pointwise',
      'Conv2d_12_pointwise', 'Conv2d_13_pointwise']
    min_depth: 모든 합성곱 연산에 적용되는 최소 depth 값(채널 수).
      depth_multiplier < 1 인 경우에만 강제되며,
      depth_multiplier ≥ 1 인 경우에는 제약 조건으로 작동하지 않는다.
    depth_multiplier: (=Width_multiplier, α) 모든 합성곱 연산의 depth(채널 수)에 적용되는 실수형 배율 값.
      0보다 커야 하며, 일반적으로 (0, 1) 범위의 값을 사용해
      모델의 파라미터 수나 연산량을 줄이는 데 사용된다.
    conv_defs: 네트워크 아키텍처를 정의하는 ConvDef namedtuple들의 리스트.
    output_stride: 입력 대비 출력 공간 해상도의 비율을 지정하는 정수 값.
      None이 아닌 경우, 활성화 맵의 공간 해상도가 더 이상 줄어들지 않도록
      필요 시 atrous(확장) 합성곱을 사용한다.
      허용되는 값은 다음과 같다:
        8  (정확한 완전 합성곱 모드, accurate fully convolutional mode)
        16 (빠른 완전 합성곱 모드, fast fully convolutional mode)
        32 (분류 모드, classification mode)
    use_explicit_padding: 합성곱 연산에서는 'VALID' 패딩을 사용하되,
      출력 크기가 'SAME' 패딩을 사용했을 때와 같도록
      입력을 사전에 패딩할지 여부를 지정한다.
    scope: 선택적 variable_scope.

  Returns:
    tensor_out: final_endpoint에 해당하는 출력 텐서
    end_points: 외부에서 사용할 수 있는 활성화 맵들의 집합.
      예를 들어 summary 작성이나 loss 계산 등에 활용할 수 있다.

  Raises:
    ValueError:
      final_endpoint가 미리 정의된 값 중 하나가 아닌 경우,
      depth_multiplier <= 0 인 경우,
      또는 지정한 output_stride가 허용되지 않은 경우 발생한다.

  """
  depth = lambda d: max(int(d * depth_multiplier), min_depth)
  # 주어진 layer의 depth(채널 수)를 depth_multiplier와 min_depth를 고려하여 
  # 최종적으로 사용할 layer의 채널 수를 계산하는 람다 함수.
  end_points = {}
  # 네트워크의 각 주요 지점에서의 출력 텐서를 저장하는 딕셔너리.
  # 각 레이어의 이름을 키(key)로, 해당 레이어의 출력 텐서를 값(value)로 저장.
  # 이 딕셔너리는 모델의 중간 결과를 추적하고 활용하는 데 사용된다.


  '''
  1단계 : 초기 설정 및 매개 변수 검증
  신경망을 만들기 전에, 함수에 전달된 매개 변수들을 검증하고 기본값 설정.
  '''
  # 각 레이어에서 감소(경량화)된 depth 값을 계산하기 위해 사용된다.
  if depth_multiplier <= 0:
    raise ValueError('depth_multiplier is not greater than zero.')
    # raise : 예외를 발생시키는 키워드.프로그램 흐름 중단. 오류 알림.
  if conv_defs is None:
    conv_defs = MOBILENETV1_CONV_DEFS
    
  if output_stride is not None and output_stride not in [8, 16, 32]:
    raise ValueError('Only allowed output_stride values are 8, 16, 32.')

  padding = 'SAME'
  if use_explicit_padding:
    padding = 'VALID'
  # depth_multiplier <= 0: depth_multiplier는 모델의 채널 수를 조절하여 경량화하는 역할을 하므로, 0보다 큰 값이어야 함.
  # conv_defs is None: conv_defs 매개 변수가 제공되지 않은 경우, 기본 MobileNet V1 아키텍처(MOBILENETV1_CONV_DEFS)를 사용하도록 설정.
  # output_stride not in [8, 16, 32]: output_stride는 최종 출력 feature map 해상도(크기) 비율을 지정하는데, 허용되는 값은 8, 16, 32뿐임.
  # padding = 'SAME' / 'VALID' : use_explicite_padding = True이면, 'VALID' 패딩을 사용하고, 그렇지 않으면 'SAME' 패딩을 사용하도록 설정.

  '''
  2단계 : 스코프 설정 및 루프 진입
  Tensorflow 그래프에서 변수들을 체계적으로 관리하기 위한 스코프를 설정하고, 신경망 계층을 만들기 위한 for 루프 시작.
  '''

  with tf.compat.v1.variable_scope(scope, 'MobilenetV1', [inputs]):
    with slim.arg_scope([slim.conv2d, slim.separable_conv2d], padding=padding):
      
      current_stride = 1
      # current_stride 변수는 현재 레이어까지의 합성곱 스트라이드의 누적 곱으로,
      # 활성화 맵의 출력 스트라이드(output stride)를 추적한다.
      # 이를 통해 다음 합성곱을 적용했을 때 출력 스트라이드가
      # 목표 output_stride보다 커질 경우,
      # atrous(확장) 합성곱을 사용하도록 판단할 수 있다.
      
      rate = 1
      # The atrous convolution rate parameter.
      
      net = inputs
      # net은 신경망 계층을 통과하면서 점진적으로 업데이트될 텐서.
      # 초기에는 입력 텐서(inputs)로 설정됨.

      for i, conv_def in enumerate(conv_defs):
        # 각 layer 순회하며 가져와 신경망을 구성.
        end_point_base = 'Conv2d_%d' % i
        # %i = %d

        '''
        3단계 : 스트라이드 및 atrous rate 결정
        output_stride를 구현하는 핵심 로직
        '''
        if output_stride is not None and current_stride == output_stride:
          # 현재까지 축소된 비율이 우리가 목표로 하는 축소 비율에 도달했는가?
          # 출력 해상도가 목표 수준에 도달하면 더 이상 해상도를 줄이지 않고,
          # atrous 합성곱을 사용해 receptive field만 확장한다.
          # 이때 이후 레이어를 위해 atrous rate를 누적해서 증가시킨다.
          layer_stride = 1
          layer_rate = rate
          rate *= conv_def.stride
          # 다음 atrous conv을 위해 rate 값을 현재 레이어의 원래 stride만큼 곱함.
        else:
          layer_stride = conv_def.stride
          layer_rate = 1
          current_stride *= conv_def.stride

        '''
        4단계 : Convolution 레이어 생성
        conv_def에 따라 layer 생성.
        `isinstance(conv_def, Conv)`: conv_def가 일반 Conv 객체일 경우, slim.conv2d를 사용해 표준 컨볼루션을 만듭니다.
        `isinstance(conv_def, DepthSepConv)`: conv_def가 DepthSepConv 객체일 경우, MobileNet의 핵심인 깊이별 분리 컨볼루션을 2단계로 만듭니다.
          1. Depthwise: slim.separable_conv2d의 두 번째 인자(출력 채널 수)를 None으로 주면 깊이별(depthwise) 컨볼루션만 수행됩니다. 여기서 계산된 layer_stride와 layer_rate가 적용됩니다.
          2. Pointwise: slim.conv2d를 커널 크기 [1, 1]로 호출하여 점별(pointwise) 컨볼루션을 수행합니다. 이는 채널 수를 조절하는 역할을 합니다.
        `end_points[end_point] = net`: 생성된 각 계층의 출력 net을 end_points 딕셔너리에 저장합니다.
        `if end_point == final_endpoint:`: 현재 만든 계층이 사용자가 요청한 마지막 계층(final_endpoint)이라면, 더 이상 망을 구성하지 않고 즉시 결과를 반환합니다.
        '''
        if isinstance(conv_def, Conv):                        # Standard Convolution layer
          end_point = end_point_base                          # 해당 레이어의 출력 텐서를 외부에서 참조 가능.
          if use_explicit_padding:
            net = _fixed_padding(net, conv_def.kernel)
          net = slim.conv2d(net, depth(conv_def.depth), conv_def.kernel,
                            stride=conv_def.stride,
                            scope=end_point)
          end_points[end_point] = net
          if end_point == final_endpoint:
            return net, end_points

        elif isinstance(conv_def, DepthSepConv):               # Depthwise Separable Convolution layer
          end_point = end_point_base + '_depthwise'            # 'Conv2d_1_depthwise'

          # filters=None일 경우 depthwise convolution 수행
          if use_explicit_padding:
            net = _fixed_padding(net, conv_def.kernel, layer_rate)
          net = slim.separable_conv2d(net, None, conv_def.kernel,
                                      depth_multiplier=1,
                                      stride=layer_stride,
                                      rate=layer_rate,
                                      scope=end_point)
          # num_output = None : slim은 PW를 생략하고 DW만 수행. None은 채널 수를 바꾸지 말고, 입력 채널 수와 동일하게 유지하라는 의미.
          end_points[end_point] = net
          if end_point == final_endpoint:
            return net, end_points

          end_point = end_point_base + '_pointwise'

          net = slim.conv2d(net, depth(conv_def.depth), [1, 1],
                            stride=1,
                            scope=end_point)

          end_points[end_point] = net
          if end_point == final_endpoint:
            return net, end_points
        else:
          raise ValueError('Unknown convolution type %s for layer %d'
                           % (conv_def.ltype, i))
 
  '''
  5단계 : 최종 에러 처리
  for 루프가 끝난 후에도 final_endpoint에 도달하지 못한 경우, 잘못된 매개 변수임을 알리는 에러를 발생.
  '''
  raise ValueError('Unknown final endpoint %s' % final_endpoint)


def mobilenet_v1(inputs,
                 num_classes=1000,
                 dropout_keep_prob=0.999,
                 is_training=True,
                 min_depth=8,
                 depth_multiplier=1.0,
                 conv_defs=None,
                 prediction_fn=slim.softmax,
                 spatial_squeeze=True,
                 reuse=None,
                 scope='MobilenetV1',
                 global_pool=False):
  """Mobilenet v1 model for classification.
  mobilenet_v1_base를 기반으로, classification을 위한 완전한 모델 구성
  
  주요 역할과 흐름
  1. 입력 검증: 입력 텐서가올바른지 확인.
  2. 네트워크 구성: mobilenet_v1_base를 호출하여 기본 MobileNet V1 네트워크를 구성.
  3. Pooling : base 모델에서 나온 feature map의 크기를 줄여 1차원 벡터에 가깝게 만듦.
  4. 분류 계층(Logits) 추가 : dropout과 1x1 컨볼루션을 사용해 최종 클래스 예측을 위한 logits 레이어를 추가.
  5. 출력 반환: logits와 end_points 딕셔너리를 반환.


  Args:
    inputs: [batch_size, height, width, channels] 형태의 텐서
    num_classes: 예측할 클래스의 개수.
    0 또는 None인 경우 logits 레이어는 생성되지 않으며, 대신 dropout 적용 이전의 logits 입력 특성(feature)이 반환.
    dropout_keep_prob: 활성화 값 중 유지되는 비율.
    is_training: 현재 학습 중인지 여부를 나타내는 불리언 값.
    min_depth: 모든 합성곱 연산에 적용되는 최소 depth 값(채널 수).
    depth_multiplier < 1 인 경우에만 강제되며, depth_multiplier ≥ 1 인 경우에는 활성 제약 조건이 아니다.
    depth_multiplier: 모든 합성곱 연산의 depth(채널 수)에 적용되는 실수형 배율 값.
    0보다 커야 하며, 일반적으로 (0, 1) 범위의 값을 사용해 모델의 파라미터 수나 연산량을 줄이는 데 사용된다.
    conv_defs: 네트워크 아키텍처를 정의하는 ConvDef namedtuple들의 리스트.
    prediction_fn: logits로부터 예측값을 생성하는 함수.
    spatial_squeeze: True인 경우 logits의 형태는 [B, C],
    False인 경우 logits의 형태는 [B, 1, 1, C]가 된다.
    여기서 B는 batch_size, C는 클래스 수를 의미한다.
    reuse: 네트워크 및 변수들을 재사용할지 여부.
    재사용하려면 scope가 반드시 지정되어야 한다.
    scope: 선택적 variable_scope.
    global_pool: logits 레이어 이전에 수행되는 평균 풀링(avg pooling)을 제어하는 선택적 불리언 플래그.
    False이거나 설정되지 않은 경우, 기본 입력 크기는 1x1로 줄어들도록 고정된 윈도우로 풀링되며,
    더 큰 입력의 경우에는 더 큰 출력이 생성된다.
    True인 경우, 입력 크기와 상관없이 항상 1x1로 풀링된다.

  Returns:
    net: num_classes가 0이 아닌 정수인 경우
      logits(softmax 이전의 활성화 값)을 갖는 2차원 텐서,
      num_classes가 0 또는 None인 경우에는
      logits 레이어에 입력되는(dropout 적용 이전의) 특성 텐서.
    end_points: 네트워크의 각 구성 요소에 대응하는
      활성화 값들을 담은 딕셔너리.

  Raises:
    ValueError: 입력 텐서의 차원(rank)이 올바르지 않은 경우 발생한다.
  """
  input_shape = inputs.get_shape().as_list()
  if len(input_shape) != 4:                                                # 4차원 텐서인지 확인
    raise ValueError('Invalid input tensor rank, expected 4, was: %d' %
                     len(input_shape))

  with tf.compat.v1.variable_scope(
      scope, 'MobilenetV1', [inputs], reuse=reuse) as scope:               # 변수 스코프 설정. 
    with slim.arg_scope([slim.batch_norm, slim.dropout],
                        is_training=is_training):                          # 학습 모드일 때와 평가 모드일 때 동작 다르게
      net, end_points = mobilenet_v1_base(inputs, scope=scope,
                                          min_depth=min_depth,
                                          depth_multiplier=depth_multiplier,
                                          conv_defs=conv_defs)
      with tf.compat.v1.variable_scope('Logits'):
        if global_pool:
          # Global average pooling.
          net = tf.reduce_mean(
              input_tensor=net, axis=[1, 2], keepdims=True, name='global_pool')
          end_points['global_pool'] = net
        else:
          # Pooling with a fixed kernel size.
          kernel_size = _reduced_kernel_size_for_small_input(net, [7, 7])
          net = slim.avg_pool2d(net, kernel_size, padding='VALID',
                                scope='AvgPool_1a')
          end_points['AvgPool_1a'] = net
          # net은 아직 공간적인 차원을 가지고 있으므로, 1x1으로 줄여주는 작업이 필요.
          # if global_pool: 각 채널별 특징 맵 전체 평균 계산. 모델이 다양한 크기의 입력에 대응할 수 있게 됨.
          # else : 기본 입력 크기(224x224)에 맞춰 7x7 커널의 풀링 커널을 사용해 1x1로 줄임.


        # 최종 분류 계층 (Logits layer)
        if not num_classes:
          return net, end_points
        # 1 x 1 x 1024
        net = slim.dropout(net, keep_prob=dropout_keep_prob, scope='Dropout_1b')
        logits = slim.conv2d(net, num_classes, [1, 1], activation_fn=None,
                             normalizer_fn=None, scope='Conv2d_1c_1x1')
        if spatial_squeeze:
          logits = tf.squeeze(logits, [1, 2], name='SpatialSqueeze')
      end_points['Logits'] = logits
      if prediction_fn:
        end_points['Predictions'] = prediction_fn(logits, scope='Predictions')
  return logits, end_points
  '''
   if not num_classes:: 만약 num_classes가 0 또는 None이면, 분류 계층 없이 풀링된 특징 벡터(net)까지만 반환합니다. 즉, 이 모델을 순수한 특징 추출기로 사용할 수 있습니다.
   slim.dropout: 과적합(overfitting)을 방지하기 위해 풀링된 특징 벡터에 드롭아웃을 적용합니다.
   `logits = slim.conv2d(...)`: 이 부분이 사실상 최종 분류를 위한 완전 연결 계층(Fully Connected Layer)의 역할을 합니다.
      * 1x1 크기의 컨볼루션을 num_classes 개수만큼 적용합니다. 이는 각 픽셀 위치에서 모든 채널의 정보를 종합하여 num_classes 개의 값으로 매핑하는 것과 같으며, 입력이 1x1이므로 완전 연결 계층과 동일한 연산이 됩니다.
      * activation_fn=None, normalizer_fn=None: 최종 logits는 소프트맥스(softmax) 함수에 들어가기 전의 원시 점수이므로, 별도의 활성화 함수나 정규화를 적용하지 않습니다.
   if spatial_squeeze:: logits는 [Batch, 1, 1, num_classes] 형태를 가집니다. tf.squeeze를 사용해 불필요한 1차원들을 제거하여 [Batch, num_classes]라는 더 깔끔한 형태로 만듭니다.
   end_points['Logits'] = logits: 최종 logits를 end_points 딕셔너리에 추가합니다.
   if prediction_fn:: softmax와 같은 prediction_fn이 주어지면, logits를 확률값으로 변환한 Predictions도 end_points에 추가합니다.
  '''

mobilenet_v1.default_image_size = 224


def wrapped_partial(func, *args, **kwargs):
  partial_func = functools.partial(func, *args, **kwargs)
  functools.update_wrapper(partial_func, func)
  return partial_func


mobilenet_v1_075 = wrapped_partial(mobilenet_v1, depth_multiplier=0.75)
mobilenet_v1_050 = wrapped_partial(mobilenet_v1, depth_multiplier=0.50)
mobilenet_v1_025 = wrapped_partial(mobilenet_v1, depth_multiplier=0.25)


def _reduced_kernel_size_for_small_input(input_tensor, kernel_size):
  """
  입력 크기가 작은 경우 자동으로 줄어드는 커널 크기를 정의한다.

  그래프 생성 시점에 입력 이미지의 크기를 알 수 없는 경우,
  이 함수는 입력 이미지가 충분히 크다고 가정한다.

  Args:
    input_tensor: [batch_size, height, width, channels] 형태의 입력 텐서
    kernel_size: 원하는 커널 크기.
      길이 2의 리스트 형태로 [kernel_height, kernel_width]

  Returns:
    조정된 커널 크기를 갖는 텐서
  """
  shape = input_tensor.get_shape().as_list()                          # [batch_size, height, width, channels]
  if shape[1] is None or shape[2] is None:
    kernel_size_out = kernel_size
  else:
    kernel_size_out = [min(shape[1], kernel_size[0]),
                       min(shape[2], kernel_size[1])]
  return kernel_size_out


def mobilenet_v1_arg_scope(
    is_training=True,
    weight_decay=0.00004,
    stddev=0.09,
    regularize_depthwise=False,
    batch_norm_decay=0.9997,
    batch_norm_epsilon=0.001,
    batch_norm_updates_collections=tf.compat.v1.GraphKeys.UPDATE_OPS):
  """
  MobileNetV1의 기본 arg_scope를 정의한다.
  코드의 중복을 줄이고, 모델의 하이퍼파라미터를 한 곳에서 체계적으로 관리.
  with 블록 안에서 특정 함수들이 호출될 때마다 적용될 기본 인자들을 미리 지정.
  모델을 만들 때 필요한 모든 기본 인자들을 묶어 'arg_scope'로 제공.

  Args:
    is_training: 모델이 학습 중인지 여부.
    None으로 설정된 경우, 이 파라미터는 batch_norm arg_scope에 추가되지 않는다.
    weight_decay: 모델 정규화를 위해 사용하는 weight decay 값.
    stddev: 절단 정규 분포(truncated normal) 가중치 초기화에 사용되는 표준편차 값.
    regularize_depthwise: depthwise 합성곱에 대해 정규화를 적용할지 여부.
    batch_norm_decay: batch normalization 이동 평균에 사용되는 decay 값.
    batch_norm_epsilon: batch normalization에서 0으로 나누는 것을 방지하기 위해 분산에 더해지는 작은 값.
    batch_norm_updates_collections: batch normalization 업데이트 연산들을 저장할 컬렉션.
    normalizer_fn: 합성곱 연산 이후에 적용할 정규화 함수.

  Returns:
    MobileNet V1 모델에 사용할 arg_scope.
  """
  batch_norm_params = {
      # Keras Layer는 'decay' 대신 'momentum' 파라미터를 사용.
      'momentum': batch_norm_decay,
      'epsilon': batch_norm_epsilon
  }
  # Keras Layer는 'is_training' 대신 'training' 파라미터를 사용.
  if is_training is not None:
    batch_norm_params['training'] = is_training

  # Set weight_decay for Conv and DepthSepConv layers.
  weights_init = tf.compat.v1.truncated_normal_initializer(stddev=stddev)                       # 절단 정규분포
  regularizer = tf.keras.regularizers.l2(0.5 * (weight_decay))                                     # L2 정규화
  if regularize_depthwise:
    depthwise_regularizer = regularizer
  else:
    depthwise_regularizer = None
  with slim.arg_scope(
    [slim.conv2d, slim.separable_conv2d],
    weights_initializer=weights_init,
    activation_fn=tf.nn.relu6,
    # slim.batch_norm 대신 Keras의 BatchNormalization 레이어 사용
    normalizer_fn=tf.keras.layers.BatchNormalization,
    # Keras Layer에 맞는 파라미터 전달
    normalizer_params=batch_norm_params):
    # 모든 conv와 separableConv에 적용
    with slim.arg_scope([slim.conv2d], weights_regularizer=regularizer):
      # 일반 Conv 레이어에만 적용
      with slim.arg_scope([slim.separable_conv2d],
                          weights_regularizer=depthwise_regularizer) as sc:
          # SeparableConv에만 적용
          return sc
# slim.batch_norm 대신 Keras의 BatchNormalization 레이어를 사용하도록 변경.
# Keras의 BatchNormalization 레이어는 decay와 is_training이 아닌 momentum과 training 파라미터를 사용.

